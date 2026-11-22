#ifndef LINKER_H
#define LINKER_H

#include "ast.h"

/*
 * Enlazado de modulos (#import "otro.oly";). Se resuelve sobre el AST, antes del
 * analisis semantico: cada modulo se parsea una sola vez y sus elementos (funciones,
 * globales y codigo de nivel superior) se anteponen al cuerpo del programa en orden
 * de dependencias. Cada nodo conserva su archivo de origen para los diagnosticos.
 */

/* Resuelve los #import de `raiz` (fuente `ruta`). Devuelve la cantidad de errores. */
int enlazar(Nodo *raiz, const char *ruta, int verboso);

/* Modulos enlazados en la ultima llamada (sin contar el programa principal). */
int enlace_n_modulos(void);
const char *enlace_modulo(int i);

#endif
