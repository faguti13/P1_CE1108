#include "codegen.h"
#include "semantico.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*
 * Generacion de codigo. Recorre el AST ya analizado y emite el IR
 * (instrucciones con registros fisicos y etiquetas). El planificador
 * (planificador.c) las reparte despues en bundles.
 *
 * Convenciones ABI (docs/04_generacion_codigo.md y compiler-integration.md):
 *   - r8..r23: pila de temporales de las expresiones; R(d) = r(8+d).
 *   - r24..r27: auxiliares (r26 direccion grande de store, r27 constantes).
 *   - r1: valor de retorno; r28 gp; r29 sp; r30 fp; r31 ra.
 *   - argumentos en la pila: el k-esimo en -4*(k+1)(sp) del llamador.
 *   - marco: fp-4*(k+1) parametros, luego ra, fp anterior, locales y la
 *     zona de derrame de los temporales vivos durante una llamada.
 */

#define MAX_D 15
#define R_AUX 26
#define R_CONST 27

static IR *ir;
static const char *archivo;
static int errores;
static int cur_linea;
static int cur_col;
static Simbolo *fn;                 /* funcion actual; NULL en el codigo de nivel superior */
static long zona;                   /* inicio de la zona de derrame */
static char epilogo[96];
static Nodo *ultimo_ret;
static int n_etq;
static int desbordo;

/* Reporta un error de generacion (codigo 30) y sigue. */
static void error(const Nodo *n, const char *fmt, ...)
{
    va_list ap;
    char msg[400];

    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    fprintf(stderr, "{ERROR 30}:%s:%d:%d:\"%s\"\n", archivo, n ? n->linea : cur_linea,
            n ? n->col : cur_col, msg);
    errores++;
}

/* Agrega un elemento vacio al final del IR. */
static Item *nuevo_item(void)
{
    if (ir->n == ir->cap) {
        ir->cap = ir->cap ? ir->cap * 2 : 1024;
        ir->v = realloc(ir->v, (size_t)ir->cap * sizeof(Item));
        if (!ir->v) {
            fprintf(stderr, "sin memoria\n");
            exit(1);
        }
    }
    memset(&ir->v[ir->n], 0, sizeof(Item));
    return &ir->v[ir->n++];
}

/* Emite una instruccion de tres operandos. */
static void emit(Op op, int a, int b, int c)
{
    Item *it = nuevo_item();
    it->ins.op = op;
    it->ins.a = a;
    it->ins.b = b;
    it->ins.c = c;
    it->ins.linea = cur_linea;
}

/* Emite un FROUND: L, R, indice de llave, ronda y los dos destinos. */
static void emit_fround(int a, int b, int k, int r, int dl, int dr)
{
    Item *it = nuevo_item();
    it->ins.op = OP_FROUND;
    it->ins.a = a;
    it->ins.b = b;
    it->ins.c = k;
    it->ins.d = r;
    it->ins.e = dl;
    it->ins.f = dr;
    it->ins.linea = cur_linea;
}

/* Emite un salto cuyo destino todavia es un nombre de etiqueta. */
static void emit_salto(Op op, int a, int b, const char *etq)
{
    Item *it = nuevo_item();
    it->ins.op = op;
    it->ins.a = a;
    it->ins.b = b;
    it->ins.etq = strdup(etq);
    it->ins.linea = cur_linea;
}

/* Marca un punto del IR con una etiqueta, sin emitir instruccion. */
static void etiqueta(const char *nombre)
{
    Item *it = nuevo_item();
    it->es_etq = 1;
    it->etq = strdup(nombre);
}

/* Reserva un nombre L_n nuevo para saltos internos. */
static char *nueva_etq(void)
{
    char buf[32];
    snprintf(buf, sizeof buf, "L_%d", n_etq++);
    return strdup(buf);
}

/* Temporal de profundidad d: r(8+d). Si pasa de r23, error 30. */
static int R(int d)
{
    if (d > MAX_D) {
        if (!desbordo) {
            error(NULL, "expresion demasiado compleja: se agotan los registros temporales");
        }
        desbordo = 1;
        return 8 + MAX_D;
    }
    return 8 + d;
}

