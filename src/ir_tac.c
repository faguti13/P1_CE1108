#include "ir_tac.h"
#include "isa.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*
 * Generacion del codigo de 3 direcciones desde el AST ya analizado.
 * Misma semantica que el generador de P1 (codegen.c):
 *   - lazos con la prueba al final (goto a la condicion, cuerpo, paso, condicion)
 *   - && y || como valor evaluan ambos lados; en una condicion hacen corto circuito
 *   - listas: encabezado [dim0, dim1] y datos desde +8, indice lineal i*dim1+j
 *   - argumentos: se evaluan todos y luego se pasan (param k), y despues el call
 *   - cargar_llave sobre un temporal lo borra despues (clear)
 * Diferencias: las variables escalares locales y los parametros son registros
 * virtuales (no memoria), las expresiones constantes se pliegan y los
 * desplazamientos constantes de las listas van dentro de la direccion.
 */

static TacProg *prog;
static TacFunc *F;
static int n_etq;
static int cur_linea;
static int errores;
static const char *archivo;

/* ---------------- utilidades del IR ---------------- */

// Hace: redimensiona memoria o aborta ante un error; recibe: p, n; genera: puntero o NULL segun corresponda.
static void *xrealloc(void *p, size_t n)
{
    p = realloc(p, n ? n : 1);
    if (!p) {
        fprintf(stderr, "sin memoria\n");
        exit(1);
    }
    return p;
}

// Hace: crea una variable virtual en la funcion; recibe: fn, nombre; genera: estado o cantidad calculada.
int tac_nuevo_v(TacFunc *fn, const char *nombre)
{
    int v = fn->nv++;
    fn->vnombre = xrealloc(fn->vnombre, (size_t)fn->nv * sizeof(char *));
    fn->vsim = xrealloc(fn->vsim, (size_t)fn->nv * sizeof(Simbolo *));
    fn->vparam = xrealloc(fn->vparam, (size_t)fn->nv * sizeof(int));
    fn->vnombre[v] = nombre ? strdup(nombre) : NULL;
    fn->vsim[v] = NULL;
    fn->vparam[v] = -1;
    return v;
}

// Hace: inserta una instruccion TAC en una posicion; recibe: fn, pos; genera: puntero o NULL segun corresponda.
Tac *tac_insertar(TacFunc *fn, int pos)
{
    if (fn->n == fn->cap) {
        fn->cap = fn->cap ? fn->cap * 2 : 256;
        fn->v = xrealloc(fn->v, (size_t)fn->cap * sizeof(Tac));
    }
    memmove(&fn->v[pos + 1], &fn->v[pos], (size_t)(fn->n - pos) * sizeof(Tac));
    fn->n++;
    memset(&fn->v[pos], 0, sizeof(Tac));
    fn->v[pos].d = fn->v[pos].d2 = -1;
    return &fn->v[pos];
}

// Hace: elimina una instruccion TAC; recibe: fn, pos; genera: efectos sobre estructuras o salida.
void tac_borrar(TacFunc *fn, int pos)
{
    free(fn->v[pos].etq);
    memmove(&fn->v[pos], &fn->v[pos + 1], (size_t)(fn->n - pos - 1) * sizeof(Tac));
    fn->n--;
}

// Hace: agrega una instruccion TAC al bloque actual; recibe: op; genera: puntero o NULL segun corresponda.
static Tac *emit(TacOp op)
{
    Tac *t = tac_insertar(F, F->n);
    t->op = op;
    t->linea = cur_linea;
    return t;
}

// Hace: construye un operando de variable virtual; recibe: v; genera: valor o indicador calculado.
static Opnd V(int v) { Opnd o = {OP_V, v}; return o; }
// Hace: construye un operando constante; recibe: c; genera: valor o indicador calculado.
static Opnd C(long c) { Opnd o = {OP_C, c}; return o; }
// Hace: construye un operando de registro; recibe: r; genera: valor o indicador calculado.
static Opnd R(int r) { Opnd o = {OP_R, r}; return o; }
// Hace: construye un operando vacio; recibe: sin parametros; genera: valor o indicador calculado.
static Opnd NADA(void) { Opnd o = {OP_NADA, 0}; return o; }

// Hace: reserva un temporal virtual; recibe: sin parametros; genera: valor o indicador calculado.
static int nuevo_t(void) { return tac_nuevo_v(F, NULL); }

// Hace: crea una etiqueta unica; recibe: sin parametros; genera: puntero o NULL segun corresponda.
static char *nueva_etq(void)
{
    char buf[32];
    snprintf(buf, sizeof buf, "L_%d", n_etq++);
    return strdup(buf);
}

// Hace: reporta un error al generar TAC; recibe: n, fmt, argumentos variables; genera: efectos sobre estructuras o salida.
static void error(const Nodo *n, const char *fmt, ...)
{
    va_list ap;
    char msg[300];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    fprintf(stderr, "{ERROR 30}:%s:%d:%d:\"%s\"\n", n && n->archivo ? n->archivo : archivo, n ? n->linea : cur_linea,
            n ? n->col : 0, msg);
    errores++;
}

// Hace: identifica los virtuales leidos por una instruccion; recibe: t, v; genera: estado o cantidad calculada.
int tac_lee(const Tac *t, int *v)
{
    int n = 0;
    if (t->op == TAC_CLEAR || t->op == TAC_LABEL) {
        return 0;
    }
    if (t->a.k == OP_V) {
        v[n++] = (int)t->a.v;
    }
    if (t->b.k == OP_V) {
        v[n++] = (int)t->b.v;
    }
    return n;
}

