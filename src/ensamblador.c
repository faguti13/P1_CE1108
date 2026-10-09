#include "ensamblador.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Ensamblador propio (jaf -x). Es independiente de vliw_grupo2/tools/
 * vliwasm.py: ambos implementan la misma sintaxis (adenda v1.1 de isa.md)
 * y se validan byte a byte entre si (make xcheck).
 *
 * Una línea es un bundle; los slots se separan con '|'. Cada instrucción va sola en el slot que le toca por su tipo; los demás son NOP.
 * Directivas: .text .data .equ .word .space .align. Comentario: ';'.
 */

/* Símbolo: constante .equ o etiqueta (dirección de un bundle en .text o de un dato en .data). */
typedef struct Simb {
    char *nombre;
    long valor;
    int dato;                   /* 1 si apunta a la sección .data */
} Simb;

/* Instrucción tal como se leyó: mnemónico y operandos en texto.
   Se convierte a Ins en la segunda pasada. */
typedef struct Crudo {          /* instrucción sin resolver */
    char *mn;
    char *ops[8];
    int n_ops;
} Crudo;

/* Un bundle leído: número de línea del fuente y lo que hay en cada slot. */
typedef struct Linea {
    int num;
    Crudo slot[4];
    int usado[4];
} Linea;

/* Estado del ensamblador entre las dos pasadas:
 -   constantes .equ y etiquetas
 -   bundles leídos
 -   etiquetas pendientes (vistas pero aún sin instrucción o dato)
 -   sección .data
 -   buffer del mensaje de error */
typedef struct Estado {
    Simb *equ;
    int n_equ;
    Simb *etq;
    int n_etq;
    Linea *lineas;
    int n_lin;
    int cap_lin;
    char **pend;
    int n_pend;
    uint8_t *datos;
    long n_datos, cap_datos;
    char *err;
    size_t n_err;
} Estado;

/* Escribe "linea N: mensaje" en el buffer de error y devuelve 1. */
#define FALLO(st, ln, ...) do { \
        int _k = snprintf((st)->err, (st)->n_err, "linea %d: ", (ln)); \
        if (_k > 0 && (size_t)_k < (st)->n_err) snprintf((st)->err + _k, (st)->n_err - (size_t)_k, __VA_ARGS__); \
        return 1; } while (0)

/* Quita los espacios del inicio y del final de s y devuelve el nuevo inicio. */
static char *recortar(char *s)
{
    char *fin;
    while (isspace((unsigned char)*s)) {
        s++;
    }
    fin = s + strlen(s);
    while (fin > s && isspace((unsigned char)fin[-1])) {
        *--fin = '\0';
    }
    return s;
}

/* Busca un símbolo por nombre; devuelve NULL si no está. */
static Simb *buscar(Simb *v, int n, const char *nombre)
{
    int i;
    for (i = 0; i < n; i++) {
        if (strcmp(v[i].nombre, nombre) == 0) {
            return &v[i];
        }
    }
    return NULL;
}

/* Convierte un operando a número:
 -   primero lo busca como constante .equ o como etiqueta
 -   si no, lo lee como entero decimal, hexadecimal (0x) u octal
   Devuelve 1 y deja el error si no es un valor válido. */
static int num(Estado *st, int ln, const char *tok, long *out)
{
    char *t = strdup(tok);
    char *r = recortar(t);
    Simb *s;
    char *fin;
    long v;

    if ((s = buscar(st->equ, st->n_equ, r)) || (s = buscar(st->etq, st->n_etq, r))) {
        *out = s->valor;
        free(t);
        return 0;
    }
    v = strtol(r, &fin, 0);
    if (*r == '\0' || *fin != '\0') {
        snprintf(st->err, st->n_err, "linea %d: valor invalido '%s'", ln, tok);
        free(t);
        return 1;
    }
    *out = v;
    free(t);
    return 0;
}

/* Convierte un operando a número de registro: r0..r31 o los alias zero, gp,
   sp, fp y ra. Devuelve 1 si no es un registro válido. */
