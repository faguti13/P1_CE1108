#define _DEFAULT_SOURCE   /* realpath */
#include "linker.h"
#include "comun.h"
#include "ide.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 * Enlazador de #import. Recorrido en profundidad: un modulo entra al programa
 * despues de los modulos que el importa (post-orden), asi que sus funciones y
 * globales ya existen cuando el importador las usa. Un modulo que ya esta en el
 * camino actual es una importacion circular: se avisa y se ignora.
 *
 * Errores:
 *   {ERROR 03} modulo no encontrado o con errores lexicos/sintacticos
 *   {AVISO 02} importacion circular (se ignora)
 */

#define MAX_MODULOS 64

typedef struct Modulo {
    char *ruta;        /* ruta canonica, para detectar repetidos y ciclos */
    char *nombre;      /* ruta como se abrio, para los mensajes */
    int en_camino;     /* 1 mientras se enlazan sus propios imports */
    Nodo *cuerpo;      /* elementos de nivel superior del modulo */
} Modulo;

static Modulo mods[MAX_MODULOS];
static int n_mods;
static Nodo *orden_ini, *orden_fin;  /* cadena de elementos en orden de dependencias */
static int errores;
static int verb;

// Hace: registra y muestra un diagnostico de enlace; recibe: n, archivo, codigo, tipo, fmt, arg; genera: efectos sobre estructuras o salida.
static void error_en(const Nodo *n, const char *archivo, int codigo, const char *tipo, const char *fmt,
                     const char *arg)
{
    char msg[512];

    snprintf(msg, sizeof msg, fmt, arg);
    fprintf(stderr, "{%s %02d}:%s:%d:%d:\"%s\"\n", tipo, codigo, archivo, n ? n->linea : 0, n ? n->col : 0, msg);
    ide_registrar_error(archivo, n ? n->linea : 0, n ? n->col : 0, codigo, msg, tipo[0] == 'E' ? "Error" : "Aviso");
    if (tipo[0] == 'E') {
        errores++;
    }
}

/* Directorio de `ruta` (con la barra final) o "" si no tiene. */
// Hace: extrae el directorio de una ruta; recibe: ruta; genera: puntero o NULL segun corresponda.
static char *dir_de(const char *ruta)
{
    const char *s = strrchr(ruta, '/');
    size_t n = s ? (size_t)(s - ruta + 1) : 0;
    char *d = malloc(n + 1);
    memcpy(d, ruta, n);
    d[n] = '\0';
    return d;
}

/* Ruta del modulo: relativa al importador; si no existe, tal cual. Agrega .oly si falta. */
// Hace: localiza la ruta del modulo importado; recibe: importador, nombre; genera: puntero o NULL segun corresponda.
static char *resolver(const char *importador, const char *nombre)
{
    char *dir = dir_de(importador);
    const char *ext = strrchr(nombre, '.');
    int con_ext = ext && !strchr(ext, '/');
    size_t n = strlen(dir) + strlen(nombre) + 5;
    char *r = malloc(n);
    FILE *f;

    snprintf(r, n, "%s%s%s", nombre[0] == '/' ? "" : dir, nombre, con_ext ? "" : ".oly");
    free(dir);
    if ((f = fopen(r, "r"))) {
        fclose(f);
        return r;
    }
    snprintf(r, n, "%s%s", nombre, con_ext ? "" : ".oly");
    if ((f = fopen(r, "r"))) {
        fclose(f);
        return r;
    }
    free(r);
    return NULL;
}

// Hace: normaliza la ruta de un archivo; recibe: ruta; genera: puntero o NULL segun corresponda.
static char *canonica(const char *ruta)
{
    char buf[PATH_MAX];
    return strdup(realpath(ruta, buf) ? buf : ruta);
}

// Hace: busca un modulo previamente cargado; recibe: canon; genera: valor o indicador calculado.
static int buscar_modulo(const char *canon)
{
    int i;
    for (i = 0; i < n_mods; i++) {
        if (!strcmp(mods[i].ruta, canon)) {
            return i;
        }
    }
    return -1;
}

/* Agrega la cadena `elems` al final del programa enlazado. */
// Hace: agrega elementos al programa enlazado; recibe: elems; genera: efectos sobre estructuras o salida.
static void anexar(Nodo *elems)
{
    Nodo *u;
    if (!elems) {
        return;
    }
    if (!orden_ini) {
        orden_ini = elems;
    } else {
        orden_fin->sig = elems;
    }
    for (u = elems; u->sig; u = u->sig) {
    }
    orden_fin = u;
}