/* Cierto si v cabe en el inmediato de 15 bits con signo. */
static int cabe15(long v)
{
    return v >= ISA_IMM_MIN && v <= ISA_IMM_MAX;
}

/*
 * Carga un entero de 32 bits en rd. No hay propagacion de constantes:
 * el literal se materializa aqui, no se pliega en el AST.
 * Si cabe en 15 bits basta un SUMI r0, v, rd. Si no, se parte en
 * trozos de 11 bits con SUMI y DLII (docs/isa.md).
 */
static void const_load(int rd, long v)
{
    uint32_t u = (uint32_t)v;
    long s = (int32_t)u;

    if (cabe15(s)) {
        emit(OP_SUMI, 0, (int)s, rd);
        return;
    }
    if ((u >> 11) <= 16383u) {
        emit(OP_SUMI, 0, (int)(u >> 11), rd);
        emit(OP_DLII, rd, 11, rd);
        if (u & 0x7FF) {
            emit(OP_SUMI, rd, (int)(u & 0x7FF), rd);
        }
        return;
    }
    emit(OP_SUMI, 0, (int)(u >> 22), rd);
    emit(OP_DLII, rd, 11, rd);
    emit(OP_SUMI, rd, (int)((u >> 11) & 0x7FF), rd);
    emit(OP_DLII, rd, 11, rd);
    if (u & 0x7FF) {
        emit(OP_SUMI, rd, (int)(u & 0x7FF), rd);
    }
}

/* rd = rs + v. Usa SUMI si cabe; si no, arma v en r27 y suma. */
static void add_imm(int rd, int rs, long v)
{
    if (cabe15(v)) {
        emit(OP_SUMI, rs, (int)v, rd);
    } else {
        const_load(R_CONST, v);
        emit(OP_SUM, rs, R_CONST, rd);
    }
}

/* Carga la palabra base+off en rd (CP, o suma previa si el offset no cabe). */
static void mem_ld(int rd, int base, long off)
{
    if (cabe15(off)) {
        emit(OP_CP, base, (int)off, rd);
    } else {
        const_load(rd, off);
        emit(OP_SUM, base, rd, rd);
        emit(OP_CP, rd, 0, rd);
    }
}

/* Guarda rs en base+off (AP). El offset grande se arma en r26. */
static void mem_st(int rs, int base, long off)
{
    if (cabe15(off)) {
        emit(OP_AP, base, (int)off, rs);
    } else {
        const_load(R_AUX, off);
        emit(OP_SUM, base, R_AUX, R_AUX);
        emit(OP_AP, R_AUX, 0, rs);
    }
}

/* gp si la variable es global; fp si es local o parametro. */
static int base_de(const Simbolo *s)
{
    return s->alm == AL_GLOBAL ? R_GP : R_FP;
}

/* Carga una variable escalar desde su direccion de gp o fp. */
static void load_var(int rd, const Simbolo *s)
{
    mem_ld(rd, base_de(s), s->offset);
}

/* Guarda un escalar en la direccion de la variable. */
static void store_var(int rs, const Simbolo *s)
{
    mem_st(rs, base_de(s), s->offset);
}

/* Direccion de la lista (una lista parametro ya guarda la direccion). */
static void addr_lista(int rd, const Simbolo *s)
{
    if (s->clase == CL_PARAM) {
        load_var(rd, s);
    } else {
        add_imm(rd, base_de(s), s->offset);
    }
}

/* Desplazamiento, desde fp, del temporal i en la zona de derrame. */
static long off_derrame(int i)
{
    return -(zona + 4L * (i + 1));
}

/* ---------------- expresiones ---------------- */

static void gen_expr(Nodo *n, int d);
static void gen_call(Nodo *n, int d, int valor);

/* Cierto si n es el literal entero 0 (se compara contra r0). */
static int es_cero(const Nodo *n)
{
    long v;
    return n->tipo == NODO_LITERAL && n->ty == TY_ENT && sem_const(n, &v) && v == 0;
}

/* Cierto si n es un entero literal que cabe en el inmediato. */
static int lit_pequeno(const Nodo *n, long *v)
{
    return n->tipo == NODO_LITERAL && n->ty == TY_ENT && sem_const(n, v) && cabe15(*v);
}

