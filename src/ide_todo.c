/*
 * ide_todo.c - `jaf --ide-todo`: todo lo que pintan las vistas del IDE en una
 * sola invocacion (diagnosticos, tabla de simbolos, AST, IR, bundles y
 * metricas del generador de P1 contra la tuberia optimizadora).
 *
 * El mismo AST se compila dos veces: primero con gen_programa (base) y luego
 * con opt_compilar, que puede desenrollarlo; por eso la base va primero.
 *
 * La tuberia optimizadora (ir_tac, opt_*, sim_ciclos) es del Integrante 4.
 * Mientras esos modulos no esten en el repo, JAF_IDE_CON_OPT vale 0 y este
 * archivo solo llena la parte de P1: "opt" sale con ok=false, el TAC queda
 * vacio y en su lugar va el IR lineal de P1 (ir.p1). Cuando entren opt.h,
 * ir_tac.h y sim_ciclos.h, la parte optimizada se compila sola sin tocar este
 * archivo (solo falta pasar &oc desde main.c).
 */

#include "ide.h"

#include "ast.h"
#include "codegen.h"
#include "planificador.h"
#include "semantico.h"

#if defined(__has_include)
#if __has_include("opt.h") && __has_include("ir_tac.h") && __has_include("sim_ciclos.h")
#define JAF_IDE_CON_OPT 1
#endif
#endif
#ifndef JAF_IDE_CON_OPT
#define JAF_IDE_CON_OPT 0
#endif

#if JAF_IDE_CON_OPT
#include "ir_tac.h"
#include "opt.h"
#include "sim_ciclos.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IDE_MAX_CICLOS 20000000L
#define IDE_MAX_BUNDLES 4000
#define IDE_MAX_IR 20000

typedef struct {
    int ok;
    char error[256];
    Programa p;
    EstadPlan plan;
#if JAF_IDE_CON_OPT
    EstadSim sim;
#endif
    double ms;
} Corrida;

static double ahora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

/* Planifica, resuelve y valida la codificacion; luego simula (si hay simulador). */
static void terminar_corrida(Corrida *c, const IR *ir, double t0)
{
    uint8_t *code;

    planificar(ir, &c->p, &c->plan);
    c->ms = ahora_ms() - t0;
    if (prog_resolver(&c->p, c->error, sizeof c->error)) {
        return;
    }
    code = calloc((size_t)(c->p.n ? c->p.n : 1), 16);
    if (prog_codificar(&c->p, code, c->error, sizeof c->error)) {
        free(code);
        return;
    }
    free(code);
#if JAF_IDE_CON_OPT
    sim_ciclos(&c->p, IDE_MAX_CICLOS, &c->sim, NULL);
#endif
    c->ok = 1;
}

/* eo es const void* para no depender de EstadOpt cuando no hay tuberia. */
static void corrida_json(FILE *f, const Corrida *c, const void *eo)
{
    if (!c->ok) {
        fputs("{\"ok\": false, \"error\": ", f);
        ide_json_str(f, c->error);
        fputc('}', f);
        return;
    }
    fprintf(f,
            "{\"ok\": true, \"bundles\": %ld, \"instrucciones\": %ld, \"slots_usados\": %ld, \"bytes\": %d, "
            "\"tiempo_ms\": %.3f",
            c->plan.bundles, c->plan.instrucciones, c->plan.slots_usados, c->p.n * 16, c->ms);
#if JAF_IDE_CON_OPT
    fputs(", \"sim\": ", f);
    sim_json(&c->sim, f);
    if (eo) {
        fputs(", \"estad\": ", f);
        opt_estad_json((const EstadOpt *)eo, f);
    }
#else
    (void)eo;
#endif
    fputc('}', f);
}

static void paquetes_json(FILE *f, const Corrida *c)
{
    int i, s, k, n;

    fputc('[', f);
    n = c->ok ? c->p.n : 0;
    if (n > IDE_MAX_BUNDLES) {
        n = IDE_MAX_BUNDLES;
    }
    for (i = 0; i < n; i++) {
        const Bundle *b = &c->p.b[i];
        char buf[160];
        fprintf(f, "%s{\"dir\": %ld, \"etq\": [", i ? ",\n " : "\n ", b->dir);
        for (k = 0; k < b->n_etq; k++) {
            if (k) {
                fputs(", ", f);
            }
            ide_json_str(f, b->etiquetas[k]);
        }
        fputs("], \"slots\": [", f);
        for (s = 0; s < 4; s++) {
            buf[0] = '\0';
            if (b->usado[s]) {
                ins_texto(&b->ins[s], buf, sizeof buf);
            }
            if (s) {
                fputs(", ", f);
            }
            ide_json_str(f, buf);
        }
        fputs("], \"lineas\": [", f);
        for (s = 0; s < 4; s++) {
            fprintf(f, "%s%d", s ? ", " : "", b->usado[s] ? b->ins[s].linea : 0);
        }
        fputs("]}", f);
    }
    fputs(n ? "\n]" : "]", f);
}