// Hace: identifica los virtuales escritos por una instruccion; recibe: t, v; genera: estado o cantidad calculada.
int tac_escribe(const Tac *t, int *v)
{
    int n = 0;
    if (t->d >= 0) {
        v[n++] = t->d;
    }
    if (t->d2 >= 0) {
        v[n++] = t->d2;
    }
    return n;
}

// Hace: determina si una instruccion produce efectos observables; recibe: t; genera: estado o cantidad calculada.
int tac_efecto(const Tac *t)
{
    switch (t->op) {
    case TAC_STORE: case TAC_LABEL: case TAC_JMP: case TAC_BCOND: case TAC_JZ: case TAC_JNZ:
    case TAC_PARAM: case TAC_CALL: case TAC_RET: case TAC_HALT: case TAC_AUTH: case TAC_LOGOUT:
    case TAC_LOADKEY: case TAC_FROUND: case TAC_CLEAR:
        return 1;
    default:
        return 0;
    }
}

// Hace: detecta instrucciones de salto; recibe: t; genera: estado o cantidad calculada.
int tac_es_salto(const Tac *t)
{
    return t->op == TAC_JMP || t->op == TAC_BCOND || t->op == TAC_JZ || t->op == TAC_JNZ;
}

// Hace: determina si una instruccion cierra un bloque; recibe: t; genera: estado o cantidad calculada.
int tac_termina_bloque(const Tac *t)
{
    return tac_es_salto(t) || t->op == TAC_RET || t->op == TAC_HALT;
}

// Hace: cuenta instrucciones del programa TAC; recibe: p; genera: valor o indicador calculado.
long tac_contar(const TacProg *p)
{
    long n = 0;
    int i, k;
    for (i = 0; i < p->n; i++) {
        for (k = 0; k < p->f[i].n; k++) {
            n += p->f[i].v[k].op != TAC_LABEL;
        }
    }
    return n;
}

/* ---------------- variables ---------------- */

// Hace: detecta un tipo de lista; recibe: ty; genera: valor o indicador calculado.
static int es_lista_ty(int ty) { return ty == TY_LISTA_ENT || ty == TY_LISTA_BOOL; }

/* Escalar local o parametro (incluida la direccion de una lista parametro): vive en un virtual. */
// Hace: determina el manejo de un simbolo en registros; recibe: s; genera: valor o indicador calculado.
static int promovido(const Simbolo *s)
{
    if (s->alm == AL_PARAM) {
        return 1;
    }
    return s->alm == AL_LOCAL && !es_lista_ty(s->ty);
}

// Hace: obtiene el registro base de un simbolo; recibe: s; genera: valor o indicador calculado.
static int base_reg(const Simbolo *s)
{
    return s->alm == AL_GLOBAL ? R_GP : R_FP;
}

// Hace: obtiene o crea el virtual de un simbolo; recibe: s; genera: valor o indicador calculado.
static int vreg_de(Simbolo *s)
{
    if (!s->tac_v) {
        int v = tac_nuevo_v(F, s->nombre);
        F->vsim[v] = s;
        s->tac_v = v + 1;
    }
    return s->tac_v - 1;
}

// Hace: emite una carga de memoria; recibe: base, off; genera: valor o indicador calculado.
static Opnd load_mem(Opnd base, long off)
{
    Tac *t;
    int d = nuevo_t();
    t = emit(TAC_LOAD);
    t->d = d;
    t->a = base;
    t->imm = off;
    return V(d);
}

// Hace: emite una escritura en memoria; recibe: base, off, val; genera: efectos sobre estructuras o salida.
static void store_mem(Opnd base, long off, Opnd val)
{
    Tac *t = emit(TAC_STORE);
    t->a = base;
    t->b = val;
    t->imm = off;
}

// Hace: lee el valor de un simbolo; recibe: s; genera: valor o indicador calculado.
static Opnd load_var(Simbolo *s)
{
    if (promovido(s)) {
        return V(vreg_de(s));
    }
    return load_mem(R(base_reg(s)), s->offset);
}

/* d = val (copia o constante) */
// Hace: asigna un operando a un destino; recibe: d, val; genera: efectos sobre estructuras o salida.
static void mover(int d, Opnd val)
{
    Tac *t;
    if (val.k == OP_C) {
        t = emit(TAC_CONST);
        t->d = d;
        t->imm = val.v;
    } else {
        t = emit(TAC_COPY);
        t->d = d;
        t->a = val;
    }
}

// Hace: guarda un valor en un simbolo; recibe: s, val; genera: efectos sobre estructuras o salida.
static void store_var(Simbolo *s, Opnd val)
{
    if (promovido(s)) {
        Tac *ult = F->n ? &F->v[F->n - 1] : NULL;
        /* x = expr: el temporal recien calculado se escribe directo en x */
        if (val.k == OP_V && !F->vsim[val.v] && ult && ult->d == val.v && ult->op != TAC_FROUND) {
            ult->d = vreg_de(s);
            return;
        }
        mover(vreg_de(s), val);
    } else {
        store_mem(R(base_reg(s)), s->offset, val);
    }
}

