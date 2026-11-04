/*
 * ide.c - Respuesta JSON para el prototipo de IDE.
 *
 * Estado: esqueleto. `jaf --ide` ya habla con ide/jaf_ide.py.
 * Cada funcion TODO es una feature del enunciado (sugerencias, fixes,
 * subrayado de errores, clones Tipo I/II entre archivos).
 */

#include "ide.h"

#include "clones.h"

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
        } else {
            fputc(c, f);
        }
    }
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

/* TODO: falta ';' (s01), llave (s02), parentesis. kind insert/replace. */
static void emitir_fixes(FILE *f, char **archivos, int n)
{
    (void)archivos;
    (void)n;
    fputs("[]", f);
}

/* TODO: keywords JAF + simbolos visibles en el cursor (tabla de simbolos). */
static void emitir_completions(FILE *f, char **archivos, int n)
{
    (void)archivos;
    (void)n;
    fputs("[]", f);
}

/* Tipo I / Tipo II entre archivos — ver src/clones.c */
static void emitir_clones(FILE *f, char **archivos, int n)
{
    clones_emitir_json(f, archivos, n);
}

void ide_emitir_json(char **archivos, int n)
{
    int i;

    fputs("{\n  \"diagnostics\": ", stdout);
    emitir_diagnostics(stdout, archivos, n);
    fputs(",\n  \"fixes\": ", stdout);
    emitir_fixes(stdout, archivos, n);
    fputs(",\n  \"completions\": ", stdout);
    emitir_completions(stdout, archivos, n);
    fputs(",\n  \"clones\": ", stdout);
    emitir_clones(stdout, archivos, n);
    fputs(",\n  \"files\": [", stdout);
    for (i = 0; i < n; i++) {
        if (i) {
            fputs(", ", stdout);
        }
        fputc('"', stdout);
        json_escape(stdout, archivos[i]);
        fputc('"', stdout);
    }
    fputs("]\n}\n", stdout);
}
