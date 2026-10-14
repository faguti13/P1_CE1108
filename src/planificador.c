#include "planificador.h"

#include <stdlib.h>
#include <string.h>

/*
 * Planificador de bundles (docs/04_generacion_codigo.md).
 *
 * El procesador no detecta ni resuelve dependencias: una escritura hecha
 * en el bundle i solo se lee desde el bundle i+3 (sin forwarding ni
 * stalls). El planificador reparte las instrucciones de cada segmento en
 * bundles de 4 slots respetando:
 *   RAW  lectura >= escritura + 3
 *   WAW  la segunda escritura en un bundle estrictamente posterior
 *   WAR  la escritura va en el mismo bundle de la ultima lectura o despues
 *        (hardware: una lectura con escritura en vuelo no se admite en el contrato)
 *   LSU  una sola instruccion de memoria por bundle, en orden de programa
 *   CRY  una sola instruccion del slot 3 por bundle, en orden de programa
 *
 * Un segmento termina en una etiqueta, un salto, una llamada, un retorno
 * o FIN. Reglas de frontera:
 *   - salto condicional: 2 bundles vacios detras (se ejecutan si no salta)
 *   - salto incondicional, retorno, FIN: nada detras (los 2 bundles se descartan)
 *   - llamada: nada detras; el codigo siguiente se ejecuta al volver
 *   - paso a una etiqueta sin salto: se rellena con bundles vacios hasta que
 *     todas las escrituras pendientes sean visibles (regla de drenado)
 */

#define NADA (-1000)

typedef struct Seg {
    Bundle *b;
    int n, cap;
    int lastw[32];
    int maxr[32];
    int lastlsu;
    int lastcry;
    int maxb;
} Seg;

/* Segmento vacio: ninguna escritura ni lectura registrada. */
static void seg_iniciar(Seg *s)
{
    int i;
    memset(s, 0, sizeof *s);
    for (i = 0; i < 32; i++) {
        s->lastw[i] = NADA;
        s->maxr[i] = NADA;
    }
    s->lastlsu = NADA;
    s->lastcry = NADA;
    s->maxb = -1;
}

/* Crece el segmento hasta tener al menos n bundles, los nuevos vacios. */
static void seg_asegurar(Seg *s, int n)
{
    while (s->n < n) {
        if (s->n == s->cap) {
            s->cap = s->cap ? s->cap * 2 : 16;
            s->b = realloc(s->b, (size_t)s->cap * sizeof(Bundle));
        }
        memset(&s->b[s->n], 0, sizeof(Bundle));
        s->n++;
    }
}

/* El mayor de dos indices de bundle. */
static int max2(int a, int b) { return a > b ? a : b; }

/*
 * Primer bundle donde la instruccion es legal. No usa latencias por unidad
 * funcional ni renombra: el pipeline sin forwarding fija la distancia.
 *   RAW  lectura >= ultima escritura + 3
 *   WAW  escritura >= ultima escritura + 1
 *   WAR  escritura >= ultima lectura
 *   LSU y cripto: como mucho una por bundle, en orden de programa (+1)
 */
static int cota(const Seg *s, const Ins *i, int *lee, int nl, int *esc, int ne)
{
    int e = 0, k;
    int slot = op_slot(i->op);

    for (k = 0; k < nl; k++) {
        e = max2(e, s->lastw[lee[k]] + 3);
    }
    for (k = 0; k < ne; k++) {
        e = max2(e, s->lastw[esc[k]] + 1);
        e = max2(e, s->maxr[esc[k]]);
    }
    if (slot == 1) {
        e = max2(e, s->lastlsu + 1);
    } else if (slot == 3) {
        e = max2(e, s->lastcry + 1);
    }
    return e;
}

/*
 * Encaje voraz: el primer bundle >= cota cuyo slot fijo este libre.
 * Cada opcode tiene un solo slot (ALU, LSU, salto o cripto), asi que
 * dos SUM no comparten bundle. Actualiza lastw, maxr, lastlsu y lastcry.
 */
static int colocar(Seg *s, const Ins *i, int minimo)
{
    int lee[8], esc[8], nl, ne, k, b;
    int slot = op_slot(i->op);

    ins_rw(i, lee, &nl, esc, &ne);
    b = max2(cota(s, i, lee, nl, esc, ne), minimo);
    while (1) {
        seg_asegurar(s, b + 1);
        if (!s->b[b].usado[slot]) {
            break;
        }
        b++;
    }
    s->b[b].ins[slot] = *i;
    s->b[b].usado[slot] = 1;
    for (k = 0; k < nl; k++) {
        s->maxr[lee[k]] = max2(s->maxr[lee[k]], b);
    }
    for (k = 0; k < ne; k++) {
        s->lastw[esc[k]] = b;
    }
    if (slot == 1) {
        s->lastlsu = b;
    } else if (slot == 3) {
        s->lastcry = b;
    }
    s->maxb = max2(s->maxb, b);
    return b;
}

typedef enum { FR_CAIDA, FR_COND, FR_INCOND, FR_LLAMADA } Frontera;

