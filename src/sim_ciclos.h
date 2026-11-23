#ifndef SIM_CICLOS_H
#define SIM_CICLOS_H

#include "isa.h"

#include <stdio.h>

typedef struct EstadSim {
    long ciclos; /* memoria ideal (equivale a tb_top MODE=0) */
    long bundles; /* bundles que llegaron a EX */
    long instrucciones; /* instrucciones (no NOP) ejecutadas */
    long saltos_tomados;
    long lecturas, escrituras;
    long l1_fallos, l2_fallos;
    long ciclos_cache; /* estimacion con L1+L2 (tb_top MODE=3) */
    long riesgos; /* lecturas de un registro con escritura en vuelo */
    int terminado; /* llego a FIN */
    unsigned sr;
} EstadSim;

#define SIM_PEN_L2_HIT 8
#define SIM_PEN_L2_MISS 61

/* Corre el programa (ya resuelto con prog_resolver). mem: 64 KB de datos (NULL = interna). */
int sim_ciclos(const Programa *p, long max_ciclos, EstadSim *e, unsigned char *mem);
void sim_imprimir(const EstadSim *e, FILE *f);
void sim_json(const EstadSim *e, FILE *f);

#endif
