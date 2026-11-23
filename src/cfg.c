#include "cfg.h"

#include <stdlib.h>
#include <string.h>

// Hace: reserva memoria inicializada; recibe: n, t; genera: puntero o NULL segun corresponda.
static void *xcalloc(size_t n, size_t t)
{
    void *p = calloc(n ? n : 1, t ? t : 1);
    if (!p) {
        fprintf(stderr, "sin memoria\n");
        exit(1);
    }
    return p;
}

// Hace: encuentra el bloque de una etiqueta; recibe: g, etq; genera: valor o indicador calculado.
static int bloque_de_etq(const Cfg *g, const char *etq)
{
    int i;
    for (i = 0; i < g->n; i++) {
        const Tac *t = &g->fn->v[g->b[i].ini];
        int k;
        /* un bloque puede empezar con varias etiquetas seguidas */
        for (k = g->b[i].ini; k < g->b[i].fin && g->fn->v[k].op == TAC_LABEL; k++) {
            if (!strcmp(g->fn->v[k].etq, etq)) {
                return i;
            }
        }
        (void)t;
    }
    return -1;
}

// Hace: agrega un predecesor sin duplicarlo; recibe: b, p; genera: efectos sobre estructuras o salida.
static void agregar_pred(Bloque *b, int p)
{
    b->pred = realloc(b->pred, (size_t)(b->npred + 1) * sizeof(int));
    b->pred[b->npred++] = p;
}

// Hace: consulta el bloque de una instruccion; recibe: g, i; genera: estado o cantidad calculada.
int cfg_bloque_de(const Cfg *g, int i)
{
    int lo = 0, hi = g->n - 1;
    while (lo <= hi) {
        int m = (lo + hi) / 2;
        if (i < g->b[m].ini) {
            hi = m - 1;
        } else if (i >= g->b[m].fin) {
            lo = m + 1;
        } else {
            return m;
        }
    }
    return -1;
}

// Hace: identifica lazos en el grafo; recibe: g; genera: efectos sobre estructuras o salida.
static void lazos(Cfg *g)
{
    int n = g->n, w = (n + 63) / 64;
    uint64_t *dom = xcalloc((size_t)n * (size_t)w, sizeof(uint64_t));
    uint64_t *tmp = xcalloc((size_t)w, sizeof(uint64_t));
    int *pila = xcalloc((size_t)n, sizeof(int));
    char *en = xcalloc((size_t)n, 1);
    int i, k, cambio;

    /* alcanzables desde la entrada */
    if (n) {
        int sp = 0;
        pila[sp++] = 0;
        g->b[0].alcanzable = 1;
        while (sp) {
            int u = pila[--sp];
            for (k = 0; k < g->b[u].nsucc; k++) {
                int s = g->b[u].succ[k];
                if (!g->b[s].alcanzable) {
                    g->b[s].alcanzable = 1;
                    pila[sp++] = s;
                }
            }
        }
    }
    /* dominadores: dom[0] = {0}; dom[b] = {b} U interseccion de dom[pred] */
    for (i = 0; i < n; i++) {
        uint64_t *d = dom + (size_t)i * w;
        if (i == 0) {
            bs_pon(d, 0);
        } else {
            memset(d, 0xff, (size_t)w * sizeof(uint64_t));
        }
    }
    do {
        cambio = 0;
        for (i = 1; i < n; i++) {
            uint64_t *d = dom + (size_t)i * w;
            int primero = 1;
            if (!g->b[i].alcanzable) {
                continue;
            }
            for (k = 0; k < g->b[i].npred; k++) {
                int p = g->b[i].pred[k];
                int j;
                if (!g->b[p].alcanzable) {
                    continue;
                }
                for (j = 0; j < w; j++) {
                    tmp[j] = primero ? dom[(size_t)p * w + j] : (tmp[j] & dom[(size_t)p * w + j]);
                }
                primero = 0;
            }
            if (primero) {
                memset(tmp, 0, (size_t)w * sizeof(uint64_t));
            }
            bs_pon(tmp, i);
            if (memcmp(tmp, d, (size_t)w * sizeof(uint64_t))) {
                memcpy(d, tmp, (size_t)w * sizeof(uint64_t));
                cambio = 1;
            }
        }
    } while (cambio);

    /* arcos de retorno y lazo natural de cada uno */
    for (i = 0; i < n; i++) {
        if (!g->b[i].alcanzable) {
            continue;
        }
        for (k = 0; k < g->b[i].nsucc; k++) {
            int h = g->b[i].succ[k];
            int sp = 0, cuenta = 1, j;
            if (!bs_tiene(dom + (size_t)i * w, h)) {
                continue;
            }
            memset(en, 0, (size_t)n);
            en[h] = 1;
            if (!en[i]) {
                en[i] = 1;
                cuenta++;
                pila[sp++] = i;
            }
            while (sp) {
                int u = pila[--sp];
                for (j = 0; j < g->b[u].npred; j++) {
                    int p = g->b[u].pred[j];
                    if (!en[p] && g->b[p].alcanzable) {
                        en[p] = 1;
                        cuenta++;
                        pila[sp++] = p;
                    }
                }
            }
            for (j = 0; j < n; j++) {
                g->b[j].prof += en[j];
            }
            g->lazos = realloc(g->lazos, (size_t)(g->nlazos + 1) * sizeof(Lazo));
            g->lazos[g->nlazos].cab = h;
            g->lazos[g->nlazos].cola = i;
            g->lazos[g->nlazos].nbloques = cuenta;
            g->nlazos++;
        }
    }
    free(dom);
    free(tmp);
    free(pila);
    free(en);
}

