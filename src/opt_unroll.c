#include "opt.h"
#include "semantico.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * --unroll=K | --unroll=auto, sobre el AST ya analizado (los clones comparten
 * los simbolos). Solo lazos `para` canonicos:
 *     para (i = e0; i < N | i <= N; i += c | i = i + c) cuerpo
 * con c constante > 0, N constante o variable escalar que el cuerpo no escribe
 * (si N o i son globales, el cuerpo no puede llamar funciones) y un cuerpo que
 * no escribe i.
 *
 * auto: no desenrolla cuerpos con llamadas de usuario ni cuerpos grandes; K = 4
 * para cuerpos chicos y K = 2 para medianos.
 * Con N conocido y pocas vueltas (auto: <= 8 y cuerpo chico; K: <= K) el lazo se aplana:
 *     init; cuerpo; paso; cuerpo; paso; ...
 * Si no, lazo principal de K copias con guarda i + (K-1)*c < N y un lazo
 * epilogo con la condicion original para las vueltas que sobran:
 *     init;
 *     para (; i + (K-1)*c < N; paso) { cuerpo; paso; ... cuerpo }
 *     para (; i < N; paso) cuerpo
 */

#define K_AUTO 4
#define PLANO_AUTO 8
#define MEDIANO_AUTO 48     /* nodos del AST (~24 instrucciones TAC) */
#define MAX_NODOS_AUTO 120

static EstadOpt *E;
static int K;

// Hace: cuenta nodos de un subarbol; recibe: n; genera: valor o indicador calculado.
static int contar(const Nodo *n)
{
    int c = 0;
    for (; n; n = n->sig) {
        c += 1 + contar(n->hijo);
    }
    return c;
}

/* El subarbol escribe s o llama a una funcion de usuario (*llama). */
// Hace: detecta escrituras sobre un simbolo y llamadas; recibe: n, s, llama; genera: valor o indicador calculado.
static int escribe(const Nodo *n, const Simbolo *s, int *llama)
{
    int w = 0;
    for (; n; n = n->sig) {
        if ((n->tipo == NODO_ASIGNACION && n->hijo && n->hijo->tipo == NODO_IDENT && n->hijo->sim == s) ||
            (n->tipo == NODO_DECLARACION && n->sim == s)) {
            w = 1;
        }
        if (n->tipo == NODO_LLAMADA && n->sim) {
            if (n->sim->clase == CL_INTRINSECA) {
                if (n->sim->intr == IN_RONDA_FEISTEL || n->sim->intr == IN_FEISTEL_CIFRAR ||
                    n->sim->intr == IN_FEISTEL_DESCIFRAR) {
                    const Nodo *a0 = n->hijo, *a1 = a0 ? a0->sig : NULL;
                    if ((a0 && a0->tipo == NODO_IDENT && a0->sim == s) || (a1 && a1->tipo == NODO_IDENT && a1->sim == s)) {
                        w = 1;
                    }
                }
            } else {
                *llama = 1;
            }
        }
        w |= escribe(n->hijo, s, llama);
    }
    return w;
}

// Hace: crea un identificador AST para un simbolo; recibe: s, pos; genera: puntero o NULL segun corresponda.
static Nodo *ident(Simbolo *s, const Nodo *pos)
{
    Nodo *n = nodo_en(NODO_IDENT, s->nombre, pos->linea, pos->col);
    n->sim = s;
    n->ty = s->ty;
    n->archivo = pos->archivo;
    return n;
}

// Hace: crea un nodo literal entero; recibe: v, pos; genera: puntero o NULL segun corresponda.
static Nodo *literal(long v, const Nodo *pos)
{
    char buf[32];
    Nodo *n;
    snprintf(buf, sizeof buf, "%ld", v);
    n = nodo_en(NODO_LITERAL, buf, pos->linea, pos->col);
    n->val = v;
    n->ty = TY_ENT;
    n->archivo = pos->archivo;
    return n;
}