/* Valor en un virtual (materializa constantes y registros fisicos). */
// Hace: convierte un operando a virtual; recibe: o; genera: valor o indicador calculado.
static Opnd en_v(Opnd o)
{
    if (o.k == OP_V) {
        return o;
    }
    {
        int d = nuevo_t();
        mover(d, o);
        return V(d);
    }
}

/* ---------------- expresiones ---------------- */

static Opnd expr(Nodo *n);
static Opnd llamada(Nodo *n, int valor);

// Hace: comprueba si la operacion es conmutativa; recibe: b; genera: valor o indicador calculado.
static int conmutativa(TacBop b)
{
    return b == B_ADD || b == B_MUL || b == B_AND || b == B_OR || b == B_EQ || b == B_NE;
}

// Hace: evalua una operacion entre constantes; recibe: op, a, b, r; genera: valor o indicador calculado.
static int plegar(TacBop op, long a, long b, long *r)
{
    int32_t x = (int32_t)a, y = (int32_t)b;
    switch (op) {
    case B_ADD: *r = (int32_t)((uint32_t)x + (uint32_t)y); return 1;
    case B_SUB: *r = (int32_t)((uint32_t)x - (uint32_t)y); return 1;
    case B_MUL: *r = (int32_t)((uint32_t)x * (uint32_t)y); return 1;
    case B_DIV: if (y == 0 || (x == INT32_MIN && y == -1)) return 0; *r = x / y; return 1;
    case B_MOD: if (y == 0 || (x == INT32_MIN && y == -1)) return 0; *r = x % y; return 1;
    case B_AND: *r = x & y; return 1;
    case B_OR: *r = x | y; return 1;
    case B_SHL: *r = (int32_t)((uint32_t)x << (y & 31)); return 1;
    case B_LT: *r = x < y; return 1;
    case B_LE: *r = x <= y; return 1;
    case B_GT: *r = x > y; return 1;
    case B_GE: *r = x >= y; return 1;
    case B_EQ: *r = x == y; return 1;
    case B_NE: *r = x != y; return 1;
    default: return 0;
    }
}

// Hace: genera una operacion binaria TAC; recibe: op, a, b; genera: valor o indicador calculado.
static Opnd bin(TacBop op, Opnd a, Opnd b)
{
    Tac *t;
    long r;
    int d;

    if (a.k == OP_C && b.k == OP_C && plegar(op, a.v, b.v, &r)) {
        return C(r);
    }
    if (a.k == OP_C && conmutativa(op)) {
        Opnd x = a;
        a = b;
        b = x;
    }
    /* identidades que no cambian el resultado */
    if (b.k == OP_C && ((b.v == 0 && (op == B_ADD || op == B_SUB || op == B_SHL)) || (b.v == 1 && (op == B_MUL || op == B_DIV)))) {
        return a;
    }
    if (a.k != OP_V && a.k != OP_R) {
        a = en_v(a);
    }
    d = nuevo_t();
    t = emit(TAC_BIN);
    t->bop = op;
    t->d = d;
    t->a = a;
    t->b = b;
    return V(d);
}

// Hace: traduce un operador del AST a TAC; recibe: op; genera: valor o indicador calculado.
static TacBop bop_de(const char *op)
{
    if (!strcmp(op, "+")) return B_ADD;
    if (!strcmp(op, "-")) return B_SUB;
    if (!strcmp(op, "*")) return B_MUL;
    if (!strcmp(op, "/")) return B_DIV;
    if (!strcmp(op, "%")) return B_MOD;
    if (!strcmp(op, "&&")) return B_AND;
    if (!strcmp(op, "||")) return B_OR;
    if (!strcmp(op, "<")) return B_LT;
    if (!strcmp(op, "<=")) return B_LE;
    if (!strcmp(op, ">")) return B_GT;
    if (!strcmp(op, ">=")) return B_GE;
    if (!strcmp(op, "==")) return B_EQ;
    if (!strcmp(op, "!=")) return B_NE;
    return B_N;
}

/* Direccion de una lista: parametro (ya es la direccion) o base + desplazamiento. */
// Hace: obtiene la direccion base de una lista; recibe: s; genera: valor o indicador calculado.
static Opnd addr_lista(Simbolo *s)
{
    if (s->clase == CL_PARAM) {
        return load_var(s);
    }
    return bin(B_ADD, R(base_reg(s)), C(s->offset));
}

/* Elemento de lista: queda en M[*base + *off]. */
// Hace: calcula la referencia a un elemento; recibe: n, base, off; genera: efectos sobre estructuras o salida.
static void elem_ref(Nodo *n, Opnd *base, long *off)
{
    Simbolo *s = n->sim;
    Nodo *i = n->hijo;
    Nodo *j = i ? i->sig : NULL;
    Opnd b0, idx;
    long o0;

    if (s->clase == CL_PARAM) {
        b0 = load_var(s);
        o0 = 8;
    } else {
        b0 = R(base_reg(s));
        o0 = s->offset + 8;
    }
    if (j) {
        Opnd oi = expr(i);
        Opnd oj = expr(j);
        Opnd dim1 = s->clase == CL_PARAM ? load_mem(b0, 4) : C(s->dim[1]);
        idx = bin(B_ADD, bin(B_MUL, oi, dim1), oj);
    } else {
        idx = expr(i);
    }
    if (idx.k == OP_C) {
        *base = b0;
        *off = o0 + 4 * idx.v;
        return;
    }
    *base = bin(B_ADD, b0, bin(B_SHL, idx, C(2)));
    *off = o0;
}

