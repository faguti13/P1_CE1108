#ifndef IR_TAC_H
#define IR_TAC_H
#include "ast.h"
#include <stdio.h>

typedef enum {
    TAC_LABEL, TAC_COPY, TAC_BIN, TAC_UN, TAC_LOAD, TAC_STORE,
    TAC_JMP, TAC_JZ, TAC_PARAM, TAC_CALL, TAC_RET, TAC_HALT
} TipoTac;

typedef struct {
    TipoTac tipo;
    char *res, *arg1, *arg2, *op;
    int linea;
} Tac;

typedef struct {
    Tac *v;
    int n, cap;
} CodigoTac;

/* Construye codigo de tres direcciones desde el AST validado. */
void tac_generar(const Nodo *raiz, CodigoTac *codigo);
void tac_imprimir(const CodigoTac *codigo, FILE *salida);
void tac_liberar(CodigoTac *codigo);
#endif