typedef struct Plan {
    Programa *p;
    Ins *ops;
    int n_ops, cap_ops;
    char **pend;
    int n_pend;
    long n_ins;
} Plan;

/* Copia un bundle al programa y le pega las etiquetas pendientes. */
static void agregar_bundle(Plan *pl, const Bundle *src)
{
    Bundle *dst = prog_nuevo_bundle(pl->p);
    int k;
    memcpy(dst->ins, src->ins, sizeof dst->ins);
    memcpy(dst->usado, src->usado, sizeof dst->usado);
    for (k = 0; k < pl->n_pend; k++) {
        bundle_etiqueta(dst, pl->pend[k]);
        free(pl->pend[k]);
    }
    pl->n_pend = 0;
}

/*
 * Cierra un segmento. El salto, si lo hay, va al final.
 *   FR_COND     dos bundles vacios: el salto se resuelve en EX y tira IF e ID;
 *               si no se toma, esos dos tambien corren y el destino queda a +3
 *   FR_INCOND   nada detras: S, SRG y FIN siempre descartan los dos siguientes
 *   FR_LLAMADA  nada detras: al volver de SYE el codigo siguiente si se ejecuta
 *   FR_CAIDA    bundles vacios hasta que la ultima escritura sea visible (+3)
 */
static void vaciar(Plan *pl, const Ins *term, Frontera fr)
{
    Seg s;
    int i, k;

    if (pl->n_ops == 0 && !term) {
        return;
    }
    seg_iniciar(&s);
    for (i = 0; i < pl->n_ops; i++) {
        colocar(&s, &pl->ops[i], 0);
        pl->n_ins++;
    }
    if (term) {
        colocar(&s, term, max2(s.maxb, 0));
        pl->n_ins++;
    }
    for (i = 0; i < s.n; i++) {
        agregar_bundle(pl, &s.b[i]);
    }
    if (fr == FR_COND) {
        Bundle vacio;
        memset(&vacio, 0, sizeof vacio);
        agregar_bundle(pl, &vacio);
        agregar_bundle(pl, &vacio);
    } else if (fr == FR_CAIDA) {
        int maxw = NADA;
        int falta;
        Bundle vacio;
        for (k = 0; k < 32; k++) {
            maxw = max2(maxw, s.lastw[k]);
        }
        falta = maxw + 3 - s.n;
        memset(&vacio, 0, sizeof vacio);
        while (falta-- > 0) {
            agregar_bundle(pl, &vacio);
        }
    }
    free(s.b);
    pl->n_ops = 0;
}

/*
 * Recorre el IR en orden de programa. Un segmento se corta en una etiqueta,
 * un salto condicional, S/SRG/FIN o SYE. No es el arbol de prioridades del
 * curso: es list scheduling voraz de primer hueco. Los slots vacios no se
 * rellenan aqui; el emisor imprime NOP solo si el bundle entero esta vacio
 * y el binario escribe el NOP canonico de cada slot.
 */
int planificar(const IR *ir, Programa *p, EstadPlan *est)
{
    Plan pl;
    int i, s, b;
    long slots = 0;

    memset(&pl, 0, sizeof pl);
    pl.p = p;
    prog_iniciar(p);

    for (i = 0; i < ir->n; i++) {
        const Item *it = &ir->v[i];
        if (it->es_etq) {
            vaciar(&pl, NULL, FR_CAIDA);
            pl.pend = realloc(pl.pend, (size_t)(pl.n_pend + 1) * sizeof(char *));
            pl.pend[pl.n_pend++] = strdup(it->etq);
            continue;
        }
        switch (it->ins.op) {
        case OP_SIG: case OP_SNIG: case OP_SMI: case OP_SMQ:
            vaciar(&pl, &it->ins, FR_COND);
            break;
        case OP_S: case OP_SRG: case OP_FIN:
            vaciar(&pl, &it->ins, FR_INCOND);
            break;
        case OP_SYE:
            vaciar(&pl, &it->ins, FR_LLAMADA);
            break;
        default:
            if (pl.n_ops == pl.cap_ops) {
                pl.cap_ops = pl.cap_ops ? pl.cap_ops * 2 : 256;
                pl.ops = realloc(pl.ops, (size_t)pl.cap_ops * sizeof(Ins));
            }
            pl.ops[pl.n_ops++] = it->ins;
            break;
        }
    }
    vaciar(&pl, NULL, FR_INCOND);
    if (pl.n_pend) {                 /* etiquetas al final del codigo */
        Bundle vacio;
        memset(&vacio, 0, sizeof vacio);
        agregar_bundle(&pl, &vacio);
    }
    for (b = 0; b < p->n; b++) {
        for (s = 0; s < 4; s++) {
            slots += p->b[b].usado[s];
        }
    }
    if (est) {
        est->instrucciones = pl.n_ins;
        est->bundles = p->n;
        est->slots_usados = slots;
    }
    free(pl.ops);
    free(pl.pend);
    return 0;
}
