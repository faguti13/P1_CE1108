#include "cfg.h"
#include <stdlib.h>
#include <string.h>

static int salto(TipoTac t) { return t == TAC_JMP || t == TAC_JZ; }
static int fin(TipoTac t) { return salto(t) || t == TAC_RET || t == TAC_HALT; }
static int buscar(const CodigoTac *c, const char *etiqueta) {
    int i;
    for (i = 0; i < c->n; i++)
        if (c->v[i].tipo == TAC_LABEL && !strcmp(c->v[i].res, etiqueta)) return i;
    return -1;
}
static void agregar(Bloque *b, int id) {
    int j;
    if (id < 0) return;
    for (j = 0; j < b->nsucc; j++) if (b->succ[j] == id) return;
    if (b->nsucc < 2) b->succ[b->nsucc++] = id;
}
void cfg_construir(const CodigoTac *c, GrafoFlujo *g) {
    int *lider, *mapa, i, j, k, n = c->n;
    memset(g, 0, sizeof *g);
    if (!n) return;
    lider = calloc((size_t)n, sizeof *lider);
    mapa = malloc((size_t)n * sizeof *mapa);
    lider[0] = 1;
    for (i = 0; i < n; i++) {
        if (salto(c->v[i].tipo)) {
            j = buscar(c, c->v[i].res);
            if (j >= 0) lider[j] = 1;
        }
        if (fin(c->v[i].tipo) && i + 1 < n) lider[i+1] = 1;
        /* Una funcion o etiqueta de destino también puede empezar bloque. */
        if (c->v[i].tipo == TAC_LABEL) lider[i] = 1;
    }
    for (i = 0; i < n; i++) if (lider[i]) g->n++;
    g->v = calloc((size_t)g->n, sizeof *g->v);
    k = -1;
    for (i = 0; i < n; i++) {
        if (lider[i]) {
            k++;
            g->v[k].inicio = i;
            if (k) g->v[k-1].fin = i - 1;
        }
        mapa[i] = k;
    }
    g->v[k].fin = n - 1;
    for (i = 0; i < g->n; i++) {
        Bloque *b = &g->v[i];
        const Tac *t = &c->v[b->fin];
        if (salto(t->tipo)) {
            j = buscar(c, t->res);
            if (j >= 0) agregar(b, mapa[j]);
        }
        if (t->tipo != TAC_JMP && t->tipo != TAC_RET && t->tipo != TAC_HALT
            && i + 1 < g->n)
            agregar(b, i+1);
    }
    free(lider); free(mapa);
}
void cfg_imprimir(const CodigoTac *c, const GrafoFlujo *g, FILE *out) {
    int i, j;
    fprintf(out, "Bloques basicos: %d\n", g->n);
    for (i = 0; i < g->n; i++) {
        const Bloque *b = &g->v[i];
        fprintf(out, "\nB%d (instrucciones %d..%d):\n", i, b->inicio, b->fin);
        for (j = b->inicio; j <= b->fin; j++) {
            CodigoTac uno = { (Tac *)&c->v[j], 1, 1 };
            fprintf(out, "    [%d] ", j);
            /* Se reutiliza el impresor de TAC sin duplicar la logica. */
            { FILE *tmp = tmpfile();
              if (tmp) {
                  int ch;
                  tac_imprimir(&uno, tmp); rewind(tmp);
                  /* El impresor usa indice 0 cuando se le pasa una sola
                     instruccion; quitamos ese prefijo para no confundirlo
                     con el indice real del programa. */
                  fgetc(tmp); fgetc(tmp); fgetc(tmp); fgetc(tmp); fgetc(tmp);
                  while ((ch = fgetc(tmp)) != EOF) fputc(ch, out);
                  fclose(tmp);
              }
            }
        }
        fprintf(out, "  sucesores:");
        if (!b->nsucc) fprintf(out, " ninguno");
        for (j = 0; j < b->nsucc; j++) fprintf(out, " B%d", b->succ[j]);
        fputc('\n', out);
    }
}
void cfg_liberar(GrafoFlujo *g) { free(g->v); memset(g, 0, sizeof *g); }