// Hace: construye una operacion binaria AST; recibe: op, l, r, ty, pos; genera: puntero o NULL segun corresponda.
static Nodo *binario(const char *op, Nodo *l, Nodo *r, int ty, const Nodo *pos)
{
    Nodo *n = nodo_binario(op, l, r);
    n->ty = ty;
    n->linea = pos->linea;
    n->col = pos->col;
    n->archivo = pos->archivo;
    return n;
}

// Hace: crea un nodo AST conservando su ubicacion; recibe: t, texto, pos; genera: puntero o NULL segun corresponda.
static Nodo *nodo_de(TipoNodo t, const char *texto, const Nodo *pos)
{
    Nodo *n = nodo_en(t, texto, pos->linea, pos->col);
    n->archivo = pos->archivo;
    return n;
}

// Hace: identifica la variable de control del bucle; recibe: init; genera: puntero o NULL segun corresponda.
static Simbolo *var_de(const Nodo *init)
{
    if (init->tipo == NODO_DECLARACION && init->sim && init->sim->ty == TY_ENT && init->sim->ndim == 0 && init->hijo &&
        init->hijo->sig) {
        return init->sim;
    }
    if (init->tipo == NODO_ASIGNACION && !strcmp(init->texto, "=") && init->hijo->tipo == NODO_IDENT &&
        init->hijo->sim && init->hijo->sim->ty == TY_ENT) {
        return init->hijo->sim;
    }
    return NULL;
}

// Hace: obtiene el valor inicial del contador; recibe: init; genera: puntero o NULL segun corresponda.
static const Nodo *valor_inicial(const Nodo *init)
{
    return init->hijo->sig;
}

// Hace: reconoce el incremento de un contador; recibe: act, s, c; genera: valor o indicador calculado.
static int paso_de(const Nodo *act, const Simbolo *s, long *c)
{
    const Nodo *dst, *val;
    if (act->tipo != NODO_ASIGNACION) {
        return 0;
    }
    dst = act->hijo;
    val = dst ? dst->sig : NULL;
    if (!dst || dst->tipo != NODO_IDENT || dst->sim != s || !val) {
        return 0;
    }
    if (!strcmp(act->texto, "+=")) {
        return sem_const(val, c) && *c > 0;
    }
    if (!strcmp(act->texto, "=") && val->tipo == NODO_EXP && !strcmp(val->texto, "+") && val->hijo->tipo == NODO_IDENT &&
        val->hijo->sim == s) {
        return sem_const(val->hijo->sig, c) && *c > 0;
    }
    return 0;
}

/* Agrega `n` al final de los hijos de `bloque`. */
// Hace: agrega una sentencia a un bloque; recibe: bloque, n; genera: efectos sobre estructuras o salida.
static void colgar(Nodo *bloque, Nodo *n)
{
    nodo_hijo(bloque, n);
}