// Hace: traduce una expresion del AST a TAC; recibe: n; genera: valor o indicador calculado.
static Opnd expr(Nodo *n)
{
    long v;
    TacBop op;

    if (!n) {
        return C(0);
    }
    cur_linea = n->linea;
    switch (n->tipo) {
    case NODO_LITERAL:
        return C(n->val);
    case NODO_IDENT:
        if (es_lista_ty(n->sim->ty)) {
            return addr_lista(n->sim);
        }
        return load_var(n->sim);
    case NODO_INDICE: {
        Opnd base;
        long off;
        elem_ref(n, &base, &off);
        return load_mem(base, off);
    }
    case NODO_LLAMADA:
        return llamada(n, 1);
    case NODO_EXP:
        break;
    default:
        error(n, "expresion no generable");
        return C(0);
    }
    if (n->ty == TY_ENT && sem_const(n, &v)) {
        return C((int32_t)v);
    }
    if (!strcmp(n->texto, "!") || !strcmp(n->texto, "neg")) {
        Opnd a = expr(n->hijo);
        Tac *t;
        int d;
        if (a.k == OP_C) {
            return C(!strcmp(n->texto, "!") ? !a.v : (int32_t)(-(uint32_t)a.v));
        }
        d = nuevo_t();
        t = emit(TAC_UN);
        t->bop = !strcmp(n->texto, "!") ? U_NOT : U_NEG;
        t->d = d;
        t->a = a;
        return V(d);
    }
    op = bop_de(n->texto);
    {
        Opnd a = expr(n->hijo);
        Opnd b = expr(n->hijo->sig);
        return bin(op, a, b);
    }
}

/* ---------------- condiciones (corto circuito) ---------------- */

// Hace: invierte una comparacion; recibe: b; genera: valor o indicador calculado.
static TacBop negar(TacBop b)
{
    switch (b) {
    case B_LT: return B_GE;
    case B_GE: return B_LT;
    case B_LE: return B_GT;
    case B_GT: return B_LE;
    case B_EQ: return B_NE;
    case B_NE: return B_EQ;
    default: return b;
    }
}

// Hace: emite un salto; recibe: etq; genera: efectos sobre estructuras o salida.
static void salto(const char *etq)
{
    Tac *t = emit(TAC_JMP);
    t->etq = strdup(etq);
}

// Hace: emite una etiqueta; recibe: etq; genera: efectos sobre estructuras o salida.
static void etiqueta(const char *etq)
{
    Tac *t = emit(TAC_LABEL);
    t->etq = strdup(etq);
}

/* Salta a etq cuando la condicion vale `si`. */
// Hace: genera el salto condicional de una expresion; recibe: n, etq, si; genera: efectos sobre estructuras o salida.
static void cond(Nodo *n, const char *etq, int si)
{
    cur_linea = n->linea;
    if (n->tipo == NODO_LITERAL && n->ty == TY_BOOL) {
        if ((n->val != 0) == (si != 0)) {
            salto(etq);
        }
        return;
    }
    if (n->tipo == NODO_EXP) {
        const char *op = n->texto;
        Nodo *l = n->hijo;
        Nodo *r = l ? l->sig : NULL;
        TacBop b = bop_de(op);
        if (b == B_AND || b == B_OR) {
            int corta = (b == B_AND) ? !si : si;   /* el lado izquierdo decide solo */
            if (corta) {
                cond(l, etq, si);
                cond(r, etq, si);
            } else {
                char *sigue = nueva_etq();
                cond(l, sigue, !si);
                cond(r, etq, si);
                etiqueta(sigue);
                free(sigue);
            }
            return;
        }
        if (!strcmp(op, "!")) {
            cond(l, etq, !si);
            return;
        }
        if (b >= B_LT && b <= B_NE) {
            Opnd x = expr(l);
            Opnd y = expr(r);
            long res;
            Tac *t;
            if (x.k == OP_C && y.k == OP_C && plegar(b, x.v, y.v, &res)) {
                if ((res != 0) == (si != 0)) {
                    salto(etq);
                }
                return;
            }
            t = emit(TAC_BCOND);
            t->bop = si ? b : negar(b);
            t->a = x;
            t->b = y;
            t->etq = strdup(etq);
            return;
        }
    }
    {
        Opnd x = expr(n);
        Tac *t;
        if (x.k == OP_C) {
            if ((x.v != 0) == (si != 0)) {
                salto(etq);
            }
            return;
        }
        t = emit(si ? TAC_JNZ : TAC_JZ);
        t->a = x;
        t->etq = strdup(etq);
    }
}

/* ---------------- llamadas e intrinsecas ---------------- */

// Hace: almacena un valor en un destino asignable; recibe: lv, val; genera: efectos sobre estructuras o salida.
static void store_lvalue(Nodo *lv, Opnd val)
{
    if (lv->tipo == NODO_IDENT) {
        store_var(lv->sim, val);
    } else {
        Opnd base;
        long off;
        elem_ref(lv, &base, &off);
        store_mem(base, off, val);
    }
}

