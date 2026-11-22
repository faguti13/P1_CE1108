#include "cfg.h"
#include "opt.h"

#include <stdlib.h>
#include <string.h>

/*
 * Eliminacion de codigo muerto (--dce), hasta punto fijo:
 *   1. saltos con condicion constante: goto o nada
 *   2. bloques inalcanzables desde la entrada
 *   3. goto a la etiqueta que sigue
 *   4. definiciones cuyo valor no esta vivo despues (vivacidad del CFG); no se
 *      tocan stores, llamadas, retornos, terminar, param ni las intrinsecas de
 *      boveda/Feistel. Una llamada cuyo valor no se usa pierde solo el destino.
 *   5. etiquetas que ningun salto nombra (cortan bloques y obligan al planificador
 *      a drenar el pipeline)
 */

static long contar_borradas;

// Hace: retira una instruccion TAC; recibe: fn, k; genera: efectos sobre estructuras o salida.
static void borrar(TacFunc *fn, int k)
{
    if (fn->v[k].op != TAC_LABEL) {
        contar_borradas++;
    }
    tac_borrar(fn, k);
}

// Hace: simplifica cadenas de saltos; recibe: fn; genera: valor o indicador calculado.
static int plegar_saltos(TacFunc *fn)
{
    int k, cambio = 0;
    for (k = 0; k < fn->n; k++) {
        Tac *t = &fn->v[k];
        int salta = -1;
        if (t->op == TAC_BCOND && t->a.k == OP_C && t->b.k == OP_C) {
            long a = t->a.v, b = t->b.v;
            switch (t->bop) {
            case B_LT: salta = a < b; break;
            case B_LE: salta = a <= b; break;
            case B_GT: salta = a > b; break;
            case B_GE: salta = a >= b; break;
            case B_EQ: salta = a == b; break;
            case B_NE: salta = a != b; break;
            default: break;
            }
        } else if ((t->op == TAC_JZ || t->op == TAC_JNZ) && t->a.k == OP_C) {
            salta = t->op == TAC_JZ ? t->a.v == 0 : t->a.v != 0;
        }
        if (salta == 1) {
            t->op = TAC_JMP;
            t->a.k = t->b.k = OP_NADA;
            cambio = 1;
        } else if (salta == 0) {
            borrar(fn, k--);
            cambio = 1;
        }
    }
    return cambio;
}

// Hace: elimina instrucciones o bloques inalcanzables; recibe: fn; genera: valor o indicador calculado.
static int inalcanzables(TacFunc *fn)
{
    Cfg g;
    int i, k, cambio = 0;
    cfg_construir(&g, fn);
    for (i = g.n - 1; i >= 0; i--) {
        if (!g.b[i].alcanzable) {
            for (k = g.b[i].fin - 1; k >= g.b[i].ini; k--) {
                borrar(fn, k);
            }
            cambio = 1;
        }
    }
    cfg_liberar(&g);
    return cambio;
}

// Hace: elimina saltos al bloque contiguo; recibe: fn; genera: valor o indicador calculado.
static int saltos_al_siguiente(TacFunc *fn)
{
    int k, j, cambio = 0;
    for (k = 0; k < fn->n; k++) {
        Tac *t = &fn->v[k];
        if (!tac_es_salto(t)) {
            continue;
        }
        for (j = k + 1; j < fn->n && fn->v[j].op == TAC_LABEL; j++) {
            if (!strcmp(fn->v[j].etq, t->etq)) {
                borrar(fn, k--);
                cambio = 1;
                break;
            }
        }
    }
    return cambio;
}

// Hace: retira etiquetas que no son necesarias; recibe: fn; genera: valor o indicador calculado.
static int etiquetas_sueltas(TacFunc *fn)
{
    int k, j, cambio = 0;
    for (k = 0; k < fn->n; k++) {
        int usada = 0;
        if (fn->v[k].op != TAC_LABEL) {
            continue;
        }
        for (j = 0; j < fn->n && !usada; j++) {
            usada = tac_es_salto(&fn->v[j]) && !strcmp(fn->v[j].etq, fn->v[k].etq);
        }
        if (!usada) {
            borrar(fn, k--);
            cambio = 1;
        }
    }
    return cambio;
}

// Hace: elimina definiciones sin uso ni efectos; recibe: fn; genera: valor o indicador calculado.
static int definiciones_muertas(TacFunc *fn)
{
    Cfg g;
    int i, k, j, cambio = 0;
    uint64_t *vivo;
    char *muere = calloc((size_t)fn->n + 1, 1);

    cfg_construir(&g, fn);
    cfg_vivacidad(&g);
    vivo = calloc((size_t)g.palabras + 1, sizeof(uint64_t));
    for (i = 0; i < g.n; i++) {
        Bloque *b = &g.b[i];
        memcpy(vivo, b->out, (size_t)g.palabras * sizeof(uint64_t));
        for (k = b->fin - 1; k >= b->ini; k--) {
            Tac *t = &fn->v[k];
            int lee[4], esc[4], nl, ne, alguno = 0;
            ne = tac_escribe(t, esc);
            for (j = 0; j < ne; j++) {
                alguno |= bs_tiene(vivo, esc[j]);
            }
            if (t->op == TAC_COPY && t->a.k == OP_V && t->a.v == t->d) {
                muere[k] = 1;
                continue;
            }
            if (ne > 0 && !alguno && !tac_efecto(t)) {
                muere[k] = 1;
                continue;
            }
            if (t->op == TAC_CALL && t->d >= 0 && !bs_tiene(vivo, t->d)) {
                t->d = -1;
                cambio = 1;
            }
            for (j = 0; j < ne; j++) {
                bs_quita(vivo, esc[j]);
            }
            nl = tac_lee(t, lee);
            for (j = 0; j < nl; j++) {
                bs_pon(vivo, lee[j]);
            }
        }
    }
    for (k = fn->n - 1; k >= 0; k--) {
        if (muere[k]) {
            borrar(fn, k);
            cambio = 1;
        }
    }
    free(vivo);
    free(muere);
    cfg_liberar(&g);
    return cambio;
}

// Hace: ejecuta las pasadas de eliminacion de codigo muerto; recibe: fn, e; genera: efectos sobre estructuras o salida.
void opt_dce(TacFunc *fn, EstadOpt *e)
{
    int cambio, vueltas = 0;
    contar_borradas = 0;
    do {
        cambio = plegar_saltos(fn);
        cambio |= inalcanzables(fn);
        cambio |= saltos_al_siguiente(fn);
        cambio |= definiciones_muertas(fn);
        cambio |= etiquetas_sueltas(fn);
    } while (cambio && ++vueltas < 32);
    if (e) {
        e->eliminadas_dce += contar_borradas;
    }
}
