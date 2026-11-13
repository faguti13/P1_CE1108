#include "ir_tac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Este codigo sirve para analizar y optimizar antes de elegir registros. */
typedef struct {
    CodigoTac *c;
    int temporal, etiqueta;
} Generador;

static char *copia(const char *s) { return strdup(s ? s : ""); }
static char *numero(long n) {
    char s[40]; snprintf(s, sizeof s, "%ld", n); return copia(s);
}
static char *nuevo(Generador *g, char letra) {
    char s[40];
    int n = letra == 't' ? g->temporal++ : g->etiqueta++;
    snprintf(s, sizeof s, "%c%d", letra, n);
    return copia(s);
}
static void poner(Generador *g, TipoTac tipo, const char *res,
                  const char *a, const char *b, const char *op, int linea) {
    CodigoTac *c = g->c;
    if (c->n == c->cap) {
        c->cap = c->cap ? c->cap * 2 : 64;
        c->v = realloc(c->v, (size_t)c->cap * sizeof *c->v);
        if (!c->v) { perror("realloc"); exit(1); }
    }
    c->v[c->n++] = (Tac){ tipo, copia(res), copia(a), copia(b), copia(op), linea };
}
static const char *nombre(const Nodo *n) {
    return n && n->texto ? n->texto : "?";
}
static char *expresion(Generador *g, const Nodo *n);