static int reg(Estado *st, int ln, const char *tok, int *out)
{
    char *t = strdup(tok);
    char *r = recortar(t);
    char *fin;
    long v;
    int i;

    for (i = 0; r[i]; i++) {
        r[i] = (char)tolower((unsigned char)r[i]);
    }
    *out = -1;
    if (!strcmp(r, "zero")) *out = 0;
    else if (!strcmp(r, "gp")) *out = R_GP;
    else if (!strcmp(r, "sp")) *out = R_SP;
    else if (!strcmp(r, "fp")) *out = R_FP;
    else if (!strcmp(r, "ra")) *out = R_RA;
    else if (r[0] == 'r' && isdigit((unsigned char)r[1])) {
        v = strtol(r + 1, &fin, 10);
        if (*fin == '\0' && v >= 0 && v <= 31) {
            *out = (int)v;
        }
    }
    free(t);
    if (*out < 0) {
        snprintf(st->err, st->n_err, "linea %d: registro invalido '%s'", ln, tok);
        return 1;
    }
    return 0;
}

/* Agrega un byte a la sección .data; el buffer crece al doble cuando se llena. */
static void agregar_dato(Estado *st, uint8_t b)
{
    if (st->n_datos == st->cap_datos) {
        st->cap_datos = st->cap_datos ? st->cap_datos * 2 : 256;
        st->datos = realloc(st->datos, (size_t)st->cap_datos);
    }
    st->datos[st->n_datos++] = b;
}

/* Agrega un símbolo al final de la tabla v. No revisa repetidos:
   las etiquetas de código repetidas las detecta prog_resolver. */
static void agregar_simb(Simb **v, int *n, const char *nombre, long valor)
{
    *v = realloc(*v, (size_t)(*n + 1) * sizeof(Simb));
    (*v)[*n].nombre = strdup(nombre);
    (*v)[*n].valor = valor;
    (*v)[*n].dato = 0;
    (*n)++;
}

/* Separa los operandos por comas y los copia sin espacios.
   Devuelve cuántos hay, o -1 si son más de max. */
static int dividir_ops(char *s, char **ops, int max)
{
    int n = 0;
    char *tok;
    s = recortar(s);
    if (*s == '\0') {
        return 0;
    }
    while (1) {
        tok = strchr(s, ',');
        if (tok) {
            *tok = '\0';
        }
        if (n < max) {
            ops[n++] = strdup(recortar(s));
        } else {
            return -1;
        }
        if (!tok) {
            break;
        }
        s = tok + 1;
    }
    return n;
}

/* Indica si s sirve como nombre de etiqueta: empieza con letra o '_' y
   sigue con letras, dígitos o '_'. */
static int es_ident(const char *s)
{
    if (!(isalpha((unsigned char)*s) || *s == '_')) {
        return 0;
    }
    for (; *s; s++) {
        if (!(isalnum((unsigned char)*s) || *s == '_')) {
            return 0;
        }
    }
    return 1;
}

/* Primera pasada, recorre el texto línea por línea:
 -   quita los comentarios (';') y lee las etiquetas del inicio ("nombre:")
 -   directivas: .text y .data cambian de sección, .equ define una constante,
     .word, .space y .align llenan la sección de datos
 -   cada línea de .text es un bundle: se separa por '|' y cada instrucción
     va al slot de su tipo, todavía sin resolver
   Las etiquetas de código valen indice_del_bundle * 16 y las de datos su
   dirección desde 0x10000. */
