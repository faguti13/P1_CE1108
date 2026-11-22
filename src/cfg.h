#ifndef CFG_H
#define CFG_H

#include "ir_tac.h"

#include <stdint.h>
#include <stdio.h>

/*
 * Grafo de flujo de control de una funcion del TAC.
 * Un bloque basico empieza en una etiqueta, en la primera instruccion o despues
 * de un salto/return/terminar, y abarca v[ini..fin). Las llamadas no cortan el
 * bloque (el reordenamiento las trata como barreras).
 * Vivacidad por punto fijo hacia atras sobre conjuntos de bits de virtuales:
 *   out[B] = U in[S] (S sucesor de B);  in[B] = use[B] U (out[B] - def[B])
 * Lazos: arco de retorno u->h con h dominando a u; el cuerpo es el lazo natural.
 */

typedef struct Bloque {
    int ini, fin;
    int succ[2], nsucc;
    int *pred, npred;
    uint64_t *def, *use, *in, *out;
    int prof;          /* profundidad de anidamiento de lazos */
    int alcanzable;
} Bloque;

typedef struct Lazo {
    int cab, cola;     /* arco de retorno cola -> cab */
    int nbloques;
} Lazo;

typedef struct Cfg {
    TacFunc *fn;
    Bloque *b;
    int n;
    int palabras;      /* uint64_t por conjunto de virtuales */
    Lazo *lazos;
    int nlazos;
    int iter_vivos;    /* iteraciones del punto fijo */
} Cfg;

void cfg_construir(Cfg *g, TacFunc *fn);
void cfg_vivacidad(Cfg *g);
void cfg_liberar(Cfg *g);
void cfg_imprimir(const Cfg *g, FILE *f);
/* Bloque que contiene la instruccion i. */
int cfg_bloque_de(const Cfg *g, int i);

static inline int bs_tiene(const uint64_t *s, int i) { return (int)((s[i >> 6] >> (i & 63)) & 1u); }
static inline void bs_pon(uint64_t *s, int i) { s[i >> 6] |= (uint64_t)1 << (i & 63); }
static inline void bs_quita(uint64_t *s, int i) { s[i >> 6] &= ~((uint64_t)1 << (i & 63)); }

#endif