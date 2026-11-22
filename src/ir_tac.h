#ifndef IR_TAC_H
#define IR_TAC_H

#include "ast.h"
#include "semantico.h"

#include <stdio.h>

/*
 * Representacion intermedia de 3 direcciones (cuadruplas), independiente de los
 * registros fisicos. Cada funcion tiene registros virtuales ilimitados:
 *   - temporales de expresiones (t0, t1, ...)
 *   - variables escalares locales y parametros promovidos (llevan el nombre del simbolo)
 * Las globales y las listas viven en memoria (TAC_LOAD / TAC_STORE sobre gp o fp).
 * Operandos: registro virtual, constante o registro fisico fijo (r0, gp, fp, sp).
 */

typedef enum TacOp {
    TAC_CONST,    /* d = c */
    TAC_COPY,     /* d = a */
    TAC_BIN,      /* d = a op b */
    TAC_UN,       /* d = op a */
    TAC_LOAD,     /* d = M[a + imm] */
    TAC_STORE,    /* M[a + imm] = b */
    TAC_LABEL,    /* etq: */
    TAC_JMP,      /* goto etq */
    TAC_BCOND,    /* if a op b goto etq */
    TAC_JZ,       /* if a == 0 goto etq */
    TAC_JNZ,      /* if a != 0 goto etq */
    TAC_PARAM,    /* argumento imm = a */
    TAC_CALL,     /* d = call etq (imm argumentos); d puede no existir */
    TAC_RET,      /* return [a] */
    TAC_HALT,     /* terminar */
    TAC_AUTH,     /* AUTH a */
    TAC_LOGOUT,   /* LOGOUT */
    TAC_RDSR,     /* d = SR */
    TAC_LOADKEY,  /* boveda[imm][imm2] = a */
    TAC_FROUND,   /* (d, d2) = FROUND(a, b, llave imm, ronda imm2) */
    TAC_CLEAR,    /* a = 0, no eliminable: borra una subllave del registro */
    TAC_N
} TacOp;

typedef enum TacBop {
    B_ADD, B_SUB, B_MUL, B_DIV, B_MOD, B_AND, B_OR, B_SHL,
    B_LT, B_LE, B_GT, B_GE, B_EQ, B_NE,
    U_NEG, U_NOT,
    B_N
} TacBop;

typedef enum { OP_NADA = 0, OP_V, OP_C, OP_R } OpndK;

typedef struct Opnd {
    OpndK k;
    long v;            /* numero de virtual, valor constante o registro fisico */
} Opnd;

typedef struct Tac {
    TacOp op;
    TacBop bop;
    int d, d2;         /* destinos virtuales (-1 si no hay) */
    Opnd a, b;
    long imm, imm2;
    char *etq;         /* etiqueta de LABEL/JMP/BCOND/JZ/JNZ o funcion de CALL */
    int linea;
} Tac;

typedef struct TacFunc {
    char *nombre;      /* "_inicio" o el nombre de la funcion JAF */
    Simbolo *sim;      /* NULL en _inicio */
    Tac *v;
    int n, cap;
    int nv;            /* cantidad de registros virtuales */
    char **vnombre;    /* nombre de la variable de cada virtual (NULL = temporal) */
    Simbolo **vsim;    /* simbolo promovido de cada virtual (NULL = temporal) */
    int *vparam;       /* indice de parametro del virtual o -1 */
} TacFunc;

typedef struct TacProg {
    TacFunc *f;
    int n;
} TacProg;

/* Genera el TAC de un programa ya analizado (y opcionalmente desenrollado). */
int tac_generar(Nodo *raiz, TacProg *p, const char *archivo);
void tac_liberar(TacProg *p);
void tac_imprimir(const TacProg *p, FILE *f);
void tac_imprimir_func(const TacFunc *fn, FILE *f);
void tac_texto(const TacFunc *fn, const Tac *t, char *buf, size_t n);
void tac_vtexto(const TacFunc *fn, int v, char *buf, size_t n);   /* nombre de un virtual */
/* Cantidad de instrucciones (sin etiquetas). */
long tac_contar(const TacProg *p);

/* Utilidades para las pasadas de optimizacion. */
int tac_nuevo_v(TacFunc *fn, const char *nombre);
Tac *tac_insertar(TacFunc *fn, int pos);       /* hueco vacio en pos */
void tac_borrar(TacFunc *fn, int pos);
/* Virtuales que lee / escribe una instruccion (hasta 3 lecturas, 2 escrituras). */
int tac_lee(const Tac *t, int *v);
int tac_escribe(const Tac *t, int *v);
/* Instrucciones que no se pueden eliminar aunque su resultado no se use. */
int tac_efecto(const Tac *t);
int tac_es_salto(const Tac *t);              /* JMP, BCOND, JZ, JNZ */
int tac_termina_bloque(const Tac *t);        /* salto, RET o HALT */
const char *tac_bop_texto(TacBop b);

#endif