// Hace: transforma un bucle canonico en copias; recibe: para; genera: efectos sobre estructuras o salida.
static void desenrollar(Nodo *para)
{
    Nodo *init = para->hijo;
    Nodo *cond = init ? init->sig : NULL;
    Nodo *act = cond ? cond->sig : NULL;
    Nodo *cuerpo = act ? act->sig : NULL;
    Simbolo *s;
    Nodo *lim;
    long c, n0 = 0, nf = 0;
    int menor_igual, llama = 0, conocido, k_usar, i;
    long vueltas = -1;

    if (!cuerpo || !(s = var_de(init)) || !paso_de(act, s, &c)) {
        return;
    }
    if (cond->tipo != NODO_EXP || (strcmp(cond->texto, "<") && strcmp(cond->texto, "<=")) ||
        cond->hijo->tipo != NODO_IDENT || cond->hijo->sim != s) {
        return;
    }
    menor_igual = !strcmp(cond->texto, "<=");
    lim = cond->hijo->sig;
    if (escribe(cuerpo, s, &llama)) {
        return;
    }
    if (!sem_const(lim, &nf)) {
        if (lim->tipo != NODO_IDENT || !lim->sim || lim->sim->ty != TY_ENT || lim->sim->ndim != 0) {
            return;
        }
        if (escribe(cuerpo, lim->sim, &llama) || lim->sim == s) {
            return;
        }
        if (lim->sim->alm == AL_GLOBAL && llama) {
            return;
        }
    }
    if (s->alm == AL_GLOBAL && llama) {
        return;
    }
    conocido = sem_const(lim, &nf) && sem_const(valor_inicial(init), &n0);
    if (conocido) {
        long hasta = menor_igual ? nf + 1 : nf;
        vueltas = hasta > n0 ? (hasta - n0 + c - 1) / c : 0;
    }
    if (K < 0) {
        /* auto: sin llamadas de usuario; K=4 cuerpo chico, K=2 mediano, nada si es grande */
        int nodos = contar(cuerpo->hijo);
        int llama_cuerpo = 0;
        escribe(cuerpo, NULL, &llama_cuerpo);
        if (llama_cuerpo || nodos > MAX_NODOS_AUTO) {
            return;
        }
        k_usar = nodos <= MEDIANO_AUTO ? K_AUTO : 2;
        if (conocido && vueltas <= PLANO_AUTO && nodos <= MEDIANO_AUTO) {
            k_usar = (int)vueltas;
        }
    } else {
        k_usar = K;
        if (conocido && vueltas <= K) {
            k_usar = (int)vueltas;
        }
    }

    if (conocido && vueltas == k_usar) {
        /* aplanado: init; (cuerpo; paso) * vueltas */
        Nodo *orig_init = init;
        para->tipo = NODO_BLOQUE;
        para->hijo = NULL;
        orig_init->sig = NULL;
        colgar(para, orig_init);
        for (i = 0; i < vueltas; i++) {
            colgar(para, nodo_clonar(cuerpo, 0));
            colgar(para, nodo_clonar(act, 0));
        }
        E->lazos_desenrollados++;
        E->iteraciones_unroll += vueltas > 0 ? vueltas - 1 : 0;
        return;
    }
    if (k_usar < 2) {
        return;
    }
    {
        Nodo *orig_init = init;
        Nodo *principal = nodo_de(NODO_PARA, NULL, para);
        Nodo *epilogo = nodo_de(NODO_PARA, NULL, para);
        Nodo *bloque = nodo_de(NODO_BLOQUE, NULL, para);
        Nodo *guarda = binario(cond->texto, binario("+", ident(s, cond), literal((long)(k_usar - 1) * c, cond), TY_ENT, cond),
                               nodo_clonar(lim, 0), TY_BOOL, cond);

        for (i = 0; i < k_usar; i++) {
            colgar(bloque, nodo_clonar(cuerpo, 0));
            if (i < k_usar - 1) {
                colgar(bloque, nodo_clonar(act, 0));
            }
        }
        colgar(principal, nodo_de(NODO_LISTA, "inicio", para));
        colgar(principal, guarda);
        colgar(principal, nodo_clonar(act, 0));
        colgar(principal, bloque);

        /* el epilogo reutiliza los nodos originales */
        cond->sig = NULL;
        act->sig = NULL;
        cuerpo->sig = NULL;
        colgar(epilogo, nodo_de(NODO_LISTA, "inicio", para));
        colgar(epilogo, cond);
        colgar(epilogo, act);
        colgar(epilogo, cuerpo);

        para->tipo = NODO_BLOQUE;
        para->hijo = NULL;
        orig_init->sig = NULL;
        colgar(para, orig_init);
        colgar(para, principal);
        if (!(conocido && vueltas % k_usar == 0)) {
            colgar(para, epilogo);
        }
        E->lazos_desenrollados++;
        E->iteraciones_unroll += k_usar - 1;
    }
}

// Hace: busca y optimiza bucles en el AST; recibe: n; genera: efectos sobre estructuras o salida.
static void recorrer(Nodo *n)
{
    for (; n; n = n->sig) {
        recorrer(n->hijo);
        if (n->tipo == NODO_PARA) {
            desenrollar(n);
        }
    }
}

// Hace: aplica desenrollado a los bucles elegibles; recibe: raiz, k, e; genera: efectos sobre estructuras o salida.
void opt_unroll(Nodo *raiz, int k, EstadOpt *e)
{
    E = e;
    K = k;
    recorrer(raiz);
}