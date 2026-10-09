#ifndef EMISOR_H
#define EMISOR_H

#include "isa.h"
#include <stdio.h>

/* Escribe el programa en ensamblador: una linea por bundle, slots separados por '|'. */
void emitir_asm(const Programa *p, FILE *f, const char *origen);

/* Vista -p: direccion, instrucciones y las 4 palabras de 32 bits. */
void emitir_paquetes(const Programa *p, const uint8_t *code, FILE *f);

#endif
