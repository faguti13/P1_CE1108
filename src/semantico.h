#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "ast.h"
#include <stdio.h>

/* Tipos de expresion durante el anlisis sematico.
  TY_ERR es compatible con todo para no encadenar errores. */
enum { TY_ERR = 0, TY_ENT, TY_BOOL, TY_VACIO, TY_LISTA_ENT, TY_LISTA_BOOL };

/* Clase del sibolo dentro de la tabla de simbolo
   Indica si el simbolo corresponde a una variable, parametro, funcion o funcion intrinseca
*/
typedef enum { CL_VAR, CL_PARAM, CL_FUNC, CL_INTRINSECA } Clase;

/* Ubicacion de un simbolo en memoria
   AL_NINGUNO indica que no hay ubicacion asignada
*/
typedef enum { AL_NINGUNO, AL_GLOBAL, AL_LOCAL, AL_PARAM } Alm;

/* Funciones intrinseca 
(Representan instr o funcionalidades especiales que se salen de las funciones normales del programa)
*/
typedef enum {
    IN_NINGUNA, IN_AUTENTICAR, IN_CERRAR_SESION, IN_LEER_ESTADO, IN_CARGAR_LLAVE,
    IN_RONDA_FEISTEL, IN_FEISTEL_CIFRAR, IN_FEISTEL_DESCIFRAR, IN_LEER_MEM, IN_ESCRIBIR_MEM
} Intr;

// Simbolo de la tabla de simbolo
typedef struct Simbolo {
    char *nombre;           /*Nombre del simbolo*/
    Clase clase;            /*Clase del simbolo*/
    int ty;                 /*tipo de la variable*/
    int ndim;               /*Dimensiones: 0 escalar, 1 o 2 lista, -1 lista parametro (dimension desconocida)*/
    long dim[2];            /*Tamano de cada dimension de una lista*/
    Alm alm;                /*Ubicacion del simbolo en memoria*/
    long offset;            /*Desplazamiento respecto de gp (globales) o fp (locales y parametros)*/
    int nivel;              /*Profundidad del ambito al que pertenece el simb(0 = global) */
    int linea, col;         /*Posicion de declaracion en cod fuente*/
    
    char *funcion;         /*Nombre de la funcion que contiene el simbolo
                            "(global) si el simbolo pertenece al ambito global"*/
    /*Info de las funciones*/
    int nparams;            /*Cantidad de parametros*/
    int ptipo[16];          /*Tipo de cada parametro*/
    int pndim[16];          /*Numero de dimensiones de cada parametro*/

    long marco;             /*Tamano  del marco de activacion de la funcion (bytes), sin contar la zona de derrame */
    long zona_derrame;      /*Offset (positivo hacia abajo) donde empieza la zona de derrame */
    Intr intr;              /*Identifica la funcion intrinseca asociada al simbolo si corresponde*/
    Nodo *def;              /*Nodo del AST donde se encuentra la definicion del simbolo*/
} Simbolo;

/* Cantidad de registros temporales que pueden derramarse al llamar una funcion
   Contempla de r8 a r23.
*/
#define N_DERRAME 16        
/*Estructura principal del analisis semantico*/
typedef struct Semantico {
    const char *archivo;    /*Nombre del archivo en anaisis*/
    int errores;            /*Cant. de errores semanticos encontrados*/
    int avisos;             /*Cantidad de avisos generados*/
    Simbolo **todos;        /*Arreglo con todos los simbolos en orden de declaracion */
    int n_todos;            /*Cantidad total de simbolos*/
    long bytes_globales;    /*Bytes utilizados por las variables globales.*/
    long marco_inicio;      /*Marco del codigo de nivel superior (derrame) */
} Semantico;

extern Semantico sem; /*Instancia global del estado del analizador semantico*/

/* Analiza el arbol de sintaxis
 * Se anotan los nodos con su tipo y simbolo, se asigna dir. de memoria.
 * Retorno de numero total de errores. */
int sem_analizar(Nodo *raiz, const char *archivo); 
void sem_imprimir_tabla(FILE *f);                  /*Imprime tabla de simbolo
const char *ty_nombre(int ty);                     /*De tipo interno a text*/
int sem_const(const Nodo *n, long *valor);         /*Evalua una expr constante entera */

#endif
