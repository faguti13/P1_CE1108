#ifndef CLONES_H
#define CLONES_H

#include <stdio.h>

/*
 * Deteccion de clones Tipo I / Tipo II entre archivos .oly.
 *
 * Tipo I: misma secuencia de tokens y lexemas.
 * Tipo II: misma secuencia de tipos de token (IDENT/literales normalizados).
 *
 * Emite un array JSON al estilo del contrato en ide.h:
 *   [ {"type":1|2, "a":{"file","line","end"}, "b":{...}}, ... ]
 */
void clones_emitir_json(FILE *f, char **archivos, int n);

#endif