// Hace: construye bloques y aristas del flujo; recibe: g, fn; genera: efectos sobre estructuras o salida.
void cfg_construir(Cfg *g, TacFunc *fn)
{
    int i, ini;
    memset(g, 0, sizeof *g);
    g->fn = fn;

    /* lideres */
    ini = 0;
    for (i = 0; i <= fn->n; i++) {
        int corta = i == fn->n;
        if (!corta && i > ini) {
            corta = fn->v[i].op == TAC_LABEL && fn->v[i - 1].op != TAC_LABEL;
            corta = corta || tac_termina_bloque(&fn->v[i - 1]);
        }
        if (corta && i > ini) {
            g->b = realloc(g->b, (size_t)(g->n + 1) * sizeof(Bloque));
            memset(&g->b[g->n], 0, sizeof(Bloque));
            g->b[g->n].ini = ini;
            g->b[g->n].fin = i;
            g->n++;
            ini = i;
        }
    }
    /* sucesores */
    for (i = 0; i < g->n; i++) {
        Tac *t = &fn->v[g->b[i].fin - 1];
        Bloque *b = &g->b[i];
        if (t->op == TAC_JMP) {
            int s = bloque_de_etq(g, t->etq);
            if (s >= 0) {
                b->succ[b->nsucc++] = s;
            }
        } else if (t->op == TAC_BCOND || t->op == TAC_JZ || t->op == TAC_JNZ) {
            int s = bloque_de_etq(g, t->etq);
            if (i + 1 < g->n) {
                b->succ[b->nsucc++] = i + 1;
            }
            if (s >= 0 && s != i + 1) {
                b->succ[b->nsucc++] = s;
            }
        } else if (t->op != TAC_RET && t->op != TAC_HALT) {
            if (i + 1 < g->n) {
                b->succ[b->nsucc++] = i + 1;
            }
        }
    }
    for (i = 0; i < g->n; i++) {
        int k;
        for (k = 0; k < g->b[i].nsucc; k++) {
            agregar_pred(&g->b[g->b[i].succ[k]], i);
        }
    }
    lazos(g);
}

