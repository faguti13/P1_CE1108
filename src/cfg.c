#include "cfg.h"
#include <stdlib.h>
#include <string.h>

/* Revisa si la instruccion es un salto, ya sea directo o condicional. */
static int salto(TipoTac t) { return t == TAC_JMP || t == TAC_JZ; }

/* Revisa si una instruccion termina un bloque basico. */
static int fin(TipoTac t) { return salto(t) || t == TAC_RET || t == TAC_HALT; }

/* Busca una etiqueta en el TAC y devuelve el numero de instruccion donde aparece. */
static int buscar(const CodigoTac *c, const char *etiqueta) {
    int i;
    for (i = 0; i < c->n; i++)
        if (c->v[i].tipo == TAC_LABEL && !strcmp(c->v[i].res, etiqueta)) return i;
    return -1; // Si no encontro la etiqueta, devuelve -1.
}

/* Agrega un bloque sucesor, sin repetirlo y con un maximo de dos sucesores. */
static void agregar(Bloque *b, int id) {
    int j;
    if (id < 0) return; // No agrega bloques con un numero invalido.
    for (j = 0; j < b->nsucc; j++) if (b->succ[j] == id) return; // Evita repetir sucesores.
    if (b->nsucc < 2) b->succ[b->nsucc++] = id; // Guarda el sucesor y aumenta la cantidad.
}

/* Recibe el TAC y construye sus bloques basicos y las conexiones entre ellos.
   Primero identifica los lideres, despues forma los bloques y al final sus sucesores. */
void cfg_construir(const CodigoTac *c, GrafoFlujo *g) {
    int *lider, *mapa, i, j, k, n = c->n;
    /* lider[i] vale 1 si la instruccion i empieza un bloque.
       mapa[i] indica a cual bloque pertenece la instruccion i. */
    memset(g, 0, sizeof *g); // Inicializa el grafo vacio.
    if (!n) return; // Si no hay TAC, no hay bloques que construir.
    lider = calloc((size_t)n, sizeof *lider); // Empieza con todos los lideres en 0.
    mapa = malloc((size_t)n * sizeof *mapa); // Reserva el arreglo para identificar cada bloque.

    /* Paso 1: identificar los lideres segun las reglas de la teoria. */
    lider[0] = 1; // La primera instruccion siempre es lider.
    for (i = 0; i < n; i++) {
        if (salto(c->v[i].tipo)) {
            j = buscar(c, c->v[i].res); // Busca donde esta la etiqueta del salto.
            if (j >= 0) lider[j] = 1; // El destino de un salto es lider.
        }
        if (fin(c->v[i].tipo) && i + 1 < n) lider[i+1] = 1; // La instruccion siguiente tambien es lider.
        /* Una funcion o etiqueta de destino tambien puede empezar bloque. */
        if (c->v[i].tipo == TAC_LABEL) lider[i] = 1;
    }

    /* Paso 2: contar los lideres y reservar espacio para los bloques. */
    for (i = 0; i < n; i++) if (lider[i]) g->n++; // Cada lider inicia un bloque nuevo.
    g->v = calloc((size_t)g->n, sizeof *g->v);
    k = -1; // Todavia no se ha creado ningun bloque.
    for (i = 0; i < n; i++) {
        if (lider[i]) {
            k++; // Empieza un bloque nuevo.
            g->v[k].inicio = i; // Guarda su primera instruccion.
            if (k) g->v[k-1].fin = i - 1; // El bloque anterior termina antes de este lider.
        }
        mapa[i] = k; // Guarda a cual bloque pertenece esta instruccion.
    }
    g->v[k].fin = n - 1; // El ultimo bloque termina en la ultima instruccion TAC.

    /* Paso 3: identificar a que bloques se puede pasar desde cada bloque.
       Solo se revisa su ultima instruccion, porque dentro del bloque no hay saltos. */
    for (i = 0; i < g->n; i++) {
        Bloque *b = &g->v[i]; // Bloque que estamos analizando.
        const Tac *t = &c->v[b->fin]; // Ultima instruccion de ese bloque.
        if (salto(t->tipo)) {
            j = buscar(c, t->res); // Busca el destino del salto en el TAC.
            if (j >= 0) agregar(b, mapa[j]); // Convierte esa instruccion en numero de bloque.
        }
        /* Si no es un salto directo, retorno o terminar, tambien puede seguir
           al bloque que aparece inmediatamente despues. */
        if (t->tipo != TAC_JMP && t->tipo != TAC_RET && t->tipo != TAC_HALT
            && i + 1 < g->n)
            agregar(b, i+1);
    }
    free(lider); free(mapa); // Ya no se necesitan los arreglos auxiliares.
}

/* Imprime cada bloque, las instrucciones TAC que contiene y sus sucesores. */
void cfg_imprimir(const CodigoTac *c, const GrafoFlujo *g, FILE *out) {
    int i, j;
    fprintf(out, "Bloques basicos: %d\n", g->n);
    for (i = 0; i < g->n; i++) {
        const Bloque *b = &g->v[i]; // Obtiene el bloque actual.
        fprintf(out, "\nB%d (instrucciones %d..%d):\n", i, b->inicio, b->fin);
        for (j = b->inicio; j <= b->fin; j++) { // Recorre las instrucciones del bloque.
            CodigoTac uno = { (Tac *)&c->v[j], 1, 1 }; // TAC temporal con una sola instruccion.
            fprintf(out, "    [%d] ", j); // Imprime el indice real de la instruccion.
            /* Se reutiliza cfg_imprimir de TAC sin duplicar la logica. */
            { FILE *tmp = tmpfile();
              if (tmp) {
                  int ch;
                  tac_imprimir(&uno, tmp); rewind(tmp); // Escribe y vuelve al inicio del archivo temporal.
                  /*cfg_imprimir usa indice 0 cuando se le pasa una sola
                     instruccion; quitamos ese prefijo para no confundirlo
                     con el indice real del programa. */
                  fgetc(tmp); fgetc(tmp); fgetc(tmp); fgetc(tmp); fgetc(tmp);
                  while ((ch = fgetc(tmp)) != EOF) fputc(ch, out); // Imprime el resto de la instruccion.
                  fclose(tmp); // Cierra el archivo temporal.
              }
            }
        }
        fprintf(out, "  sucesores:");
        if (!b->nsucc) fprintf(out, " ninguno"); // El bloque no tiene sucesores.
        for (j = 0; j < b->nsucc; j++) fprintf(out, " B%d", b->succ[j]); // Imprime cada bloque sucesor.
        fputc('\n', out);
    }
}

/* Libera el arreglo de bloques y deja el grafo vacio. */
void cfg_liberar(GrafoFlujo *g) { free(g->v); memset(g, 0, sizeof *g); }
