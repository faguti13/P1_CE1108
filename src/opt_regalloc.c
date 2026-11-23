#include "cfg.h"
#include "opt.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 * Asignacion de registros.
 *   Sin --spill (-O0): todos los virtuales viven en memoria. Una variable usa la
 *   celda que le dio el semantico (fp+offset); un parametro, la celda donde llego
 *   (asi su carga inicial desaparece); un temporal, un hueco nuevo debajo de los
 *   locales.
 *   Con --spill: barrido lineal sobre r8..r(8+N-1). El intervalo de cada virtual
// Hace: procesa los datos indicados; recibe: fn, a, v; genera: puntero o NULL segun corresponda.
 *   va de su primera a su ultima posicion viva (vivacidad del CFG). Si no hay
 *   registro libre se derrama el intervalo que termina mas lejos. Los registros
 *   se reutilizan en el orden en que se liberaron (menos WAR/WAW juntos para el
 *   planificador).
 *   Las llamadas pisan r8..r23: cada virtual en registro que vive a traves de un
 *   call se guarda en su hogar antes y se recarga despues (caller-save).
 */

#define R_PRIMERO 8

long asig_hogar(TacFunc *fn, Asig *a, int v)
{
    Simbolo *s;
    if (a->slot[v]) {
        return a->slot[v];
    }
    s = fn->vsim[v];
    if (s && (s->alm == AL_LOCAL || s->alm == AL_PARAM)) {
        a->slot[v] = s->offset;
    } else {
        a->nslots++;
        a->slot[v] = -(a->zona + 4L * a->nslots);
    }
    a->marco = a->zona + 4L * a->nslots;
    return a->slot[v];
}

// Hace: libera la asignacion de registros; recibe: a; genera: efectos sobre estructuras o salida.
void asig_liberar(Asig *a)
{
    free(a->reg);
    free(a->slot);
    free(a->ini);
    free(a->fin);
    memset(a, 0, sizeof *a);
}

static int *ord_ini;

// Hace: compara intervalos por su inicio; recibe: x, y; genera: valor o indicador calculado.
static int cmp_ini(const void *x, const void *y)
{
    int a = *(const int *)x, b = *(const int *)y;
    if (ord_ini[a] != ord_ini[b]) {
        return ord_ini[a] < ord_ini[b] ? -1 : 1;
    }
    return a - b;
}

// Hace: calcula rangos vivos de virtuales; recibe: fn, a; genera: efectos sobre estructuras o salida.
static void intervalos(TacFunc *fn, Asig *a)
{
    Cfg g;
    int i, k, v, j;

    cfg_construir(&g, fn);
    cfg_vivacidad(&g);
    for (v = 0; v < fn->nv; v++) {
        a->ini[v] = INT_MAX;
        a->fin[v] = -1;
    }
#define TOCA(vv, p)                                    \
    do {                                               \
        if ((p) < a->ini[vv]) a->ini[vv] = (p);        \
        if ((p) > a->fin[vv]) a->fin[vv] = (p);        \
    } while (0)
    for (i = 0; i < g.n; i++) {
        Bloque *b = &g.b[i];
        for (v = 0; v < fn->nv; v++) {
            if (bs_tiene(b->in, v)) {
                TOCA(v, 2 * b->ini);
            }
            if (bs_tiene(b->out, v)) {
                TOCA(v, 2 * b->fin - 1);
            }
        }
        for (k = b->ini; k < b->fin; k++) {
            int lee[4], esc[4];
            int nl = tac_lee(&fn->v[k], lee);
            int ne = tac_escribe(&fn->v[k], esc);
            for (j = 0; j < nl; j++) {
                TOCA(lee[j], 2 * k);
            }
            for (j = 0; j < ne; j++) {
                TOCA(esc[j], 2 * k + 1);
            }
        }
    }
#undef TOCA
    cfg_liberar(&g);
}

// Hace: asigna registros por barrido lineal; recibe: fn, c, a, e; genera: efectos sobre estructuras o salida.
static void barrido(TacFunc *fn, const OptCfg *c, Asig *a, EstadOpt *e)
{
    int nreg = c->max_regs > 0 && c->max_regs <= 16 ? c->max_regs : 16;
    int *orden = malloc((size_t)(fn->nv + 1) * sizeof(int));
    int *activo = malloc((size_t)(fn->nv + 1) * sizeof(int));
    int cola[16], ncola = 0, nact = 0, n = 0;
    int i, k, v;

    for (i = 0; i < nreg; i++) {
        cola[ncola++] = R_PRIMERO + i;
    }
    for (v = 0; v < fn->nv; v++) {
        if (a->fin[v] >= 0) {
            orden[n++] = v;
        }
    }
    ord_ini = a->ini;
    qsort(orden, (size_t)n, sizeof(int), cmp_ini);

    for (i = 0; i < n; i++) {
        v = orden[i];
        /* expira en orden de fin: el registro liberado antes se reusa antes */
        while (1) {
            int mejor = -1;
            for (k = 0; k < nact; k++) {
                if (a->fin[activo[k]] < a->ini[v] && (mejor < 0 || a->fin[activo[k]] < a->fin[activo[mejor]])) {
                    mejor = k;
                }
            }
            if (mejor < 0) {
                break;
            }
            cola[ncola++] = a->reg[activo[mejor]];
            activo[mejor] = activo[--nact];
        }
        if (ncola > 0) {
            a->reg[v] = cola[0];
            memmove(cola, cola + 1, (size_t)(--ncola) * sizeof(int));
            activo[nact++] = v;
        } else {
            int lejos = 0;
            for (k = 1; k < nact; k++) {
                if (a->fin[activo[k]] > a->fin[activo[lejos]]) {
                    lejos = k;
                }
            }
            if (nact > 0 && a->fin[activo[lejos]] > a->fin[v]) {
                int w = activo[lejos];
                a->reg[v] = a->reg[w];
                a->reg[w] = -1;
                activo[lejos] = v;
                e->virtuales_derramados++;
            } else {
                a->reg[v] = -1;
                e->virtuales_derramados++;
            }
        }
    }
    free(orden);
    free(activo);
}

// Hace: prepara y ejecuta asignacion o derrame; recibe: fn, c, a, e; genera: efectos sobre estructuras o salida.
void opt_regalloc(TacFunc *fn, const OptCfg *c, Asig *a, EstadOpt *e)
{
    int v, k, nv = fn->nv;

    memset(a, 0, sizeof *a);
    a->reg = malloc((size_t)(nv + 1) * sizeof(int));
    a->slot = calloc((size_t)(nv + 1), sizeof(long));
    a->ini = malloc((size_t)(nv + 1) * sizeof(int));
    a->fin = malloc((size_t)(nv + 1) * sizeof(int));
    a->zona = fn->sim ? fn->sim->zona_derrame : 0;
    a->marco = a->zona;
    for (v = 0; v < nv; v++) {
        a->reg[v] = -1;
    }
    intervalos(fn, a);
    if (c->spill) {
        barrido(fn, c, a, e);
    }
    /* hogares: derramados y los que cruzan una llamada */
    for (v = 0; v < nv; v++) {
        if (a->fin[v] >= 0 && a->reg[v] < 0) {
            asig_hogar(fn, a, v);
        }
    }
    for (k = 0; k < fn->n; k++) {
        if (fn->v[k].op != TAC_CALL) {
            continue;
        }
        for (v = 0; v < nv; v++) {
            if (a->reg[v] >= 0 && a->ini[v] < 2 * k && a->fin[v] > 2 * k + 1 && v != fn->v[k].d) {
                asig_hogar(fn, a, v);
            }
        }
    }
}
