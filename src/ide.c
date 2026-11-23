/*
 * ide.c - Respuesta JSON para el prototipo de IDE.
 *
 * Estado: esqueleto. `jaf --ide` ya habla con ide/jaf_ide.py.
 * Cada funcion TODO es una feature del enunciado (sugerencias, fixes,
 * subrayado de errores, clones Tipo I/II entre archivos).
 */

#include "ide.h"

#include "clones.h"

#include "fixes.h"
#include "semantico.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lista global de errores(Maximo 100)
static DiagError lista_errores[100];
static int num_errores = 0;

//Limpia la lista de errores
void ide_limpiar_errores(void)
{
    for (int i = 0; i < num_errores; i++) {
        free(lista_errores[i].archivo);
        free(lista_errores[i].mensaje);
        free(lista_errores[i].severidad);
    }
    num_errores = 0;
}


/* Registra un error en la lista global
 * Se llama desde semantico y main
 */
void ide_registrar_error(const char *archivo, int linea, int col, int codigo, 
                         const char *mensaje, const char *severidad)
{
    if (num_errores >= 100) {
        return; /* Lista llena */
    }
    for (int i = 0; i < num_errores; i++) {
        DiagError *y = &lista_errores[i];
        if (y->linea == linea && y->col == col && y->codigo == codigo && !strcmp(y->mensaje, mensaje ? mensaje : "")
            && !strcmp(y->archivo, archivo ? archivo : "desconocido")) {
            return; /* el mismo error visto otra vez (multi-archivo, #import) */
        }
    }
    DiagError *e = &lista_errores[num_errores++];
    e->archivo = strdup(archivo ? archivo : "desconocido");
    e->linea = linea;
    e->col = col;
    e->codigo = codigo;
    e->mensaje = strdup(mensaje ? mensaje : "");
    e->severidad = strdup(severidad ? severidad : "Error");
}

static void json_escape(FILE *f, const char *s)
{
    int c;
    if (!s) {
        return;
    }
    for (; (c = (unsigned char)*s); s++) {
        if (c == '"' || c == '\\') {
            fputc('\\', f);
            fputc(c, f);
        } else if (c == '\n') {
            fputs("\\n", f);
        } else if (c == '\t') {
            fputs("\\t", f);
        } else if (c < 0x20) {
            fprintf(f, "\\u%04x", c);
        } else {
            fputc(c, f);
        }
    }
}

void ide_json_str(FILE *f, const char *s)
{
    fputc('"', f);
    json_escape(f, s ? s : "");
    fputc('"', f);
}

/* Emite el arreglo JSON de diagnostics a partir de la lista global.
   Recorrer lexer/parser/semantico y emitir un diagnostic por error.
 * Usar linea:col que ya imprimen yyerror y sem_analizar. */
static void emitir_diagnostics(FILE *f, char **archivos, int n)
{
    (void)archivos;
    (void)n;
    fputs("[", f);
        for (int i = 0; i < num_errores; i++) {
        if (i > 0) {
            fputs(",", f);
        }
        DiagError *e = &lista_errores[i];
        fputs("{\n    \"file\": \"", f);      json_escape(f, e->archivo);
        fputs("\",\n    \"line\": ", f);      fprintf(f, "%d", e->linea);
        fputs(",\n    \"col\": ", f);         fprintf(f, "%d", e->col);
        fputs(",\n    \"code\": \"", f);      fprintf(f, "%d", e->codigo);
        fputs("\",\n    \"msg\": \"", f);     json_escape(f, e->mensaje);
        fputs("\",\n    \"severity\": \"", f); json_escape(f, e->severidad);
        fputs("\"\n  }", f);
    }
    fputs("]", f);
}

static void emitir_fixes(FILE *f, char **archivos, int n)
{
    fixes_emitir_json(f, archivos, n);
}


static const char *clase_texto(const Simbolo *s)
{
    return s->clase == CL_VAR ? "variable" : s->clase == CL_PARAM ? "parametro" : "funcion";
}

static const char *ambito_texto(const Simbolo *s)
{
    return s->alm == AL_GLOBAL ? "global" : s->alm == AL_PARAM ? "param" : s->alm == AL_LOCAL ? "local" : "-";
}

/* "list ent[4][2]", "list ent (ref)" o el tipo escalar. */
static void tipo_texto(const Simbolo *s, char *buf, size_t n)
{
    if (s->ndim == 2) {
        snprintf(buf, n, "%s[%ld][%ld]", ty_nombre(s->ty), s->dim[0], s->dim[1]);
    } else if (s->ndim == 1) {
        snprintf(buf, n, "%s[%ld]", ty_nombre(s->ty), s->dim[0]);
    } else if (s->ndim < 0) {
        snprintf(buf, n, "%s (ref)", ty_nombre(s->ty));
    } else {
        snprintf(buf, n, "%s", ty_nombre(s->ty));
    }
}