static int pasada1(Estado *st, const char *texto, Programa *p)
{
    char *copia = strdup(texto);
    char *linea_ptr = copia;
    int num_lin = 0;
    int en_data = 0;

    while (linea_ptr && *linea_ptr) {
        char *sig = strchr(linea_ptr, '\n');
        char *lin;
        char *pc;
        if (sig) {
            *sig = '\0';
        }
        num_lin++;
        pc = strchr(linea_ptr, ';');
        if (pc) {
            *pc = '\0';
        }
        lin = recortar(linea_ptr);
        linea_ptr = sig ? sig + 1 : NULL;
        if (*lin == '\0') {
            continue;
        }
        /* etiquetas al inicio */
        while (1) {
            char *dp = strchr(lin, ':');
            char nombre[128];
            size_t k;
            if (!dp) {
                break;
            }
            k = (size_t)(dp - lin);
            if (k == 0 || k >= sizeof nombre) {
                break;
            }
            memcpy(nombre, lin, k);
            nombre[k] = '\0';
            {
                char *r = recortar(nombre);
                if (!es_ident(r)) {
                    break;
                }
                st->pend = realloc(st->pend, (size_t)(st->n_pend + 1) * sizeof(char *));
                st->pend[st->n_pend++] = strdup(r);
            }
            lin = recortar(dp + 1);
        }
        if (*lin == '\0') {
            continue;
        }
        if (lin[0] == '.') {
            char *arg = lin;
            char *ops[64];
            int n, i;
            while (*arg && !isspace((unsigned char)*arg)) {
                arg++;
            }
            if (*arg) {
                *arg++ = '\0';
            }
            for (i = 0; lin[i]; i++) {
                lin[i] = (char)tolower((unsigned char)lin[i]);
            }
            if (!strcmp(lin, ".text")) {
                en_data = 0;
            } else if (!strcmp(lin, ".data")) {
                en_data = 1;
            } else if (!strcmp(lin, ".equ")) {
                long v;
                n = dividir_ops(arg, ops, 64);
                if (n != 2) {
                    FALLO(st, num_lin, ".equ nombre, valor");
                }
                if (num(st, num_lin, ops[1], &v)) {
                    return 1;
                }
                agregar_simb(&st->equ, &st->n_equ, ops[0], v);
            } else if (!strcmp(lin, ".word") || !strcmp(lin, ".space")) {
                int es_word = !strcmp(lin, ".word");
                long v;
                int j;
                if (!en_data) {
                    FALLO(st, num_lin, "directiva no valida en la seccion text: %s", lin);
                }
                for (j = 0; j < st->n_pend; j++) {
                    agregar_simb(&st->etq, &st->n_etq, st->pend[j], MEM_DATA_BASE + st->n_datos);
                    st->etq[st->n_etq - 1].dato = 1;
                }
                st->n_pend = 0;
                if (es_word) {
                    n = dividir_ops(arg, ops, 64);
                    for (j = 0; j < n; j++) {
                        if (num(st, num_lin, ops[j], &v)) {
                            return 1;
                        }
                        agregar_dato(st, (uint8_t)(v >> 24));
                        agregar_dato(st, (uint8_t)(v >> 16));
                        agregar_dato(st, (uint8_t)(v >> 8));
                        agregar_dato(st, (uint8_t)v);
                    }
                } else {
                    if (num(st, num_lin, arg, &v)) {
                        return 1;
                    }
                    for (j = 0; j < v; j++) {
                        agregar_dato(st, 0);
                    }
                }
            } else if (!strcmp(lin, ".align")) {
                long v;
                if (!en_data) {
                    FALLO(st, num_lin, ".align solo aplica en .data");
                }
                if (num(st, num_lin, arg, &v)) {
                    return 1;
                }
                while (v > 0 && st->n_datos % v) {
                    agregar_dato(st, 0);
                }
            } else {
                FALLO(st, num_lin, "directiva no valida: %s", lin);
            }
            continue;
        }
        if (en_data) {
            FALLO(st, num_lin, "instruccion fuera de .text");
        }
        /* un bundle */
        {
            Linea *ln;
            char *parte = lin;
            int j;
            if (st->n_lin == st->cap_lin) {
                st->cap_lin = st->cap_lin ? st->cap_lin * 2 : 256;
                st->lineas = realloc(st->lineas, (size_t)st->cap_lin * sizeof(Linea));
            }
            ln = &st->lineas[st->n_lin];
            memset(ln, 0, sizeof *ln);
            ln->num = num_lin;
            for (j = 0; j < st->n_pend; j++) {
                agregar_simb(&st->etq, &st->n_etq, st->pend[j], (long)st->n_lin * 16);
            }
            st->n_pend = 0;
            while (parte) {
                char *barra = strchr(parte, '|');
                char *tok, *resto;
                Crudo c;
                int s;
                Op op;
                if (barra) {
                    *barra = '\0';
                }
                tok = recortar(parte);
                parte = barra ? barra + 1 : NULL;
                if (*tok == '\0') {
                    continue;
                }
                memset(&c, 0, sizeof c);
                resto = tok;
                while (*resto && !isspace((unsigned char)*resto)) {
                    resto++;
                }
                if (*resto) {
                    *resto++ = '\0';
                }
                c.mn = strdup(tok);
                {
                    int i;
                    for (i = 0; c.mn[i]; i++) {
                        c.mn[i] = (char)toupper((unsigned char)c.mn[i]);
                    }
                }
                c.n_ops = dividir_ops(resto, c.ops, 8);
                if (c.n_ops < 0) {
                    FALLO(st, num_lin, "demasiados operandos");
                }
                if (!strcmp(c.mn, "NOP")) {
                    continue;
                }
                op = op_buscar(c.mn);
                if (op == OP_CANTIDAD) {
                    FALLO(st, num_lin, "instruccion desconocida '%s'", tok);
                }
                s = op_slot(op);
                if (ln->usado[s]) {
                    FALLO(st, num_lin, "el slot %d ya esta ocupado en este bundle (opcode fuera de su slot o 2 instrucciones del mismo tipo)", s);
                }
                ln->usado[s] = 1;
                ln->slot[s] = c;
            }
            st->n_lin++;
        }
    }
    if (st->n_pend) {
        snprintf(st->err, st->n_err, "etiqueta '%s' al final del codigo, sin instruccion", st->pend[0]);
        free(copia);
        return 1;
    }
    free(copia);
    (void)p;
    return 0;
}