/* Destino de un FROUND: el propio virtual si el lvalue es una variable promovida. */
// Hace: calcula un destino asignable; recibe: lv; genera: valor o indicador calculado.
static int destino_de(Nodo *lv)
{
    if (lv->tipo == NODO_IDENT && promovido(lv->sim)) {
        return vreg_de(lv->sim);
    }
    return nuevo_t();
}

// Hace: genera la operacion de ronda Feistel; recibe: dl, dr, a, b, k, r; genera: efectos sobre estructuras o salida.
static void fround(int dl, int dr, Opnd a, Opnd b, long k, long r)
{
    Tac *t;
    a = en_v(a);
    b = en_v(b);
    t = emit(TAC_FROUND);
    t->d = dl;
    t->d2 = dr;
    t->a = a;
    t->b = b;
    t->imm = k;
    t->imm2 = r;
}

// Hace: conserva un valor antes de modificar el destino; recibe: lv, v; genera: efectos sobre estructuras o salida.
static void guardar_si_hace_falta(Nodo *lv, int v)
{
    if (!(lv->tipo == NODO_IDENT && promovido(lv->sim) && vreg_de(lv->sim) == v)) {
        store_lvalue(lv, V(v));
    }
}

// Hace: traduce una llamada intrinseca; recibe: n; genera: valor o indicador calculado.
static Opnd intrinseca(Nodo *n)
{
    Simbolo *f = n->sim;
    Nodo *a0 = n->hijo;
    Nodo *a1 = a0 ? a0->sig : NULL;
    Nodo *a2 = a1 ? a1->sig : NULL;
    Nodo *a3 = a2 ? a2->sig : NULL;
    long k = 0, r = 0, s = 0;
    Tac *t;

    switch (f->intr) {
    case IN_AUTENTICAR: {
        Opnd x = en_v(expr(a0));
        t = emit(TAC_AUTH);
        t->a = x;
        break;
    }
    case IN_CERRAR_SESION:
        emit(TAC_LOGOUT);
        break;
    case IN_LEER_ESTADO: {
        int d = nuevo_t();
        t = emit(TAC_RDSR);
        t->d = d;
        return V(d);
    }
    case IN_CARGAR_LLAVE: {
        Opnd x = expr(a0);
        int temporal = !(x.k == OP_V && F->vsim[x.v]);
        sem_const(a1, &k);
        sem_const(a2, &s);
        x = en_v(x);
        t = emit(TAC_LOADKEY);
        t->a = x;
        t->imm = k;
        t->imm2 = s;
        if (temporal) {
            t = emit(TAC_CLEAR);
            t->d = (int)x.v;
        }
        break;
    }
    case IN_LEER_MEM:
        return load_mem(en_v(expr(a0)), 0);
    case IN_ESCRIBIR_MEM: {
        Opnd dir = en_v(expr(a0));
        Opnd val = expr(a1);
        store_mem(dir, 0, val);
        break;
    }
    case IN_RONDA_FEISTEL: {
        Opnd x, y;
        int dl, dr;
        sem_const(a2, &k);
        sem_const(a3, &r);
        x = expr(a0);
        y = expr(a1);
        dl = destino_de(a0);
        dr = destino_de(a1);
        fround(dl, dr, x, y, k, r);
        guardar_si_hace_falta(a0, dl);
        guardar_si_hace_falta(a1, dr);
        break;
    }
    case IN_FEISTEL_CIFRAR:
    case IN_FEISTEL_DESCIFRAR: {
        Opnd x, y;
        int t1 = nuevo_t(), t2 = nuevo_t(), t3 = nuevo_t(), t4 = nuevo_t(), t5 = nuevo_t(), t6 = nuevo_t();
        int dl = destino_de(a0), dr = destino_de(a1);
        sem_const(a2, &k);
        x = expr(a0);
        y = expr(a1);
        if (f->intr == IN_FEISTEL_CIFRAR) {
            fround(t1, t2, x, y, k, 0);
            fround(t3, t4, V(t1), V(t2), k, 1);
            fround(t5, t6, V(t3), V(t4), k, 2);
            fround(dl, dr, V(t5), V(t6), k, 3);
        } else {
            /* descifrado: rondas 3..0 con L y R cruzados (docs/isa.md) */
            fround(t1, t2, y, x, k, 3);
            fround(t3, t4, V(t1), V(t2), k, 2);
            fround(t5, t6, V(t3), V(t4), k, 1);
            fround(dr, dl, V(t5), V(t6), k, 0);
        }
        guardar_si_hace_falta(a0, dl);
        guardar_si_hace_falta(a1, dr);
        break;
    }
    default:
        break;
    }
    return NADA();
}

// Hace: traduce una llamada de funcion; recibe: n, valor; genera: valor o indicador calculado.
static Opnd llamada(Nodo *n, int valor)
{
    Simbolo *f = n->sim;
    Opnd args[16];
    Nodo *a;
    int k, na = 0;
    char destino[128];
    Tac *t;

    if (!f) {
        return C(0);
    }
    if (f->clase == CL_INTRINSECA) {
        return intrinseca(n);
    }
    for (a = n->hijo; a && na < 16; a = a->sig) {
        args[na++] = expr(a);
    }
    cur_linea = n->linea;
    for (k = 0; k < na; k++) {
        t = emit(TAC_PARAM);
        t->a = args[k];
        t->imm = k;
    }
    snprintf(destino, sizeof destino, "f_%s", f->nombre);
    t = emit(TAC_CALL);
    t->etq = strdup(destino);
    t->imm = na;
    if (valor) {
        t->d = nuevo_t();
        return V(t->d);
    }
    return NADA();
}

