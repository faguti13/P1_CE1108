#ifndef CFG_H
#define CFG_H
#include "ir_tac.h"
#include <stdio.h>

typedef struct {
    int inicio, fin; /* rango de instrucciones [inicio, fin] */
    int succ[2], nsucc;
} Bloque;
typedef struct {
    Bloque *v;
    int n;
} GrafoFlujo;

/* Lideres: primera instruccion, destinos de salto y siguiente de saltos/retornos. */
void cfg_construir(const CodigoTac *c, GrafoFlujo *g);
void cfg_imprimir(const CodigoTac *c, const GrafoFlujo *g, FILE *out);
void cfg_liberar(GrafoFlujo *g);
#endif
