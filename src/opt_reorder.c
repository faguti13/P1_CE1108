#include "cfg.h"
#include "opt.h"

#include <stdlib.h>
#include <string.h>

/*
 * --rename: dentro de cada bloque basico, toda definicion de un virtual que no
 * sea la ultima del bloque recibe un virtual nuevo, y las lecturas siguientes
 * (hasta la proxima definicion) se reescriben. Rompe las dependencias WAW/WAR
 * entre usos sucesivos de una variable (tipico tras --unroll), y la asignacion
 * de registros puede darles registros distintos.
 * No se renombran los virtuales que borra un CLEAR (la subllave tiene que
 * borrarse del mismo registro que leyo LOADKEY).
 *
 * --reorder: list scheduling por bloque sobre el DAG de dependencias.
 * Prioridad: altura (camino mas largo hasta el final del tramo, RAW = 3 bundles,
 * que es la distancia del pipeline sin forwarding). Llamadas, intrinsecas
 * de boveda/Feistel y saltos son barreras: cortan el tramo.
 * Memoria: dos accesos con la misma base (sin redefinir en el tramo) y
 * desplazamientos distintos no se alias; gp y fp/sp son zonas disjuntas.
 */

/* ---------------- rename ---------------- */

// Hace: renombra temporales para reducir dependencias falsas; recibe: fn, e; genera: efectos sobre estructuras o salida.
void opt_rename(TacFunc *fn, EstadOpt *e)
{
    Cfg g;
    int i, k, j;
    int *mapa, *ultima;
    char *fijo;

    cfg_construir(&g, fn);
    mapa = malloc((size_t)(fn->nv + 1) * sizeof(int));
    ultima = malloc((size_t)(fn->nv + 1) * sizeof(int));
    fijo = calloc((size_t)fn->nv + 1, 1);
    for (k = 0; k < fn->n; k++) {
        if (fn->v[k].op == TAC_CLEAR) {
            fijo[fn->v[k].d] = 1;
        }
    }
    for (i = 0; i < g.n; i++) {
        Bloque *b = &g.b[i];
        int nv0 = fn->nv;
        for (j = 0; j < nv0; j++) {
            mapa[j] = j;
            ultima[j] = -1;
        }
        for (k = b->ini; k < b->fin; k++) {
            int esc[4], ne = tac_escribe(&fn->v[k], esc);
            for (j = 0; j < ne; j++) {
                ultima[esc[j]] = k;
            }
        }
        for (k = b->ini; k < b->fin; k++) {
            Tac *t = &fn->v[k];
            if (t->a.k == OP_V && t->a.v < nv0) {
                t->a.v = mapa[t->a.v];
            }
            if (t->b.k == OP_V && t->b.v < nv0) {
                t->b.v = mapa[t->b.v];
            }
            if (t->op == TAC_CLEAR) {
                continue;
            }
            if (t->d >= 0 && t->d < nv0) {
                int v = t->d;
                if (ultima[v] != k && !fijo[v]) {
                    int nuevo = tac_nuevo_v(fn, fn->vnombre[v]);
                    t = &fn->v[k];
                    mapa[v] = nuevo;
                    t->d = nuevo;
                    e->renombradas++;
                } else {
                    mapa[v] = v;
                }
            }
            if (t->d2 >= 0 && t->d2 < nv0) {
                int v = t->d2;
                if (ultima[v] != k && !fijo[v]) {
                    int nuevo = tac_nuevo_v(fn, fn->vnombre[v]);
                    t = &fn->v[k];
                    mapa[v] = nuevo;
                    t->d2 = nuevo;
                    e->renombradas++;
                } else {
                    mapa[v] = v;
                }
            }
        }
        if (fn->nv > nv0) {
            mapa = realloc(mapa, (size_t)(fn->nv + 1) * sizeof(int));
            ultima = realloc(ultima, (size_t)(fn->nv + 1) * sizeof(int));
            fijo = realloc(fijo, (size_t)fn->nv + 1);
            memset(fijo + nv0, 0, (size_t)(fn->nv - nv0) + 1);
        }
    }
    free(mapa);
    free(ultima);
    free(fijo);
    cfg_liberar(&g);
}