/*
 * Deja en R(d) la direccion del elemento, no su valor. El dato esta
 * en 8(R(d)): el encabezado ocupa [dim0, dim1] y los datos empiezan en +8.
 * Indice lineal: i*dim1+j en 2D (dim1 se lee del encabezado, offset 4),
 * o solo i en 1D. Luego se desplaza 2 bits porque cada elemento son 4 bytes.
 * Los indices van de 0 a dim-1. Una lista 1D tiene dim1 = 1.
 */
static void gen_elem_addr(Nodo *n, int d)
{
    Simbolo *s = n->sim;
    Nodo *i = n->hijo;
    Nodo *j = i ? i->sig : NULL;

    addr_lista(R(d), s);
    gen_expr(i, d + 1);
    if (j) {
        gen_expr(j, d + 2);
        emit(OP_CP, R(d), 4, R(d + 3));            /* dim1 del encabezado */
        emit(OP_MUL, R(d + 1), R(d + 3), R(d + 1));
        emit(OP_SUM, R(d + 1), R(d + 2), R(d + 1));
    }
    emit(OP_DLII, R(d + 1), 2, R(d + 1));
    emit(OP_SUM, R(d), R(d + 1), R(d));
}

/* Niega un bool en r: deja 1-r. Sirve para <=, >= y ==. */
static void flip(int r)
{
    emit(OP_RESTI, r, 1, r);
    emit(OP_MULI, r, -1, r);          /* 1 - x */
}

/*
 * Comparacion usada como valor (el resultado se guarda), no como salto.
 * COMP es "menor estricto con signo" y escribe 0 o 1; no es SIG/BEQ ni AUTH.
 * < y > son un COMP (con operandos cruzados). <= y >= son el COMP contrario
 * negado con flip. == y != restan y comprueban x<0 o 0<x; == niega ese !=.
 */
static void gen_cmp_valor(const char *op, int d)
{
    int a = R(d), b = R(d + 1), t = R(d + 1);

    if (!strcmp(op, "<")) {
        emit(OP_COMP, a, b, a);
    } else if (!strcmp(op, ">")) {
        emit(OP_COMP, b, a, a);
    } else if (!strcmp(op, "<=")) {
        emit(OP_COMP, b, a, a);
        flip(a);
    } else if (!strcmp(op, ">=")) {
        emit(OP_COMP, a, b, a);
        flip(a);
    } else {                          /* == y != */
        emit(OP_REST, a, b, a);
        emit(OP_COMP, 0, a, t);       /* 0 < x */
        emit(OP_COMP, a, 0, a);       /* x < 0 */
        emit(OP_OLO, a, t, a);        /* x != 0 */
        if (!strcmp(op, "==")) {
            flip(a);
        }
    }
}

/*
 * Deja el valor en R(d). El operando derecho usa R(d+1); el modulo usa ademas R(d+2).
 * + - * / con literal chico a la derecha salen como SUMI/RESTI/MULI/DIVI.
 * El % siempre es DIV, MUL y REST en registros, aunque el divisor sea 2.
 * && y || aqui evaluan ambos lados (OLY/OLO). El corto-circuito vive en gen_jump.
 */