/* IR lineal del generador de P1 (antes de planificar): una instruccion por
   linea con sangria de 4 espacios y las etiquetas en su propia linea. */
static void ir_p1_texto(FILE *f, const IR *ir)
{
    int i;
    char buf[160];

    for (i = 0; i < ir->n && i < IDE_MAX_IR; i++) {
        const Item *it = &ir->v[i];
        if (it->es_etq) {
            fprintf(f, "%s:\n", it->etq ? it->etq : "?");
        } else {
            ins_texto(&it->ins, buf, sizeof buf);
            if (it->ins.linea > 0) {
                fprintf(f, "    %-40s ; oly:%d\n", buf, it->ins.linea);
            } else {
                fprintf(f, "    %s\n", buf);
            }
        }
    }
    if (ir->n > IDE_MAX_IR) {
        fprintf(f, "    ; ... %d instrucciones mas\n", ir->n - IDE_MAX_IR);
    }
}

/* Texto de un FILE* de open_memstream como cadena JSON; *buf es valido tras fclose. */
static void flujo_json(FILE *f, char **buf, FILE *flujo)
{
    fclose(flujo);
    ide_json_str(f, *buf);
    free(*buf);
}

int ide_todo(Nodo *raiz, const char *ruta, const struct OptCfg *oc)
{
    char *ast_buf = NULL, *antes_buf = NULL, *despues_buf = NULL, *p1_buf = NULL;
    size_t ast_n = 0, antes_n = 0, despues_n = 0, p1_n = 0;
    FILE *ast_f = open_memstream(&ast_buf, &ast_n);
    FILE *antes_f = open_memstream(&antes_buf, &antes_n);
    FILE *despues_f = open_memstream(&despues_buf, &despues_n);
    FILE *p1_f = open_memstream(&p1_buf, &p1_n);
    Corrida base, opt;
    int sem_ok = 0;
    char *archivos[1];
#if JAF_IDE_CON_OPT
    EstadOpt eo;
    memset(&eo, 0, sizeof eo);
#else
    (void)oc;
#endif

    memset(&base, 0, sizeof base);
    memset(&opt, 0, sizeof opt);
    strcpy(base.error, "el programa tiene errores");
    strcpy(opt.error, base.error);

    if (raiz) {
        nodo_json(ast_f, raiz);
        sem_ok = sem_analizar(raiz, ruta) == 0;
    } else {
        fputs("null", ast_f);
    }
    if (sem_ok) {
        IR ir;
        double t0 = ahora_ms();

        memset(&ir, 0, sizeof ir);
        if (gen_programa(raiz, &ir, ruta) == 0) {
            ir_p1_texto(p1_f, &ir);
            terminar_corrida(&base, &ir, t0);
        }
#if JAF_IDE_CON_OPT
        {
            TacProg tp;
            OptCfg por_defecto;

            if (!oc) {
                memset(&por_defecto, 0, sizeof por_defecto);
                por_defecto.activa = 1;
                por_defecto.max_regs = 16;
                oc = &por_defecto;
            }
            if (tac_generar(raiz, &tp, ruta) == 0) {
                tac_imprimir(&tp, antes_f);
                tac_liberar(&tp);
            }
            memset(&ir, 0, sizeof ir);
            t0 = ahora_ms();
            if (opt_compilar(raiz, ruta, oc, &ir, &eo, &tp) == 0) {
                tac_imprimir(&tp, despues_f);
                tac_liberar(&tp);
                terminar_corrida(&opt, &ir, t0);
            }
        }
#else
        strcpy(opt.error, "la tuberia optimizadora (-O0/-O1/-O2) no esta en esta version");
#endif
    }

    archivos[0] = (char *)ruta;
    fputc('{', stdout);
    ide_emitir_campos(stdout, archivos, 1);
    fputs(",\n  \"simbolos\": ", stdout);
    ide_simbolos_json(stdout);
    fputs(",\n  \"ast\": ", stdout);
    fclose(ast_f);
    fputs(ast_buf && ast_buf[0] ? ast_buf : "null", stdout);
    free(ast_buf);
    fputs(",\n  \"ir\": {\"antes\": ", stdout);
    flujo_json(stdout, &antes_buf, antes_f);
    fputs(", \"despues\": ", stdout);
    flujo_json(stdout, &despues_buf, despues_f);
    fputs(", \"p1\": ", stdout);
    flujo_json(stdout, &p1_buf, p1_f);
    fprintf(stdout, ", \"con_opt\": %s", JAF_IDE_CON_OPT ? "true" : "false");
    fputs("},\n  \"paquetes\": {\"base\": ", stdout);
    paquetes_json(stdout, &base);
    fputs(", \"opt\": ", stdout);
    paquetes_json(stdout, &opt);
    fputs("},\n  \"metricas\": {\"base\": ", stdout);
    corrida_json(stdout, &base, NULL);
    fputs(", \"opt\": ", stdout);
#if JAF_IDE_CON_OPT
    corrida_json(stdout, &opt, &eo);
#else
    corrida_json(stdout, &opt, NULL);
#endif
    fputs("}\n}\n", stdout);
    fflush(stdout);
    return 0;
}