/* ---------------- reorder ---------------- */

// Hace: reconoce instrucciones que impiden reordenar; recibe: t; genera: valor o indicador calculado.
static int es_barrera(const Tac *t)
{
    switch (t->op) {
    case TAC_LABEL: case TAC_JMP: case TAC_BCOND: case TAC_JZ: case TAC_JNZ:
    case TAC_CALL: case TAC_RET: case TAC_HALT:
    case TAC_AUTH: case TAC_LOGOUT: case TAC_RDSR: case TAC_LOADKEY: case TAC_FROUND: case TAC_CLEAR:
        return 1;
    default:
        return 0;
    }
}

// Hace: identifica operaciones de memoria; recibe: t; genera: valor o indicador calculado.
static int es_mem(const Tac *t)
{
    return t->op == TAC_LOAD || t->op == TAC_STORE || t->op == TAC_PARAM;
}

// Hace: clasifica la region de memoria de una base; recibe: base; genera: valor o indicador calculado.
static int zona_de(Opnd base)
{
    if (base.k == OP_R && base.v == R_GP) return 1;
    if (base.k == OP_R && (base.v == R_FP || base.v == R_SP)) return 2;
    return 0;
}

// Hace: extrae la base de un acceso a memoria; recibe: t; genera: valor o indicador calculado.
static Opnd base_de(const Tac *t)
{
    if (t->op == TAC_PARAM) {
        Opnd o = {OP_R, R_SP};
        return o;
    }
    return t->a;
}

// Hace: extrae el desplazamiento de un acceso; recibe: t; genera: valor o indicador calculado.
static long off_de(const Tac *t)
{
    return t->op == TAC_PARAM ? -4L * (t->imm + 1) : t->imm;
}

/* Cierto si los dos accesos pueden tocar la misma palabra. */
// Hace: comprueba posible conflicto entre accesos; recibe: x, y, base_estable; genera: valor o indicador calculado.
static int alias(const Tac *x, const Tac *y, int base_estable)
{
    Opnd bx = base_de(x), by = base_de(y);
    int zx = zona_de(bx), zy = zona_de(by);
    if (zx && zy && zx != zy) {
        return 0;
    }
    if (bx.k == by.k && bx.v == by.v && base_estable) {
        long a = off_de(x), b = off_de(y);
        return !(a + 4 <= b || b + 4 <= a);
    }
    return 1;
}

typedef struct Nodo_ {
    int nsuc, *suc, *lat;
    int npred;
    int alto;
} NodoDag;

// Hace: agrega una dependencia al DAG; recibe: d, de, a, lat; genera: efectos sobre estructuras o salida.
static void arista(NodoDag *d, int de, int a, int lat)
{
    d[de].suc = realloc(d[de].suc, (size_t)(d[de].nsuc + 1) * sizeof(int));
    d[de].lat = realloc(d[de].lat, (size_t)(d[de].nsuc + 1) * sizeof(int));
    d[de].suc[d[de].nsuc] = a;
    d[de].lat[d[de].nsuc] = lat;
    d[de].nsuc++;
    d[a].npred++;
}

/* Cierto si la base (virtual) de x se redefine entre x e y. */
// Hace: detecta cambios en la base durante un tramo; recibe: v, x, y, base; genera: valor o indicador calculado.
static int base_cambia(const Tac *v, int x, int y, Opnd base)
{
    int k, j;
    if (base.k != OP_V) {
        return 0;
    }
    for (k = x; k < y; k++) {
        int esc[4], ne = tac_escribe(&v[k], esc);
        for (j = 0; j < ne; j++) {
            if (esc[j] == base.v) {
                return 1;
            }
        }
    }
    return 0;
}