static void gen_expr(Nodo *n, int d)
{
    const char *op;
    long v;

    if (!n) {
        return;
    }
    cur_linea = n->linea;
    cur_col = n->col;
    switch (n->tipo) {
    case NODO_LITERAL:
        const_load(R(d), n->val);
        return;
    case NODO_IDENT:
        if (n->sim->ty == TY_LISTA_ENT || n->sim->ty == TY_LISTA_BOOL) {
            addr_lista(R(d), n->sim);
        } else {
            load_var(R(d), n->sim);
        }
        return;
    case NODO_INDICE:
        gen_elem_addr(n, d);
        emit(OP_CP, R(d), 8, R(d));
        return;
    case NODO_LLAMADA:
        gen_call(n, d, 1);
        return;
    case NODO_EXP:
        break;
    default:
        error(n, "expresion no generable");
        return;
    }
    op = n->texto;
    if (!strcmp(op, "!")) {
        gen_expr(n->hijo, d);
        flip(R(d));
        return;
    }
    if (!strcmp(op, "neg")) {
        gen_expr(n->hijo, d);
        emit(OP_REST, 0, R(d), R(d));
        return;
    }
    gen_expr(n->hijo, d);
    if (strchr("+-*/", op[0]) && !op[1] && lit_pequeno(n->hijo->sig, &v)) {
        Op o = op[0] == '+' ? OP_SUMI : op[0] == '-' ? OP_RESTI : op[0] == '*' ? OP_MULI : OP_DIVI;
        emit(o, R(d), (int)v, R(d));
        return;
    }
    gen_expr(n->hijo->sig, d + 1);
    if (!strcmp(op, "+")) {
        emit(OP_SUM, R(d), R(d + 1), R(d));
    } else if (!strcmp(op, "-")) {
        emit(OP_REST, R(d), R(d + 1), R(d));
    } else if (!strcmp(op, "*")) {
        emit(OP_MUL, R(d), R(d + 1), R(d));
    } else if (!strcmp(op, "/")) {
        emit(OP_DIV, R(d), R(d + 1), R(d));
    } else if (!strcmp(op, "%")) {
        emit(OP_DIV, R(d), R(d + 1), R(d + 2));
        emit(OP_MUL, R(d + 2), R(d + 1), R(d + 2));
        emit(OP_REST, R(d), R(d + 2), R(d));
    } else if (!strcmp(op, "&&")) {
        emit(OP_OLY, R(d), R(d + 1), R(d));
    } else if (!strcmp(op, "||")) {
        emit(OP_OLO, R(d), R(d + 1), R(d));
    } else {
        gen_cmp_valor(op, d);
    }
}

/*
 * Salta a etq cuando la condicion vale `si` (1 = verdadera, 0 = falsa).
 * El `si` de JAF llama con si=0: salta al sino si la condicion es falsa.
 * && y || cortocircuitan con un salto intermedio; ! invierte `si`.
 * Las comparaciones saltan con SIG/SNIG/SMI/SMQ, no con COMP.
 * Un literal 0 se compara contra r0 y no se carga.
 */
static void gen_jump(Nodo *n, const char *etq, int si, int d)
{
    const char *op;
    Nodo *l, *r;
    int a, b;
    Op o;
    int cruzar;

    if (n->tipo == NODO_LITERAL && n->ty == TY_BOOL) {
        if ((n->val != 0) == (si != 0)) {
            emit_salto(OP_S, 0, 0, etq);
        }
        return;
    }
    if (n->tipo == NODO_EXP) {
        op = n->texto;
        l = n->hijo;
        r = l ? l->sig : NULL;
        if (!strcmp(op, "&&")) {
            if (!si) {
                gen_jump(l, etq, 0, d);
                gen_jump(r, etq, 0, d);
            } else {
                char *skip = nueva_etq();
                gen_jump(l, skip, 0, d);
                gen_jump(r, etq, 1, d);
                etiqueta(skip);
            }
            return;
        }
        if (!strcmp(op, "||")) {
            if (si) {
                gen_jump(l, etq, 1, d);
                gen_jump(r, etq, 1, d);
            } else {
                char *skip = nueva_etq();
                gen_jump(l, skip, 1, d);
                gen_jump(r, etq, 0, d);
                etiqueta(skip);
            }
            return;
        }
        if (!strcmp(op, "!")) {
            gen_jump(l, etq, !si, d);
            return;
        }
        if (!strcmp(op, "==") || !strcmp(op, "!=") || !strcmp(op, "<") || !strcmp(op, ">") ||
            !strcmp(op, "<=") || !strcmp(op, ">=")) {
            if (es_cero(l)) {
                a = 0;
            } else {
                gen_expr(l, d);
                a = R(d);
            }
            if (es_cero(r)) {
                b = 0;
            } else {
                gen_expr(r, d + 1);
                b = R(d + 1);
            }
            if (!strcmp(op, "==")) {
                o = si ? OP_SIG : OP_SNIG; cruzar = 0;
            } else if (!strcmp(op, "!=")) {
                o = si ? OP_SNIG : OP_SIG; cruzar = 0;
            } else if (!strcmp(op, "<")) {
                o = si ? OP_SMQ : OP_SMI; cruzar = 0;
            } else if (!strcmp(op, ">=")) {
                o = si ? OP_SMI : OP_SMQ; cruzar = 0;
            } else if (!strcmp(op, ">")) {
                o = si ? OP_SMQ : OP_SMI; cruzar = 1;
            } else {
                o = si ? OP_SMI : OP_SMQ; cruzar = 1;
            }
            emit_salto(o, cruzar ? b : a, cruzar ? a : b, etq);
            return;
        }
    }
    gen_expr(n, d);
    emit_salto(si ? OP_SNIG : OP_SIG, R(d), 0, etq);
}

