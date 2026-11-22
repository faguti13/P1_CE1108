#ifndef IDE_H
#define IDE_H

#include <stdio.h>

/*
 * Backend del IDE (CE1108, Proyecto 2).
 * La ventana esta en ide/jaf_ide.py; este modulo imprime JSON a stdout.
 *
 * Contrato JSON de `jaf --ide a.oly b.oly ...`:
 * {
 *   "diagnostics": [ {"file","line","col","code","msg","severity"} ],   lexico 01, sintaxis 02, semantico 1x-3x
 *   "fixes":        [ {"file","line","col","kind":"insert"|"replace","text"} ],
 *   "completions":  [ {"label","kind","detail"} ],   palabra | intrinseca | variable | parametro | funcion
 *   "clones":       [ {"type":1|2, "a":{"file","line","end"}, "b":{"file","line","end"}} ],
 *   "files":        [ ... ]
 * }
 *
 * `jaf --ide-todo a.oly [-O2 --unroll=K ...]` agrega, en la misma invocacion:
 *   "simbolos"  la tabla de -m (ver ide_simbolos_json)
 *   "ast"       el arbol de --json (null si no parsea)
 *   "ir"        {"antes": TAC recien generado, "despues": TAC tras las pasadas}
 *   "paquetes"  {"base": [...], "opt": [...]}  bundles {"dir","etq","slots"[4],"lineas"[4]}
 *   "metricas"  {"base": {...}, "opt": {..., "estad": {...}}}  generador de P1 vs tuberia
 */

typedef struct {
    /* Estructura para guardar un error detectado por el compilador 
    *Los datos son dados por el lexer,parser o semnatico segun corresponda
    */
    char *archivo;    //Nombre del archvo donde ocurrió el error
    int linea;        //Número de línea
    int col;          //Número de columna
    int codigo;       //El número de error (10, 11, 12, 02, etc.)
    char *mensaje;    //Descripción del erro
    char *severidad;  //Si es "Error" o "Warning" 
} DiagError;

/*Registra un error en la lista global.Se llama desde semantico o main*/
void ide_registrar_error(const char *archivo, int linea, int col, int codigo, const char *mensaje, const char *severidad);
/*Limpia la lista de errores para cada ejecución del IDE*/
void ide_limpiar_errores(void);

void ide_emitir_json(char **archivos, int n);
/* Los campos de ide_emitir_json sin las llaves externas. */
void ide_emitir_campos(FILE *f, char **archivos, int n);
/* Cadena JSON entre comillas. */
void ide_json_str(FILE *f, const char *s);
/* Arreglo JSON con sem.todos (--tabla-json). */
void ide_simbolos_json(FILE *f);

struct Nodo;
struct OptCfg;
/* --ide-todo: raiz es NULL si el archivo no parsea. Imprime el objeto completo. */
int ide_todo(struct Nodo *raiz, const char *ruta, const struct OptCfg *oc);

#endif