/* Revisa que la instrucción tenga exactamente n operandos. */
static int op_n(Estado *st, int ln, const Crudo *c, int n)
{
    if (c->n_ops != n) {
        snprintf(st->err, st->n_err, "linea %d: %s espera %d operandos", ln, c->mn, n);
        return 1;
    }
    return 0;
}

/* Segunda pasada para un slot: convierte la instrucción cruda en un Ins con
   registros y números según el formato de su tipo (REG, IMM, CP/AP con
   off(rbase), saltos, LOADKEY, FROUND, etc.). Los saltos guardan el nombre
   de la etiqueta; el desplazamiento se calcula después en prog_codificar. */
static int resolver(Estado *st, const Linea *l, int s, Ins *out)
{
    const Crudo *c = &l->slot[s];
    Op op = op_buscar(c->mn);
    int ln = l->num;
    long v, v2;

    memset(out, 0, sizeof *out);
    out->op = op;
    out->linea = ln;
    if (op <= OP_COMP) {
        if (op_n(st, ln, c, 3) || reg(st, ln, c->ops[0], &out->a) || reg(st, ln, c->ops[1], &out->b) ||
            reg(st, ln, c->ops[2], &out->c)) {
            return 1;
        }
    } else if (op <= OP_DLDI) {
        if (op_n(st, ln, c, 3) || reg(st, ln, c->ops[0], &out->a) || num(st, ln, c->ops[1], &v) ||
            reg(st, ln, c->ops[2], &out->c)) {
            return 1;
        }
        out->b = (int)v;
    } else if (op == OP_CP || op == OP_AP) {
        char buf[128];
        char *pa, *pc;
        size_t k = 0, i;
        if (op_n(st, ln, c, 2)) {
            return 1;
        }
        for (i = 0; c->ops[0][i] && k < sizeof buf - 1; i++) {
            if (!isspace((unsigned char)c->ops[0][i])) {
                buf[k++] = c->ops[0][i];
            }
        }
        buf[k] = '\0';
        pa = strchr(buf, '(');
        pc = strrchr(buf, ')');
        if (!pa || !pc || pc < pa || pc[1] != '\0') {
            FALLO(st, ln, "%s espera off(rbase), reg", c->mn);
        }
        *pa = '\0';
        *pc = '\0';
        if (buf[0] == '\0') {
            v = 0;
        } else if (num(st, ln, buf, &v)) {
            return 1;
        }
        out->b = (int)v;
        if (reg(st, ln, pa + 1, &out->a) || reg(st, ln, c->ops[1], &out->c)) {
            return 1;
        }
    } else if (op >= OP_SIG && op <= OP_SMQ) {
        if (op_n(st, ln, c, 3) || reg(st, ln, c->ops[0], &out->a) || reg(st, ln, c->ops[1], &out->b)) {
            return 1;
        }
        out->etq = strdup(c->ops[2]);
    } else if (op == OP_S || op == OP_SYE) {
        if (op_n(st, ln, c, 1)) {
            return 1;
        }
        out->etq = strdup(c->ops[0]);
    } else if (op == OP_SRG || op == OP_AUTH || op == OP_RDSR) {
        if (op_n(st, ln, c, 1) || reg(st, ln, c->ops[0], &out->a)) {
            return 1;
        }
    } else if (op == OP_FIN || op == OP_LOGOUT) {
        if (op_n(st, ln, c, 0)) {
            return 1;
        }
    } else if (op == OP_LOADKEY) {
        if (op_n(st, ln, c, 3) || reg(st, ln, c->ops[0], &out->a) || num(st, ln, c->ops[1], &v) ||
            num(st, ln, c->ops[2], &v2)) {
            return 1;
        }
        out->b = (int)v;
        out->c = (int)v2;
    } else if (op == OP_FROUND) {
        if (op_n(st, ln, c, 6) || reg(st, ln, c->ops[0], &out->a) || reg(st, ln, c->ops[1], &out->b) ||
            num(st, ln, c->ops[2], &v) || num(st, ln, c->ops[3], &v2) ||
            reg(st, ln, c->ops[4], &out->e) || reg(st, ln, c->ops[5], &out->f)) {
            return 1;
        }
        out->c = (int)v;
        out->d = (int)v2;
    }
    return 0;
}