/* ---------------- llamadas e intrinsecas ---------------- */

/* Escribe reg en una variable o en el elemento de lista ya direccionado. */
static void gen_store_lvalue(Nodo *lv, int reg, int d)
{
    if (lv->tipo == NODO_IDENT) {
        store_var(reg, lv->sim);
    } else {
        gen_elem_addr(lv, d);
        emit(OP_AP, R(d), 8, reg);
    }
}

/*
 * Intrinsecas del slot 3 y de memoria cruda: AUTH, LOGOUT, RDSR, LOADKEY,
 * CP/AP a una direccion, y FROUND. Cifrar son las rondas 0..3; descifrar
 * son 3..0 con L y R cruzados. FROUND escribe dos destinos y luego se
 * guardan en los lvalues. K, S y el numero de ronda ya vienen plegados.
 */
static void gen_intrinseca(Nodo *n, int d, int valor)
{
    Simbolo *f = n->sim;
    Nodo *a0 = n->hijo;
    Nodo *a1 = a0 ? a0->sig : NULL;
    Nodo *a2 = a1 ? a1->sig : NULL;
    Nodo *a3 = a2 ? a2->sig : NULL;
    long k = 0, r = 0, s = 0;
    int A = R(d), B = R(d + 1), C = R(d + 2), E = R(d + 3);
    int i;

    (void)valor;
    switch (f->intr) {
    case IN_AUTENTICAR:
        gen_expr(a0, d);
        emit(OP_AUTH, A, 0, 0);
        break;
    case IN_CERRAR_SESION:
        emit(OP_LOGOUT, 0, 0, 0);
        break;
    case IN_LEER_ESTADO:
        emit(OP_RDSR, A, 0, 0);
        break;
    case IN_CARGAR_LLAVE:
        sem_const(a1, &k);
        sem_const(a2, &s);
        gen_expr(a0, d);
        emit(OP_LOADKEY, A, (int)k, (int)s);
        emit(OP_SUM, 0, 0, A);                 /* la subllave no se queda en el GPR */
        break;
    case IN_LEER_MEM:
        gen_expr(a0, d);
        emit(OP_CP, A, 0, A);
        break;
    case IN_ESCRIBIR_MEM:
        gen_expr(a0, d);
        gen_expr(a1, d + 1);
        emit(OP_AP, A, 0, B);
        break;
    case IN_RONDA_FEISTEL:
        sem_const(a2, &k);
        sem_const(a3, &r);
        gen_expr(a0, d);
        gen_expr(a1, d + 1);
        emit_fround(A, B, (int)k, (int)r, C, E);
        gen_store_lvalue(a0, C, d + 4);
        gen_store_lvalue(a1, E, d + 4);
        break;
    case IN_FEISTEL_CIFRAR:
        sem_const(a2, &k);
        gen_expr(a0, d);
        gen_expr(a1, d + 1);
        emit_fround(A, B, (int)k, 0, C, E);
        emit_fround(C, E, (int)k, 1, A, B);
        emit_fround(A, B, (int)k, 2, C, E);
        emit_fround(C, E, (int)k, 3, A, B);
        gen_store_lvalue(a0, A, d + 4);
        gen_store_lvalue(a1, B, d + 4);
        break;
    case IN_FEISTEL_DESCIFRAR:
        sem_const(a2, &k);
        gen_expr(a0, d);
        gen_expr(a1, d + 1);
        /* descifrado: rondas 3..0 con operandos cruzados (docs/isa.md) */
        emit_fround(B, A, (int)k, 3, C, E);
        emit_fround(C, E, (int)k, 2, B, A);
        emit_fround(B, A, (int)k, 1, C, E);
        emit_fround(C, E, (int)k, 0, B, A);
        gen_store_lvalue(a0, A, d + 4);
        gen_store_lvalue(a1, B, d + 4);
        break;
    default:
        break;
    }
    (void)i;
}