// Hace: reordena instrucciones respetando dependencias; recibe: fn, ini, fin, e; genera: efectos sobre estructuras o salida.
static void ordenar_tramo(TacFunc *fn, int ini, int fin, EstadOpt *e)
{
    int n = fin - ini, i, j, k;
    NodoDag *d;
    int *orden, *hecho, *listo;
    Tac *copia;

    if (n < 2) {
        return;
    }
    d = calloc((size_t)n, sizeof(NodoDag));
    for (i = 0; i < n; i++) {
        Tac *x = &fn->v[ini + i];
        int lx[4], ex[4], nlx = tac_lee(x, lx), nex = tac_escribe(x, ex);
        for (j = i + 1; j < n; j++) {
            Tac *y = &fn->v[ini + j];
            int ly[4], ey[4], nly = tac_lee(y, ly), ney = tac_escribe(y, ey);
            int lat = -1, p, q;
            for (p = 0; p < nex; p++) {
                for (q = 0; q < nly; q++) {
                    if (ex[p] == ly[q]) lat = 3;                 /* RAW */
                }
                for (q = 0; q < ney; q++) {
                    if (ex[p] == ey[q] && lat < 1) lat = 1;      /* WAW */
                }
            }
            for (p = 0; p < nlx; p++) {
                for (q = 0; q < ney; q++) {
                    if (lx[p] == ey[q] && lat < 0) lat = 0;      /* WAR */
                }
            }
            if (es_mem(x) && es_mem(y) && (x->op == TAC_STORE || x->op == TAC_PARAM || y->op == TAC_STORE || y->op == TAC_PARAM)) {
                Opnd bx = base_de(x);
                int estable = !base_cambia(fn->v, ini + i, ini + j, bx);
                if (alias(x, y, estable) && lat < 1) {
                    lat = 1;
                }
            }
            if (lat >= 0) {
                arista(d, i, j, lat);
            }
        }
    }
    for (i = n - 1; i >= 0; i--) {
        int h = 0;
        for (k = 0; k < d[i].nsuc; k++) {
            int c = d[i].lat[k] + d[d[i].suc[k]].alto;
            if (c > h) h = c;
        }
        d[i].alto = h + (es_mem(&fn->v[ini + i]) ? 1 : 0);
    }
    orden = malloc((size_t)n * sizeof(int));
    hecho = calloc((size_t)n, sizeof(int));
    listo = malloc((size_t)n * sizeof(int));
    for (i = 0; i < n; i++) {
        listo[i] = d[i].npred;
    }
    for (k = 0; k < n; k++) {
        int mejor = -1;
        for (i = 0; i < n; i++) {
            if (!hecho[i] && listo[i] == 0 && (mejor < 0 || d[i].alto > d[mejor].alto)) {
                mejor = i;
            }
        }
        orden[k] = mejor;
        hecho[mejor] = 1;
        for (j = 0; j < d[mejor].nsuc; j++) {
            listo[d[mejor].suc[j]]--;
        }
    }
    copia = malloc((size_t)n * sizeof(Tac));
    memcpy(copia, &fn->v[ini], (size_t)n * sizeof(Tac));
    for (k = 0; k < n; k++) {
        fn->v[ini + k] = copia[orden[k]];
        if (orden[k] != k) {
            e->reordenadas++;
        }
    }
    for (i = 0; i < n; i++) {
        free(d[i].suc);
        free(d[i].lat);
    }
    free(d);
    free(orden);
    free(hecho);
    free(listo);
    free(copia);
}

// Hace: reordena tramos de TAC seguros; recibe: fn, e; genera: efectos sobre estructuras o salida.
void opt_reorder(TacFunc *fn, EstadOpt *e)
{
    int k = 0, ini = 0;
    for (k = 0; k <= fn->n; k++) {
        if (k == fn->n || es_barrera(&fn->v[k])) {
            ordenar_tramo(fn, ini, k, e);
            ini = k + 1;
        }
    }
}
