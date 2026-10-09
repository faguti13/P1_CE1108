#ifndef ENSAMBLADOR_H
#define ENSAMBLADOR_H

#include "isa.h"
#include <stddef.h>

/* Ensambla texto fuente (sintaxis de vliw_grupo2/docs/isa.md, adenda v1.1).
 * Devuelve 0 si va bien; si no, deja el mensaje en err. */
int ensamblar_texto(const char *texto, Programa *p, char *err, size_t n_err);

#endif