/* La misma tabla que imprime -m, una fila por simbolo de sem.todos. */
void ide_simbolos_json(FILE *f)
{
    fputc('[', f);
    for (int i = 0; i < sem.n_todos; i++) {
        const Simbolo *s = sem.todos[i];
        char tipo[48];
        tipo_texto(s, tipo, sizeof tipo);
        fputs(i ? ",\n  {" : "\n  {", f);
        fputs("\"nombre\": ", f);
        ide_json_str(f, s->nombre);
        fputs(", \"clase\": ", f);
        ide_json_str(f, clase_texto(s));
        fputs(", \"tipo\": ", f);
        ide_json_str(f, tipo);
        fprintf(f, ", \"ndim\": %d, \"dim\": [%ld, %ld], \"ambito\": ", s->ndim, s->dim[0], s->dim[1]);
        ide_json_str(f, ambito_texto(s));
        fputs(", \"funcion\": ", f);
        ide_json_str(f, s->funcion ? s->funcion : "(global)");
        fprintf(f, ", \"nivel\": %d, \"base\": \"%s\", \"offset\": %ld", s->nivel,
                s->alm == AL_GLOBAL ? "gp" : s->clase == CL_FUNC ? "" : "fp", s->offset);
        fprintf(f, ", \"linea\": %d, \"col\": %d, \"archivo\": ", s->linea, s->col);
        ide_json_str(f, s->archivo);
        if (s->clase == CL_FUNC) {
            fprintf(f, ", \"nparams\": %d, \"marco\": %ld", s->nparams, s->marco);
        }
        fputc('}', f);
    }
    fputs(sem.n_todos ? "\n]" : "]", f);
}

static const char *const palabras_jaf[] = {
    "ent", "bool", "list", "vacio", "def", "llamar", "retorno", "terminar",
    "si", "sino", "mientras", "para", "verdadero", "falso", "#import",
};

static const char *const intrinsecas_jaf[][2] = {
    {"autenticar", "vacio autenticar(ent credencial)"},
    {"cerrar_sesion", "vacio cerrar_sesion()"},
    {"leer_estado", "ent leer_estado()"},
    {"cargar_llave", "vacio cargar_llave(ent valor, ent conjunto, ent palabra)"},
    {"ronda_feistel", "vacio ronda_feistel(ent L, ent R, ent conjunto, ent ronda)"},
    {"feistel_cifrar", "vacio feistel_cifrar(ent L, ent R, ent conjunto)"},
    {"feistel_descifrar", "vacio feistel_descifrar(ent L, ent R, ent conjunto)"},
    {"leer_mem", "ent leer_mem(ent direccion)"},
    {"escribir_mem", "vacio escribir_mem(ent direccion, ent valor)"},
};

static void completion(FILE *f, int *primero, const char *label, const char *kind, const char *detail)
{
    fputs(*primero ? "\n  {" : ",\n  {", f);
    *primero = 0;
    fputs("\"label\": ", f);
    ide_json_str(f, label);
    fputs(", \"kind\": ", f);
    ide_json_str(f, kind);
    fputs(", \"detail\": ", f);
    ide_json_str(f, detail);
    fputc('}', f);
}

/* Palabras clave, intrinsecas y los simbolos del ultimo analisis semantico
   (sem.todos). Un nombre repetido en varios ambitos sale una vez por ambito. */
static void emitir_completions(FILE *f, char **archivos, int n)
{
    int primero = 1;
    size_t k;
    (void)archivos;
    (void)n;

    fputc('[', f);
    for (k = 0; k < sizeof palabras_jaf / sizeof *palabras_jaf; k++) {
        completion(f, &primero, palabras_jaf[k], "palabra", "palabra reservada");
    }
    for (k = 0; k < sizeof intrinsecas_jaf / sizeof *intrinsecas_jaf; k++) {
        completion(f, &primero, intrinsecas_jaf[k][0], "intrinseca", intrinsecas_jaf[k][1]);
    }
    for (int i = 0; i < sem.n_todos; i++) {
        const Simbolo *s = sem.todos[i];
        char tipo[48], detalle[160];
        tipo_texto(s, tipo, sizeof tipo);
        if (s->clase == CL_FUNC) {
            snprintf(detalle, sizeof detalle, "%s %s(%d param.) linea %d", tipo, s->nombre, s->nparams, s->linea);
        } else {
            snprintf(detalle, sizeof detalle, "%s · %s%s%s · linea %d", tipo, ambito_texto(s),
                     s->alm == AL_GLOBAL ? "" : " de ", s->alm == AL_GLOBAL ? "" : (s->funcion ? s->funcion : "?"),
                     s->linea);
        }
        completion(f, &primero, s->nombre, clase_texto(s), detalle);
    }
    fputs(primero ? "]" : "\n]", f);
}

/* Tipo I / Tipo II entre archivos — ver src/clones.c */
static void emitir_clones(FILE *f, char **archivos, int n)
{
    clones_emitir_json(f, archivos, n);
}

void ide_emitir_campos(FILE *f, char **archivos, int n)
{
    int i;

    fputs("\n  \"diagnostics\": ", f);
    emitir_diagnostics(f, archivos, n);
    fputs(",\n  \"fixes\": ", f);
    emitir_fixes(f, archivos, n);
    fputs(",\n  \"completions\": ", f);
    emitir_completions(f, archivos, n);
    fputs(",\n  \"clones\": ", f);
    emitir_clones(f, archivos, n);
    fputs(",\n  \"files\": [", f);
    for (i = 0; i < n; i++) {
        if (i) {
            fputs(", ", f);
        }
        ide_json_str(f, archivos[i]);
    }
    fputc(']', f);
}

void ide_emitir_json(char **archivos, int n)
{
    fputc('{', stdout);
    ide_emitir_campos(stdout, archivos, n);
    fputs("\n}\n", stdout);
}