/* Punto de entrada de -x:
 -   corre la primera pasada (bundles, etiquetas y directivas)
 -   resuelve cada slot y revisa que dos slots del mismo bundle no escriban
     el mismo registro
 -   pega las etiquetas a su bundle y copia la sección .data al programa
 -   llama a prog_resolver (direcciones y etiquetas repetidas)
   Devuelve 0 si va bien; si no, deja el mensaje con el número de línea. */
int ensamblar_texto(const char *texto, Programa *p, char *err, size_t n_err)
{
    Estado st;
    int i, s, k;
    int rc = 1;

    memset(&st, 0, sizeof st);
    st.err = err;
    st.n_err = n_err;
    err[0] = '\0';
    prog_iniciar(p);

    if (pasada1(&st, texto, p)) {
        return 1;
    }
    for (i = 0; i < st.n_lin; i++) {
        Bundle *b = prog_nuevo_bundle(p);
        int dests[8], nd = 0;
        for (s = 0; s < 4; s++) {
            int lee[8], ne[8], nl, nw;
            if (!st.lineas[i].usado[s]) {
                continue;
            }
            if (resolver(&st, &st.lineas[i], s, &b->ins[s])) {
                goto fin;
            }
            b->usado[s] = 1;
            ins_rw(&b->ins[s], lee, &nl, ne, &nw);
            for (k = 0; k < nw; k++) {
                int q;
                for (q = 0; q < nd; q++) {
                    if (dests[q] == ne[k]) {
                        snprintf(err, n_err, "línea %d: dos slots del bundle escriben el mismo registro", st.lineas[i].num);
                        goto fin;
                    }
                }
                dests[nd++] = ne[k];
            }
        }
    }
    /* atar etiquetas: valor = índice de bundle * 16 */
    for (i = 0; i < st.n_etq; i++) {
        if (!st.etq[i].dato) {
            long idx = st.etq[i].valor / 16;
            if (idx >= p->n) {
                snprintf(err, n_err, "etiqueta '%s' fuera del código", st.etq[i].nombre);
                goto fin;
            }
            bundle_etiqueta(&p->b[idx], st.etq[i].nombre);
        }
    }
    p->datos = st.datos;
    p->n_datos = st.n_datos;
    if (prog_resolver(p, err, n_err)) {
        goto fin;
    }
    rc = 0;
fin:
    return rc;
}