static void enlazar_imports(Nodo *imps, const char *importador);

/* Parsea un modulo y enlaza recursivamente sus propios imports. */
// Hace: analiza y enlaza recursivamente un modulo; recibe: imp, importador; genera: efectos sobre estructuras o salida.
static void cargar_modulo(const Nodo *imp, const char *importador)
{
    char *ruta = resolver(importador, imp->texto ? imp->texto : "");
    char *canon;
    int k, st;
    FILE *f;
    Nodo *raiz;
    const char *prev_archivo = ast_archivo;

    if (!ruta) {
        error_en(imp, importador, 3, "ERROR", "no se encontro el modulo '%s'", imp->texto ? imp->texto : "");
        return;
    }
    canon = canonica(ruta);
    k = buscar_modulo(canon);
    if (k >= 0) {
        if (mods[k].en_camino) {
            error_en(imp, importador, 2, "AVISO", "importacion circular de '%s': se ignora", imp->texto);
        }
        free(ruta);
        free(canon);
        return;
    }
    if (n_mods == MAX_MODULOS) {
        error_en(imp, importador, 3, "ERROR", "demasiados modulos (maximo 64) al importar '%s'", imp->texto);
        free(ruta);
        free(canon);
        return;
    }
    k = n_mods++;
    mods[k].ruta = canon;
    mods[k].nombre = ruta;
    mods[k].en_camino = 1;
    mods[k].cuerpo = NULL;
    if (verb) {
        fprintf(stderr, "[jaf] enlazando modulo %s\n", ruta);
    }

    f = fopen(ruta, "r");
    ast_archivo = ruta;
    ast_raiz = NULL;
    lexer_reiniciar(f);
    yyin = f;
    st = yyparse();
    fclose(f);
    yyin = NULL;
    raiz = ast_raiz;
    ast_archivo = prev_archivo;
    if (st != 0 || lexer_hubo_error || !raiz) {
        error_en(imp, importador, 3, "ERROR", "el modulo '%s' tiene errores lexicos o sintacticos", imp->texto);
        mods[k].en_camino = 0;
        return;
    }
    enlazar_imports(raiz->hijo ? raiz->hijo->hijo : NULL, ruta);
    if (raiz->hijo && raiz->hijo->sig) {
        mods[k].cuerpo = raiz->hijo->sig->hijo;
        raiz->hijo->sig->hijo = NULL;
    }
    anexar(mods[k].cuerpo);
    mods[k].en_camino = 0;
}

// Hace: procesa las importaciones declaradas; recibe: imps, importador; genera: efectos sobre estructuras o salida.
static void enlazar_imports(Nodo *imps, const char *importador)
{
    Nodo *h;
    for (h = imps; h; h = h->sig) {
        if (h->tipo == NODO_IMPORTACION) {
            cargar_modulo(h, importador);
        }
    }
}

// Hace: integra los modulos con el programa principal; recibe: raiz, ruta, verboso; genera: valor o indicador calculado.
int enlazar(Nodo *raiz, const char *ruta, int verboso)
{
    Nodo *imps, *cuerpo;
    Nodo *raiz_principal = raiz;
    int i;

    for (i = 0; i < n_mods; i++) {
        free(mods[i].ruta);
        free(mods[i].nombre);
    }
    n_mods = 0;
    errores = 0;
    verb = verboso;
    orden_ini = orden_fin = NULL;
    if (!raiz || !raiz->hijo) {
        return 0;
    }
    imps = raiz->hijo->hijo;
    cuerpo = raiz->hijo->sig;
    /* el principal tambien cuenta como "en camino" para detectar que un modulo lo importe */
    mods[n_mods].ruta = canonica(ruta);
    mods[n_mods].nombre = strdup(ruta);
    mods[n_mods].en_camino = 1;
    n_mods++;
    enlazar_imports(imps, ruta);
    ast_raiz = raiz_principal;
    if (cuerpo && orden_ini) {
        orden_fin->sig = cuerpo->hijo;
        cuerpo->hijo = orden_ini;
    }
    return errores;
}

// Hace: cuenta los modulos importados; recibe: sin parametros; genera: valor o indicador calculado.
int enlace_n_modulos(void)
{
    return n_mods > 0 ? n_mods - 1 : 0;
}

// Hace: consulta un modulo ya enlazado; recibe: i; genera: puntero o NULL segun corresponda.
const char *enlace_modulo(int i)
{
    return (i >= 0 && i + 1 < n_mods) ? mods[i + 1].nombre : NULL;
}