/* ---------------- sentencias ---------------- */

static void stmt(Nodo *n);

// Hace: inicializa los elementos de una lista literal; recibe: lit, base, off; genera: efectos sobre estructuras o salida.
static void literal_lista(Nodo *lit, Opnd base, long off)
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
            literal_lista(h, base, off + 4 * ancho * i);
        } else {
            store_mem(base, off + 4 * i, expr(h));
        }
    }
}

// Hace: genera la declaracion de una lista; recibe: n; genera: efectos sobre estructuras o salida.
static void decl_lista(Nodo *n)
{
    Simbolo *s = n->sim;
    Nodo *val = n->hijo && n->hijo->sig && n->hijo->sig->sig ? n->hijo->sig->sig : NULL;
    long total = s->dim[0] * s->dim[1];
    Opnd base = R(base_reg(s));
    long off = s->offset;
    long i;

    store_mem(base, off, C(s->dim[0]));
    store_mem(base, off + 4, C(s->dim[1]));
    if (val && val->tipo == NODO_LISTA_LIT) {
        literal_lista(val, base, off + 8);
    } else if (total <= 16) {
        for (i = 0; i < total; i++) {
            store_mem(base, off + 8 + 4 * i, C(0));
        }
    } else {
        char *top = nueva_etq();
        int p = nuevo_t(), fin = nuevo_t();
        Tac *t;
        mover(p, bin(B_ADD, base, C(off + 8)));
        mover(fin, bin(B_ADD, base, C(off + 8 + 4 * total)));
        etiqueta(top);
        store_mem(V(p), 0, C(0));
        t = emit(TAC_BIN);
        t->bop = B_ADD;
        t->d = p;
        t->a = V(p);
        t->b = C(4);
        t = emit(TAC_BCOND);
        t->bop = B_LT;
        t->a = V(p);
        t->b = V(fin);
        t->etq = strdup(top);
        free(top);
    }
}

// Hace: genera el TAC de una asignacion; recibe: n; genera: efectos sobre estructuras o salida.
static void asignacion(Nodo *n)
{
    Nodo *dst = n->hijo;
    Nodo *val = dst->sig;
    int comp = strcmp(n->texto, "=") != 0;
    TacBop op = n->texto[0] == '+' ? B_ADD : B_SUB;

    if (dst->tipo == NODO_IDENT) {
        if (comp) {
            Opnd x = load_var(dst->sim);
            Opnd y = expr(val);
            if (promovido(dst->sim)) {
                /* x = x op y directo sobre el virtual de la variable */
                Tac *t;
                if (y.k == OP_C && y.v == 0) {
                    return;
                }
                t = emit(TAC_BIN);
                t->bop = op;
                t->d = vreg_de(dst->sim);
                t->a = x;
                t->b = y;
                return;
            }
            store_var(dst->sim, bin(op, x, y));
        } else {
            store_var(dst->sim, expr(val));
        }
        return;
    }
    {
        Opnd base;
        long off;
        elem_ref(dst, &base, &off);
        if (comp) {
            Opnd x = load_mem(base, off);
            Opnd y = expr(val);
            store_mem(base, off, bin(op, x, y));
        } else {
            store_mem(base, off, expr(val));
        }
    }
}

// Hace: genera el flujo TAC de un bucle; recibe: init, c, act, cuerpo; genera: efectos sobre estructuras o salida.
static void bucle(Nodo *init, Nodo *c, Nodo *act, Nodo *cuerpo)
{
    char *l_cond = nueva_etq();
    char *l_cuerpo = nueva_etq();

    if (init) {
        stmt(init);
    }
    salto(l_cond);
    etiqueta(l_cuerpo);
    stmt(cuerpo);
    if (act) {
        stmt(act);
    }
    etiqueta(l_cond);
    cond(c, l_cuerpo, 1);
    free(l_cond);
    free(l_cuerpo);
}

// Hace: traduce una sentencia del AST; recibe: n; genera: efectos sobre estructuras o salida.
static void stmt(Nodo *n)
{
    Nodo *h;

    if (!n) {
        return;
    }
    cur_linea = n->linea;
    switch (n->tipo) {
    case NODO_BLOQUE:
        for (h = n->hijo; h; h = h->sig) {
            stmt(h);
        }
        break;
    case NODO_DECLARACION:
        if (!n->sim) {
            break;
        }
        store_var(n->sim, n->hijo && n->hijo->sig ? expr(n->hijo->sig) : C(0));
        break;
    case NODO_DECL_LISTA:
        if (n->sim) {
            decl_lista(n);
        }
        break;
    case NODO_ASIGNACION:
        asignacion(n);
        break;
    case NODO_SI: {
        Nodo *c = n->hijo;
        Nodo *entonces = c->sig;
        Nodo *sino = entonces ? entonces->sig : NULL;
        char *l_sino = nueva_etq();
        cond(c, l_sino, 0);
        stmt(entonces);
        if (sino) {
            char *l_fin = nueva_etq();
            salto(l_fin);
            etiqueta(l_sino);
            stmt(sino);
            etiqueta(l_fin);
            free(l_fin);
        } else {
            etiqueta(l_sino);
        }
        free(l_sino);
        break;
    }
    case NODO_MIENTRAS:
        bucle(NULL, n->hijo, NULL, n->hijo->sig);
        break;
    case NODO_PARA: {
        Nodo *init = n->hijo;
        Nodo *c = init->sig;
        Nodo *act = c->sig;
        Nodo *cuerpo = act->sig;
        bucle(init, c, act, cuerpo);
        break;
    }
    case NODO_RETORNO: {
        Tac *t;
        Opnd x = n->hijo ? expr(n->hijo) : NADA();
        t = emit(TAC_RET);
        t->a = x;
        break;
    }
    case NODO_LLAMADA:
        llamada(n, 0);
        break;
    case NODO_TERMINAR:
        emit(TAC_HALT);
        break;
    case NODO_LISTA:
        break;
    default:
        error(n, "sentencia no generable");
        break;
    }
}