/*
 * Llamada a f_nombre. El derrame guarda r8..R(d-1) porque el callee puede
 * pisarlos; no son los argumentos. Cada argumento k se evalua en R(d+k)
 * y se escribe en -4*(k+1)(sp) antes del SYE. Tras el prologo del callee
 * esa celda es fp-4*(k+1). Si la llamada es expresion, r1 se mueve a R(d).
 */
static void gen_call(Nodo *n, int d, int valor)
{
    Simbolo *f = n->sim;
    Nodo *a;
    int k, i;
    char destino[128];

    if (!f) {
        return;
    }
    if (f->clase == CL_INTRINSECA) {
        gen_intrinseca(n, d, valor);
        return;
    }
    for (i = 0; i < d; i++) {                 /* derrame de los temporales vivos */
        mem_st(R(i), R_FP, off_derrame(i));
    }
    for (a = n->hijo, k = 0; a; a = a->sig, k++) {
        gen_expr(a, d + k);
    }
    for (a = n->hijo, k = 0; a; a = a->sig, k++) {
        mem_st(R(d + k), R_SP, -4L * (k + 1));
    }
    snprintf(destino, sizeof destino, "f_%s", f->nombre);
    cur_linea = n->linea;
    emit_salto(OP_SYE, 0, 0, destino);
    if (valor) {
        emit(OP_SUM, R_RET, 0, R(d));
    }
    for (i = 0; i < d; i++) {
        mem_ld(R(i), R_FP, off_derrame(i));
    }
}

/* ---------------- sentencias ---------------- */

static void gen_stmt(Nodo *n);

/* Escribe los valores iniciales de una lista a partir de base+off. */
static void gen_literal_lista(Nodo *lit, int base_reg, long off, int d)
{
    Nodo *h;
    long i = 0;

    for (h = lit->hijo; h; h = h->sig, i++) {
        if (h->tipo == NODO_LISTA_LIT) {
            long ancho = 0;
            Nodo *x;
            for (x = h->hijo; x; x = x->sig) {
                ancho++;
            }
            gen_literal_lista(h, base_reg, off + 4 * ancho * i, d);
        } else {
            gen_expr(h, d);
            mem_st(R(d), base_reg, off + 4 * i);
        }
    }
}

/*
 * Declara una lista: dim0 en offset 0, dim1 en offset 4, datos desde +8.
 * Con literal copia los valores. Sin literal y hasta 16 elementos, un AP
 * de r0 por celda. Si es mas grande, un bucle que recorre [base+8, base+8+4*n).
 */
static void gen_decl_lista(Nodo *n)
{
    Simbolo *s = n->sim;
    Nodo *val = n->hijo && n->hijo->sig && n->hijo->sig->sig ? n->hijo->sig->sig : NULL;
    long total = s->dim[0] * s->dim[1];
    long i;

    if (!s) {
        return;
    }
    addr_lista(R(1), s);
    const_load(R(0), s->dim[0]);
    emit(OP_AP, R(1), 0, R(0));
    const_load(R(0), s->dim[1]);
    emit(OP_AP, R(1), 4, R(0));
    if (val && val->tipo == NODO_LISTA_LIT) {
        gen_literal_lista(val, R(1), 8, 2);
    } else if (total <= 16) {
        for (i = 0; i < total; i++) {
            emit(OP_AP, R(1), (int)(8 + 4 * i), 0);
        }
    } else {
        char *top = nueva_etq();
        add_imm(R(2), R(1), 8);
        add_imm(R(3), R(1), 8 + 4 * total);
        etiqueta(top);
        emit(OP_AP, R(2), 0, 0);
        emit(OP_SUMI, R(2), 4, R(2));
        emit_salto(OP_SMQ, R(2), R(3), top);
    }
}

