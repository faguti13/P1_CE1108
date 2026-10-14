#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include "codegen.h"
#include "isa.h"

typedef struct EstadPlan {
    long instrucciones;
    long bundles;
    long slots_usados;
} EstadPlan;

/* Reparte el IR en bundles respetando latencias, WAW/WAR y orden de memoria. */
int planificar(const IR *ir, Programa *p, EstadPlan *est);

#endif