/* ---------------- funciones ---------------- */

// Hace: inicializa una funcion TAC; recibe: nombre, s; genera: puntero o NULL segun corresponda.
static TacFunc *nueva_func(const char *nombre, Simbolo *s)
{
    prog->f = xrealloc(prog->f, (size_t)(prog->n + 1) * sizeof(TacFunc));
    F = &prog->f[prog->n++];
    memset(F, 0, sizeof *F);
    F->nombre = strdup(nombre);
    F->sim = s;
    return F;
}

// Hace: traduce una funcion completa; recibe: fn; genera: efectos sobre estructuras o salida.
static void funcion(Nodo *fn)
{
    Simbolo *s = fn->sim;
    Nodo *params = fn->hijo ? fn->hijo->sig : NULL;
    Nodo *cuerpo = params ? params->sig : NULL;
    Nodo *p, *h;
    int i = 0;

    nueva_func(s->nombre, s);
    cur_linea = fn->linea;
    /* los parametros llegan en fp-4*(i+1); cada uno pasa a su virtual */
    for (p = params ? params->hijo : NULL; p; p = p->sig, i++) {
        if (p->sim) {
            int v = vreg_de(p->sim);
            Tac *t = emit(TAC_LOAD);
            F->vparam[v] = i;
            t->d = v;
            t->a = R(R_FP);
            t->imm = p->sim->offset;
        }
    }
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        stmt(h);
    }
    if (F->n == 0 || F->v[F->n - 1].op != TAC_RET) {
        Tac *t = emit(TAC_RET);
        t->a = NADA();
    }
}

// Hace: genera el programa TAC desde el AST; recibe: raiz, p, arch; genera: estado o cantidad calculada.
int tac_generar(Nodo *raiz, TacProg *p, const char *arch)
{
    Nodo *cuerpo = raiz->hijo ? raiz->hijo->sig : NULL;
    Nodo *h;
    int i, hay_main = 0;

    prog = p;
    memset(p, 0, sizeof *p);
    archivo = arch;
    n_etq = 0;
    errores = 0;
    cur_linea = 1;
    for (i = 0; i < sem.n_todos; i++) {
        sem.todos[i]->tac_v = 0;
        if (sem.todos[i]->clase == CL_FUNC && !strcmp(sem.todos[i]->nombre, "main")) {
            hay_main = 1;
        }
    }
    nueva_func("_inicio", NULL);
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        if (h->tipo != NODO_FUNCION) {
            stmt(h);
        }
    }
    if (hay_main) {
        Tac *t = emit(TAC_CALL);
        t->etq = strdup("f_main");
        t->imm = 0;
    }
    emit(TAC_HALT);
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        if (h->tipo == NODO_FUNCION && h->sim) {
            funcion(h);
        }
    }
    return errores;
}

// Hace: libera el programa TAC; recibe: p; genera: efectos sobre estructuras o salida.
void tac_liberar(TacProg *p)
{
    int i, k;
    for (i = 0; i < p->n; i++) {
        TacFunc *fn = &p->f[i];
        for (k = 0; k < fn->n; k++) {
            free(fn->v[k].etq);
        }
        for (k = 0; k < fn->nv; k++) {
            free(fn->vnombre[k]);
        }
        free(fn->v);
        free(fn->vnombre);
        free(fn->vsim);
        free(fn->vparam);
        free(fn->nombre);
    }
    free(p->f);
    memset(p, 0, sizeof *p);
}

/* ---------------- impresion ---------------- */

// Hace: obtiene el texto de un operador binario; recibe: b; genera: puntero o NULL segun corresponda.
const char *tac_bop_texto(TacBop b)
{
    static const char *t[] = {"+", "-", "*", "/", "%", "&&", "||", "<<", "<", "<=", ">", ">=", "==", "!=", "-", "!"};
    return b < B_N ? t[b] : "?";
}

// Hace: formatea el nombre de un virtual; recibe: fn, v, buf, n; genera: efectos sobre estructuras o salida.
void tac_vtexto(const TacFunc *fn, int v, char *buf, size_t n)
{
    int k, rep = 0;
    if (v < 0 || v >= fn->nv || !fn->vnombre[v]) {
        snprintf(buf, n, "t%d", v);
        return;
    }
    for (k = 0; k < fn->nv; k++) {
        if (k != v && fn->vnombre[k] && !strcmp(fn->vnombre[k], fn->vnombre[v])) {
            rep = 1;
        }
    }
    if (rep) {
        snprintf(buf, n, "%s.%d", fn->vnombre[v], v);
    } else {
        snprintf(buf, n, "%s", fn->vnombre[v]);
    }
}