/* Asignacion simple o +=/-=, a variable o a elemento de lista. */
static void gen_asignacion(Nodo *n)
{
    Nodo *dst = n->hijo;
    Nodo *val = dst->sig;
    int comp = strcmp(n->texto, "=") != 0;
    Op op = n->texto[0] == '+' ? OP_SUM : OP_REST;

    if (dst->tipo == NODO_IDENT) {
        if (comp) {
            long v;
            load_var(R(0), dst->sim);
            if (lit_pequeno(val, &v)) {
                emit(n->texto[0] == '+' ? OP_SUMI : OP_RESTI, R(0), (int)v, R(0));
            } else {
                gen_expr(val, 1);
                emit(op, R(0), R(1), R(0));
            }
        } else {
            gen_expr(val, 0);
        }
        store_var(R(0), dst->sim);
        return;
    }
    gen_elem_addr(dst, 0);                    /* R(0) */
    if (comp) {
        emit(OP_CP, R(0), 8, R(1));
        gen_expr(val, 2);
        emit(op, R(1), R(2), R(1));
    } else {
        gen_expr(val, 1);
    }
    emit(OP_AP, R(0), 8, R(1));
}

/*
 * mientras y para. La prueba queda al final: un S entra a la condicion y,
 * si es verdadera, un salto condicional vuelve al cuerpo. `act` (el paso
 * del para) se emite entre el cuerpo y la prueba.
 */
static void gen_cuerpo_bucle(Nodo *init, Nodo *cond, Nodo *act, Nodo *cuerpo)
{
    char *l_cond = nueva_etq();
    char *l_cuerpo = nueva_etq();

    if (init) {
        gen_stmt(init);
    }
    emit_salto(OP_S, 0, 0, l_cond);
    etiqueta(l_cuerpo);
    gen_stmt(cuerpo);
    if (act) {
        gen_stmt(act);
    }
    etiqueta(l_cond);
    cur_linea = cond->linea;
    gen_jump(cond, l_cuerpo, 1, 0);
}

/* Genera una sentencia: bloque, declaracion, si, bucle, retorno o terminar. */
static void gen_stmt(Nodo *n)
{
    Nodo *h;

    if (!n) {
        return;
    }
    cur_linea = n->linea;
    cur_col = n->col;
    switch (n->tipo) {
    case NODO_BLOQUE:
        for (h = n->hijo; h; h = h->sig) {
            gen_stmt(h);
        }
        break;
    case NODO_DECLARACION:
        if (!n->sim) {
            break;
        }
        if (n->hijo && n->hijo->sig) {
            gen_expr(n->hijo->sig, 0);
            store_var(R(0), n->sim);
        } else {
            mem_st(0, base_de(n->sim), n->sim->offset);
        }
        break;
    case NODO_DECL_LISTA:
        if (n->sim) {
            gen_decl_lista(n);
        }
        break;
    case NODO_ASIGNACION:
        gen_asignacion(n);
        break;
    case NODO_SI: {
        Nodo *cond = n->hijo;
        Nodo *entonces = cond->sig;
        Nodo *sino = entonces ? entonces->sig : NULL;
        char *l_sino = nueva_etq();
        gen_jump(cond, l_sino, 0, 0);
        gen_stmt(entonces);
        if (sino) {
            char *l_fin = nueva_etq();
            emit_salto(OP_S, 0, 0, l_fin);
            etiqueta(l_sino);
            gen_stmt(sino);
            etiqueta(l_fin);
        } else {
            etiqueta(l_sino);
        }
        break;
    }
    case NODO_MIENTRAS:
        gen_cuerpo_bucle(NULL, n->hijo, NULL, n->hijo->sig);
        break;
    case NODO_PARA: {
        Nodo *init = n->hijo;
        Nodo *cond = init->sig;
        Nodo *act = cond->sig;
        Nodo *cuerpo = act->sig;
        gen_cuerpo_bucle(init, cond, act, cuerpo);
        break;
    }
    case NODO_RETORNO:
        if (n->hijo) {
            gen_expr(n->hijo, 0);
            emit(OP_SUM, R(0), 0, R_RET);
        }
        if (n != ultimo_ret) {
            emit_salto(OP_S, 0, 0, epilogo);
        }
        break;
    case NODO_LLAMADA:
        gen_call(n, 0, 0);
        break;
    case NODO_TERMINAR:
        emit(OP_FIN, 0, 0, 0);
        break;
    case NODO_LISTA:
        break;
    default:
        error(n, "sentencia no generable");
        break;
    }
}

