#include "cfg.h"
#include "opt.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

static double ahora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

/*
 * Costo de derrame de una funcion con la asignacion actual: cada lectura o
 * escritura de un virtual en memoria, y cada guarda/recarga alrededor de un call,
 * pesa 8^profundidad del lazo que la contiene.
 */
static long costo_derrame(TacFunc *fn, const OptCfg *c)
{
    Asig a;
    EstadOpt basura;
    Cfg g;
    long costo = 0;
    int i, k, j, v;

    memset(&basura, 0, sizeof basura);
    opt_regalloc(fn, c, &a, &basura);
    cfg_construir(&g, fn);
    for (i = 0; i < g.n; i++) {
        long w = 1L << (3 * (g.b[i].prof < 5 ? g.b[i].prof : 5));
        for (k = g.b[i].ini; k < g.b[i].fin; k++) {
            int x[4], n = tac_lee(&fn->v[k], x);
            for (j = 0; j < n; j++) {
                costo += a.reg[x[j]] < 0 ? w : 0;
            }
            n = tac_escribe(&fn->v[k], x);
            for (j = 0; j < n; j++) {
                costo += a.reg[x[j]] < 0 ? w : 0;
            }
            if (fn->v[k].op == TAC_CALL) {
                for (v = 0; v < fn->nv; v++) {
                    if (a.reg[v] >= 0 && a.ini[v] < 2 * k && a.fin[v] > 2 * k + 1 && v != fn->v[k].d) {
                        costo += 2 * w;
                    }
                }
            }
        }
    }
    cfg_liberar(&g);
    asig_liberar(&a);
    return costo;
}

/*
 * Aplica rename o reorder. Con --spill la pasada se deshace si sube el costo de
 * derrame: el barrido lineal usa un intervalo sin huecos por virtual, asi que un
 * renombre dentro de un lazo o subir cargas puede derramar mas de lo que gana.
 */
static void con_guarda(TacFunc *fn, const OptCfg *c, EstadOpt *e, void (*pasada)(TacFunc *, EstadOpt *))
{
    EstadOpt local;
    Tac *copia;
    long antes;
    int nv0 = fn->nv, k;

    if (!c->spill) {
        pasada(fn, e);
        return;
    }
    memset(&local, 0, sizeof local);
    copia = malloc((size_t)(fn->n ? fn->n : 1) * sizeof(Tac));
    memcpy(copia, fn->v, (size_t)fn->n * sizeof(Tac));
    antes = costo_derrame(fn, c);
    pasada(fn, &local);
    if (costo_derrame(fn, c) > antes) {
        memcpy(fn->v, copia, (size_t)fn->n * sizeof(Tac));
        for (k = nv0; k < fn->nv; k++) {
            free(fn->vnombre[k]);
        }
        fn->nv = nv0;
    } else {
        e->reordenadas += local.reordenadas;
        e->renombradas += local.renombradas;
    }
    free(copia);
}

int opt_compilar(Nodo *raiz, const char *archivo, const OptCfg *c, IR *ir, EstadOpt *e, TacProg *tac_out)
{
    TacProg p;
    double t0 = ahora_ms();
    int i;

    if (c->unroll) {
        /* el conteo "antes" es el del programa sin desenrollar */
        if (tac_generar(raiz, &p, archivo) > 0) {
            return 1;
        }
        e->instrucciones_antes = tac_contar(&p);
        tac_liberar(&p);
        opt_unroll(raiz, c->unroll, e);
    }
    if (tac_generar(raiz, &p, archivo) > 0) {
        return 1;
    }
    if (!c->unroll) {
        e->instrucciones_antes = tac_contar(&p);
    }
    if (c->ver_ir_crudo) {
        printf("; TAC sin optimizar\n");
        tac_imprimir(&p, stdout);
    }
    for (i = 0; i < p.n; i++) {
        TacFunc *fn = &p.f[i];
        if (c->dce) {
            opt_dce(fn, e);
        }
        if (c->rename) {
            con_guarda(fn, c, e, opt_rename);
        }
        if (c->reorder) {
            con_guarda(fn, c, e, opt_reorder);
        }
        if (c->dce && (c->rename || c->reorder)) {
            opt_dce(fn, e);
        }
    }
    e->instrucciones_despues = tac_contar(&p);
    if (c->ver_ir) {
        printf("; TAC final\n");
        tac_imprimir(&p, stdout);
    }
    if (c->ver_cfg) {
        for (i = 0; i < p.n; i++) {
            Cfg g;
            cfg_construir(&g, &p.f[i]);
            cfg_vivacidad(&g);
            cfg_imprimir(&g, stdout);
            cfg_liberar(&g);
            putchar('\n');
        }
    }
    ir_bajar(&p, c, ir, e);
    e->tiempo_ms = ahora_ms() - t0;
    if (tac_out) {
        *tac_out = p;
    } else {
        tac_liberar(&p);
    }
    return 0;
}

void opt_imprimir_estad(const EstadOpt *e, FILE *f)
{
    fprintf(f, "[jaf] optimizacion: TAC %ld -> %ld instrucciones, %ld eliminadas por DCE\n", e->instrucciones_antes,
            e->instrucciones_despues, e->eliminadas_dce);
    fprintf(f, "[jaf]   unroll: %ld lazos, %ld copias extra del cuerpo\n", e->lazos_desenrollados, e->iteraciones_unroll);
    fprintf(f, "[jaf]   rename: %ld definiciones, reorder: %ld instrucciones movidas\n", e->renombradas,
            e->reordenadas);
    fprintf(f, "[jaf]   registros: %ld virtuales derramados, %ld cargas/guardas de derrame\n", e->virtuales_derramados,
            e->spills_insertados);
    fprintf(f, "[jaf]   %ld instrucciones de maquina, %.2f ms\n", e->ir_instrucciones, e->tiempo_ms);
}

void opt_estad_json(const EstadOpt *e, FILE *f)
{
    fprintf(f,
            "{\"instrucciones_antes\": %ld, \"instrucciones_despues\": %ld, \"eliminadas_dce\": %ld, "
            "\"lazos_desenrollados\": %ld, \"iteraciones_unroll\": %ld, \"renombradas\": %ld, \"reordenadas\": %ld, "
            "\"virtuales_derramados\": %ld, \"spills_insertados\": %ld, \"ir_instrucciones\": %ld, \"tiempo_ms\": %.3f}",
            e->instrucciones_antes, e->instrucciones_despues, e->eliminadas_dce, e->lazos_desenrollados,
            e->iteraciones_unroll, e->renombradas, e->reordenadas, e->virtuales_derramados, e->spills_insertados,
            e->ir_instrucciones, e->tiempo_ms);
}