// Hace: formatea un operando; recibe: fn, o, buf, n; genera: efectos sobre estructuras o salida.
static void otexto(const TacFunc *fn, Opnd o, char *buf, size_t n)
{
    switch (o.k) {
    case OP_V: tac_vtexto(fn, (int)o.v, buf, n); break;
    case OP_C: snprintf(buf, n, "%ld", o.v); break;
    case OP_R:
        snprintf(buf, n, "%s", o.v == R_GP ? "gp" : o.v == R_FP ? "fp" : o.v == R_SP ? "sp" : o.v == 0 ? "0" : "r?");
        break;
    default: snprintf(buf, n, "_"); break;
    }
}

// Hace: formatea un acceso a memoria; recibe: fn, base, off, buf, n; genera: efectos sobre estructuras o salida.
static void mtexto(const TacFunc *fn, Opnd base, long off, char *buf, size_t n)
{
    char b[64];
    otexto(fn, base, b, sizeof b);
    if (off == 0) {
        snprintf(buf, n, "M[%s]", b);
    } else {
        snprintf(buf, n, "M[%s %c %ld]", b, off < 0 ? '-' : '+', off < 0 ? -off : off);
    }
}

// Hace: formatea una instruccion TAC; recibe: fn, t, buf, n; genera: efectos sobre estructuras o salida.
void tac_texto(const TacFunc *fn, const Tac *t, char *buf, size_t n)
{
    char d[64], d2[64], a[64], b[64], m[96];

    tac_vtexto(fn, t->d, d, sizeof d);
    tac_vtexto(fn, t->d2, d2, sizeof d2);
    otexto(fn, t->a, a, sizeof a);
    otexto(fn, t->b, b, sizeof b);
    switch (t->op) {
    case TAC_CONST: snprintf(buf, n, "%s = %ld", d, t->imm); break;
    case TAC_COPY: snprintf(buf, n, "%s = %s", d, a); break;
    case TAC_BIN: snprintf(buf, n, "%s = %s %s %s", d, a, tac_bop_texto(t->bop), b); break;
    case TAC_UN: snprintf(buf, n, "%s = %s%s", d, tac_bop_texto(t->bop), a); break;
    case TAC_LOAD: mtexto(fn, t->a, t->imm, m, sizeof m); snprintf(buf, n, "%s = %s", d, m); break;
    case TAC_STORE: mtexto(fn, t->a, t->imm, m, sizeof m); snprintf(buf, n, "%s = %s", m, b); break;
    case TAC_LABEL: snprintf(buf, n, "%s:", t->etq); break;
    case TAC_JMP: snprintf(buf, n, "goto %s", t->etq); break;
    case TAC_BCOND: snprintf(buf, n, "if %s %s %s goto %s", a, tac_bop_texto(t->bop), b, t->etq); break;
    case TAC_JZ: snprintf(buf, n, "ifFalse %s goto %s", a, t->etq); break;
    case TAC_JNZ: snprintf(buf, n, "if %s goto %s", a, t->etq); break;
    case TAC_PARAM: snprintf(buf, n, "param %ld = %s", t->imm, a); break;
    case TAC_CALL:
        if (t->d >= 0) snprintf(buf, n, "%s = call %s, %ld", d, t->etq, t->imm);
        else snprintf(buf, n, "call %s, %ld", t->etq, t->imm);
        break;
    case TAC_RET:
        if (t->a.k != OP_NADA) snprintf(buf, n, "return %s", a);
        else snprintf(buf, n, "return");
        break;
    case TAC_HALT: snprintf(buf, n, "terminar"); break;
    case TAC_AUTH: snprintf(buf, n, "auth %s", a); break;
    case TAC_LOGOUT: snprintf(buf, n, "logout"); break;
    case TAC_RDSR: snprintf(buf, n, "%s = rdsr", d); break;
    case TAC_LOADKEY: snprintf(buf, n, "loadkey[%ld][%ld] = %s", t->imm, t->imm2, a); break;
    case TAC_FROUND: snprintf(buf, n, "(%s, %s) = fround(%s, %s, k%ld, r%ld)", d, d2, a, b, t->imm, t->imm2); break;
    case TAC_CLEAR: snprintf(buf, n, "clear %s", d); break;
    default: snprintf(buf, n, "?"); break;
    }
}

// Hace: imprime una funcion TAC; recibe: fn, f; genera: efectos sobre estructuras o salida.
void tac_imprimir_func(const TacFunc *fn, FILE *f)
{
    char buf[256];
    int i;
    fprintf(f, "funcion %s (%d virtuales)\n", fn->nombre, fn->nv);
    for (i = 0; i < fn->n; i++) {
        tac_texto(fn, &fn->v[i], buf, sizeof buf);
        if (fn->v[i].op == TAC_LABEL) {
            fprintf(f, "%s\n", buf);
        } else {
            fprintf(f, "    %-44s ; linea %d\n", buf, fn->v[i].linea);
        }
    }
}

// Hace: imprime el programa TAC completo; recibe: p, f; genera: efectos sobre estructuras o salida.
void tac_imprimir(const TacProg *p, FILE *f)
{
    int i;
    for (i = 0; i < p->n; i++) {
        tac_imprimir_func(&p->f[i], f);
        fputc('\n', f);
    }
}
