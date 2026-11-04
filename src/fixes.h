#ifndef FIXES_H
#define FIXES_H
#include <stdio.h>
/* Analisis conservador del texto: propone inserciones, nunca modifica archivos. */
void fixes_emitir_json(FILE *out, char **archivos, int n);
#endif
