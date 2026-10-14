#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "isa.h"

/* Codigo intermedio lineal: instrucciones ya con registros fisicos y etiquetas.
 * El planificador lo agrupa despues en bundles. */
typedef struct Item {
    int es_etq;
    Ins ins;
    char *etq;
} Item;

typedef struct IR {
    Item *v;
    int n, cap;
} IR;

/* Genera el IR de un programa ya analizado. Devuelve el numero de errores. */
int gen_programa(Nodo *raiz, IR *ir, const char *archivo);

#endif