/*
 * Una funcion es la etiqueta f_nombre, el cuerpo y f_nombre_fin.
 * Prologo: guarda el fp anterior en -(4*nparams+8)(sp) y ra en
 * -(4*nparams+4)(sp), copia sp a fp y baja sp el tamano del marco.
 * Epilogo: restaura ra y el fp anterior, devuelve sp a fp y hace SRG r31.
 * Si el ultimo statement ya es el retorno, no se inserta un salto extra.
 */
static void gen_funcion(Nodo *f)
{
    Simbolo *s = f->sim;
    Nodo *cuerpo = f->hijo && f->hijo->sig ? f->hijo->sig->sig : NULL;
    Nodo *h, *ult = NULL;
    char nombre[128];
    long np = s->nparams;

    fn = s;
    zona = s->zona_derrame;
    cur_linea = f->linea;
    snprintf(nombre, sizeof nombre, "f_%s", s->nombre);
    snprintf(epilogo, sizeof epilogo, "f_%s_fin", s->nombre);
    etiqueta(nombre);
    /* prologo: guarda fp y ra bajo los argumentos, crea el marco */
    mem_st(R_FP, R_SP, -(4 * np + 8));
    mem_st(R_RA, R_SP, -(4 * np + 4));
    emit(OP_SUM, R_SP, 0, R_FP);
    if (cabe15(s->marco)) {
        emit(OP_SUMI, R_SP, (int)-s->marco, R_SP);
    } else {
        const_load(R_CONST, s->marco);
        emit(OP_REST, R_SP, R_CONST, R_SP);
    }
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        ult = h;
    }
    ultimo_ret = (ult && ult->tipo == NODO_RETORNO) ? ult : NULL;
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        gen_stmt(h);
    }
    etiqueta(epilogo);
    mem_ld(R_RA, R_FP, -(4 * np + 4));
    mem_ld(R_CONST, R_FP, -(4 * np + 8));
    emit(OP_SUM, R_FP, 0, R_SP);
    emit(OP_SUM, R_CONST, 0, R_FP);
    emit(OP_SRG, R_RA, 0, 0);
    fn = NULL;
    ultimo_ret = NULL;
}

/*
 * Punto de entrada del generador. _inicio carga gp (0x10000) y sp
 * (tope de pila), abre el marco global, emite las sentencias de nivel
 * superior, llama a f_main si existe y cierra con FIN. Las funciones
 * van despues, planas: el anidamiento es solo de la pila, no del texto.
 */
int gen_programa(Nodo *raiz, IR *out, const char *arch)
{
    Nodo *cuerpo = raiz->hijo ? raiz->hijo->sig : NULL;
    Nodo *h;
    int i;
    int hay_main = 0;

    ir = out;
    archivo = arch;
    errores = 0;
    n_etq = 0;
    desbordo = 0;
    fn = NULL;
    zona = 0;
    ultimo_ret = NULL;
    cur_linea = 1;
    cur_col = 1;

    for (i = 0; i < sem.n_todos; i++) {
        if (sem.todos[i]->clase == CL_FUNC && !strcmp(sem.todos[i]->nombre, "main")) {
            hay_main = 1;
        }
    }

    etiqueta("_inicio");
    const_load(R_GP, MEM_GP);
    const_load(R_SP, MEM_STACK_TOP);
    emit(OP_SUM, R_SP, 0, R_FP);
    emit(OP_SUMI, R_SP, (int)-sem.marco_inicio, R_SP);
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        if (h->tipo != NODO_FUNCION) {
            gen_stmt(h);
        }
    }
    if (hay_main) {
        emit_salto(OP_SYE, 0, 0, "f_main");
    }
    emit(OP_FIN, 0, 0, 0);
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        if (h->tipo == NODO_FUNCION && h->sim) {
            gen_funcion(h);
        }
    }
    return errores;
}