static char *indice(Generador *g, const Nodo *n) {
    const Nodo *h = n ? n->hijo : NULL;
    char *base = expresion(g, h);
    char *idx = expresion(g, h ? h->sig : NULL);
    char *r = malloc(strlen(base) + strlen(idx) + 3);
    sprintf(r, "%s[%s]", base, idx);
    free(base); free(idx);
    return r;
}
static char *expresion(Generador *g, const Nodo *n) {
    char *a, *b, *t;
    if (!n) return copia("0");
    switch (n->tipo) {
    case NODO_LITERAL: return numero(n->val);
    case NODO_IDENT: return copia(nombre(n));
    case NODO_INDICE:
        a = indice(g, n); t = nuevo(g, 't');
        poner(g, TAC_LOAD, t, a, NULL, NULL, n->linea);
        free(a); return t;
    case NODO_LLAMADA: {
        const Nodo *args = n->hijo;
        int cuenta = 0;
        if (args && args->tipo == NODO_LISTA) args = args->hijo;
        for (; args; args = args->sig) {
            a = expresion(g, args);
            poner(g, TAC_PARAM, NULL, a, NULL, NULL, n->linea);
            free(a); cuenta++;
        }
        t = nuevo(g, 't');
        a = numero(cuenta);
        poner(g, TAC_CALL, t, nombre(n), a, NULL, n->linea);
        free(a); return t;
    }
    case NODO_EXP:
        a = expresion(g, n->hijo);
        t = nuevo(g, 't');
        if (!n->hijo || !n->hijo->sig) {
            poner(g, TAC_UN, t, a, NULL,
                  !strcmp(nombre(n), "neg") ? "-" : nombre(n), n->linea);
        } else {
            b = expresion(g, n->hijo->sig);
            poner(g, TAC_BIN, t, a, b, nombre(n), n->linea);
            free(b);
        }
        free(a); return t;
    default: return copia("0");
    }
}
static void sentencia(Generador *g, const Nodo *n);
static void bloque(Generador *g, const Nodo *n) {
    const Nodo *h;
    if (!n) return;
    if (n->tipo != NODO_BLOQUE && n->tipo != NODO_LISTA) {
        sentencia(g, n); return;
    }
    for (h = n->hijo; h; h = h->sig) sentencia(g, h);
}
static void condicion(Generador *g, const Nodo *n, const char *falso) {
    char *v = expresion(g, n);
    poner(g, TAC_JZ, falso, v, NULL, NULL, n ? n->linea : 0);
    free(v);
}
static void sentencia(Generador *g, const Nodo *n) {
    const Nodo *h;
    char *a, *b, *t, *l1, *l2;
    if (!n) return;
    switch (n->tipo) {
    case NODO_BLOQUE: bloque(g, n); break;
    case NODO_DECLARACION:
        h = n->hijo;
        if (h && h->sig) {
            a = expresion(g, h->sig);
            poner(g, TAC_COPY, nombre(n), a, NULL, NULL, n->linea);
            free(a);
        }
        break;
    case NODO_ASIGNACION:
        h = n->hijo;
        if (!h || !h->sig) break;
        a = expresion(g, h->sig);
        if (strcmp(nombre(n), "=")) {
            b = h->tipo == NODO_INDICE ? indice(g, h) : copia(nombre(h));
            t = nuevo(g, 't');
            poner(g, TAC_BIN, t, b, a,
                  nombre(n)[0] == '+' ? "+" : "-", n->linea);
            free(a); a = t; free(b);
        }
        if (h->tipo == NODO_INDICE) {
            b = indice(g, h);
            poner(g, TAC_STORE, b, a, NULL, NULL, n->linea);
            free(b);
        } else poner(g, TAC_COPY, nombre(h), a, NULL, NULL, n->linea);
        free(a); break;
    case NODO_SI:
        h = n->hijo;
        if (!h) break;
        l1 = nuevo(g, 'L'); l2 = nuevo(g, 'L');
        condicion(g, h, l1);
        bloque(g, h->sig);
        if (h->sig && h->sig->sig) {
            poner(g, TAC_JMP, l2, NULL, NULL, NULL, n->linea);
            poner(g, TAC_LABEL, l1, NULL, NULL, NULL, n->linea);
            bloque(g, h->sig->sig);
            poner(g, TAC_LABEL, l2, NULL, NULL, NULL, n->linea);
        } else poner(g, TAC_LABEL, l1, NULL, NULL, NULL, n->linea);
        free(l1); free(l2); break;
    case NODO_MIENTRAS:
        h = n->hijo;
        if (!h) break;
        l1 = nuevo(g, 'L'); l2 = nuevo(g, 'L');
        poner(g, TAC_LABEL, l1, NULL, NULL, NULL, n->linea);
        condicion(g, h, l2);
        bloque(g, h->sig);
        poner(g, TAC_JMP, l1, NULL, NULL, NULL, n->linea);
        poner(g, TAC_LABEL, l2, NULL, NULL, NULL, n->linea);
        free(l1); free(l2); break;
    case NODO_PARA:
        h = n->hijo;
        if (!h || !h->sig || !h->sig->sig) break;
        sentencia(g, h);
        l1 = nuevo(g, 'L'); l2 = nuevo(g, 'L');
        poner(g, TAC_LABEL, l1, NULL, NULL, NULL, n->linea);
        condicion(g, h->sig, l2);
        bloque(g, h->sig->sig->sig);
        sentencia(g, h->sig->sig);
        poner(g, TAC_JMP, l1, NULL, NULL, NULL, n->linea);
        poner(g, TAC_LABEL, l2, NULL, NULL, NULL, n->linea);
        free(l1); free(l2); break;
    case NODO_RETORNO:
        a = n->hijo ? expresion(g, n->hijo) : copia("");
        poner(g, TAC_RET, NULL, a, NULL, NULL, n->linea);
        free(a); break;
    case NODO_TERMINAR: poner(g, TAC_HALT, NULL, NULL, NULL, NULL, n->linea); break;
    case NODO_LLAMADA:
        a = expresion(g, n); free(a); break;
    case NODO_DECL_LISTA:
        /* Los arreglos se declaran en memoria; sus accesos usan LOAD/STORE. */
        break;
    default: break;
    }
}
void tac_generar(const Nodo *raiz, CodigoTac *codigo) {
    const Nodo *cuerpo, *h;
    Generador g = { codigo, 0, 0 };
    memset(codigo, 0, sizeof *codigo);
    if (!raiz || !raiz->hijo) return;
    cuerpo = raiz->hijo->sig;
    if (!cuerpo) return;
    poner(&g, TAC_LABEL, "_inicio", NULL, NULL, NULL, 0);
    for (h = cuerpo->hijo; h; h = h->sig)
        if (h->tipo != NODO_FUNCION) sentencia(&g, h);
    /* Igual que codegen: main se invoca desde el inicio. */
    for (h = cuerpo->hijo; h; h = h->sig)
        if (h->tipo == NODO_FUNCION && !strcmp(nombre(h), "main")) {
            poner(&g, TAC_CALL, NULL, "f_main", "0", NULL, h->linea);
            break;
        }
    poner(&g, TAC_HALT, NULL, NULL, NULL, NULL, 0);
    for (h = cuerpo->hijo; h; h = h->sig) {
        const Nodo *body;
        char *f;
        if (h->tipo != NODO_FUNCION) continue;
        f = malloc(strlen(nombre(h)) + 3);
        sprintf(f, "f_%s", nombre(h));
        poner(&g, TAC_LABEL, f, NULL, NULL, NULL, h->linea);
        body = h->hijo && h->hijo->sig ? h->hijo->sig->sig : NULL;
        bloque(&g, body);
        /* Retorno por defecto para funciones sin retorno explicito. */
        if (!codigo->n || codigo->v[codigo->n-1].tipo != TAC_RET)
            poner(&g, TAC_RET, NULL, "", NULL, NULL, h->linea);
        free(f);
    }
}
void tac_imprimir(const CodigoTac *c, FILE *out) {
    int i;
    for (i = 0; i < c->n; i++) {
        const Tac *t = &c->v[i];
        fprintf(out, "%3d  ", i);
        switch (t->tipo) {
        case TAC_LABEL: fprintf(out, "%s:", t->res); break;
        case TAC_COPY: fprintf(out, "%s = %s", t->res, t->arg1); break;
        case TAC_BIN: fprintf(out, "%s = %s %s %s", t->res, t->arg1, t->op, t->arg2); break;
        case TAC_UN: fprintf(out, "%s = %s%s", t->res, t->op, t->arg1); break;
        case TAC_LOAD: fprintf(out, "%s = %s", t->res, t->arg1); break;
        case TAC_STORE: fprintf(out, "%s = %s", t->res, t->arg1); break;
        case TAC_JMP: fprintf(out, "goto %s", t->res); break;
        case TAC_JZ: fprintf(out, "ifFalse %s goto %s", t->arg1, t->res); break;
        case TAC_PARAM: fprintf(out, "param %s", t->arg1); break;
        case TAC_CALL: if (*t->res) fprintf(out, "%s = ", t->res);
                       fprintf(out, "call %s, %s", t->arg1, t->arg2); break;
        case TAC_RET: fprintf(out, "return %s", t->arg1); break;
        case TAC_HALT: fprintf(out, "terminar"); break;
        }
        fputc('\n', out);
    }
}
void tac_liberar(CodigoTac *c) {
    int i;
    for (i = 0; i < c->n; i++) {
        free(c->v[i].res); free(c->v[i].arg1); free(c->v[i].arg2); free(c->v[i].op);
    }
    free(c->v); memset(c, 0, sizeof *c);
}