// Hace: calcula conjuntos de variables vivas; recibe: g; genera: efectos sobre estructuras o salida.
void cfg_vivacidad(Cfg *g)
{
    TacFunc *fn = g->fn;
    int w = (fn->nv + 63) / 64;
    int i, k, j, cambio;
    uint64_t *nuevo;

    g->palabras = w;
    for (i = 0; i < g->n; i++) {
        Bloque *b = &g->b[i];
        free(b->def); free(b->use); free(b->in); free(b->out);
        b->def = xcalloc((size_t)w, sizeof(uint64_t));
        b->use = xcalloc((size_t)w, sizeof(uint64_t));
        b->in = xcalloc((size_t)w, sizeof(uint64_t));
        b->out = xcalloc((size_t)w, sizeof(uint64_t));
        for (k = b->ini; k < b->fin; k++) {
            int lee[4], esc[4], nl, ne;
            nl = tac_lee(&fn->v[k], lee);
            ne = tac_escribe(&fn->v[k], esc);
            for (j = 0; j < nl; j++) {
                if (!bs_tiene(b->def, lee[j])) {
                    bs_pon(b->use, lee[j]);
                }
            }
            for (j = 0; j < ne; j++) {
                bs_pon(b->def, esc[j]);
            }
        }
    }
    nuevo = xcalloc((size_t)w, sizeof(uint64_t));
    g->iter_vivos = 0;
    do {
        cambio = 0;
        g->iter_vivos++;
        for (i = g->n - 1; i >= 0; i--) {
            Bloque *b = &g->b[i];
            memset(nuevo, 0, (size_t)w * sizeof(uint64_t));
            for (k = 0; k < b->nsucc; k++) {
                uint64_t *s = g->b[b->succ[k]].in;
                for (j = 0; j < w; j++) {
                    nuevo[j] |= s[j];
                }
            }
            memcpy(b->out, nuevo, (size_t)w * sizeof(uint64_t));
            for (j = 0; j < w; j++) {
                uint64_t x = b->use[j] | (b->out[j] & ~b->def[j]);
                if (x != b->in[j]) {
                    b->in[j] = x;
                    cambio = 1;
                }
            }
        }
    } while (cambio);
    free(nuevo);
}

// Hace: libera las estructuras del grafo; recibe: g; genera: efectos sobre estructuras o salida.
void cfg_liberar(Cfg *g)
{
    int i;
    for (i = 0; i < g->n; i++) {
        free(g->b[i].pred);
        free(g->b[i].def);
        free(g->b[i].use);
        free(g->b[i].in);
        free(g->b[i].out);
    }
    free(g->b);
    free(g->lazos);
    memset(g, 0, sizeof *g);
}

// Hace: imprime un conjunto de virtuales; recibe: g, s, f; genera: efectos sobre estructuras o salida.
static void conjunto(const Cfg *g, const uint64_t *s, FILE *f)
{
    int v, primero = 1;
    char buf[64];
    fputc('{', f);
    if (s) {
        for (v = 0; v < g->fn->nv; v++) {
            if (bs_tiene(s, v)) {
                tac_vtexto(g->fn, v, buf, sizeof buf);
                fprintf(f, "%s%s", primero ? "" : ", ", buf);
                primero = 0;
            }
        }
    }
    fputc('}', f);
}

// Hace: imprime bloques y relaciones del CFG; recibe: g, f; genera: efectos sobre estructuras o salida.
void cfg_imprimir(const Cfg *g, FILE *f)
{
    int i, k;
    char buf[256];

    fprintf(f, "cfg %s: %d bloques, %d lazos, vivacidad en %d iteraciones\n", g->fn->nombre, g->n, g->nlazos,
            g->iter_vivos);
    for (i = 0; i < g->nlazos; i++) {
        fprintf(f, "  lazo: B%d -> B%d (%d bloques)\n", g->lazos[i].cola, g->lazos[i].cab, g->lazos[i].nbloques);
    }
    for (i = 0; i < g->n; i++) {
        const Bloque *b = &g->b[i];
        fprintf(f, "B%d%s prof=%d  pred:", i, b->alcanzable ? "" : " (inalcanzable)", b->prof);
        for (k = 0; k < b->npred; k++) {
            fprintf(f, " B%d", b->pred[k]);
        }
        fprintf(f, "  succ:");
        for (k = 0; k < b->nsucc; k++) {
            fprintf(f, " B%d", b->succ[k]);
        }
        fprintf(f, "\n  in  = ");
        conjunto(g, b->in, f);
        fprintf(f, "\n");
        for (k = b->ini; k < b->fin; k++) {
            tac_texto(g->fn, &g->fn->v[k], buf, sizeof buf);
            fprintf(f, g->fn->v[k].op == TAC_LABEL ? "  %s\n" : "      %s\n", buf);
        }
        fprintf(f, "  out = ");
        conjunto(g, b->out, f);
        fprintf(f, "\n");
    }
}
