#include "opt.h"
#include "isa.h"

#include <stdlib.h>
#include <string.h>

/*
 * Bajada del TAC asignado al IR de maquina (el mismo que produce codegen.c y
 * consume el planificador). Mismas convenciones de ABI que P1:
 *   prologo/epilogo, argumentos en -4*(k+1)(sp), retorno en r1, gp/fp/sp.
 * Registros auxiliares de la bajada:
 *   r24, r25  fuentes que viven en memoria o constantes
 *   r26       destino derramado (y primer destino de FROUND)
 *   r27       segundo destino de FROUND, temporal interno (%, ==, !=) y
 *             desplazamientos grandes
 */

#define R_SRC_A 24
#define R_SRC_B 25
#define R_AUX 26
#define R_CONST 27

static IR *ir;
static TacFunc *F;
static Asig *A;
static EstadOpt *E;
static int cur_linea;

// Hace: reserva y prepara una entrada del IR; recibe: el estado global ir; genera: un puntero al nuevo Item.
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

// Hace: agrega una instruccion de maquina al IR; recibe: operacion op y operandos a, b, c; genera: un Item y actualiza el contador.
static void emit(Op op, int a, int b, int c)
{
    Item *it = nuevo_item();
    it->ins.op = op;
    it->ins.a = a;
    it->ins.b = b;
    it->ins.c = c;
    it->ins.linea = cur_linea;
    E->ir_instrucciones++;
}

// Hace: agrega una instruccion de salto con etiqueta; recibe: op, operandos a y b y etiqueta etq; genera: un Item de salto.
static void emit_salto(Op op, int a, int b, const char *etq)
{
    Item *it = nuevo_item();
    it->ins.op = op;
    it->ins.a = a;
    it->ins.b = b;
    it->ins.etq = strdup(etq);
    it->ins.linea = cur_linea;
    E->ir_instrucciones++;
}

// Hace: incorpora una etiqueta al IR; recibe: su nombre; genera: un Item marcado como etiqueta.
static void etiqueta(const char *nombre)
{
    Item *it = nuevo_item();
    it->es_etq = 1;
    it->etq = strdup(nombre);
}

// Hace: comprueba si un entero cabe en el inmediato de la ISA; recibe: valor v; genera: 1 si cabe o 0 si no.
static int cabe15(long v)
{
    return v >= ISA_IMM_MIN && v <= ISA_IMM_MAX;
}

// Hace: construye una constante en un registro; recibe: registro destino rd y valor v; genera: instrucciones de carga aritmetica.
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

// Hace: lee memoria usando base y desplazamiento; recibe: rd, base y off; genera: instrucciones de carga y calculo de direccion si corresponde.
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

/* Guarda rs en base+off; `tmp` arma la direccion si el desplazamiento no cabe. */
// Hace: escribe un registro en memoria; recibe: rs, base, off y registro auxiliar tmp; genera: instrucciones de almacenamiento.
static void mem_st(int rs, int base, long off, int tmp)
{
    if (cabe15(off)) {
        emit(OP_AP, base, (int)off, rs);
    } else {
        const_load(tmp, off);
        emit(OP_SUM, base, tmp, tmp);
        emit(OP_AP, tmp, 0, rs);
    }
}

// Hace: copia el valor de un registro a otro si son distintos; recibe: rd y rs; genera: una instruccion de movimiento cuando es necesaria.
static void mover(int rd, int rs)
{
    if (rd != rs) {
        emit(OP_SUM, rs, 0, rd);
    }
}

/* Registro con el valor del operando (carga a `scratch` si hace falta). */
// Hace: obtiene un registro con el valor del operando; recibe: operando o y registro auxiliar scratch; genera: registro fuente y cargas si se necesitan.
static int fuente(Opnd o, int scratch)
{
    switch (o.k) {
    case OP_R:
        return (int)o.v;
    case OP_C:
        if (o.v == 0) {
            return 0;
        }
        const_load(scratch, o.v);
        return scratch;
    case OP_V:
        if (A->reg[o.v] >= 0) {
            return A->reg[o.v];
        }
        mem_ld(scratch, R_FP, asig_hogar(F, A, (int)o.v));
        E->spills_insertados++;
        return scratch;
    default:
        return 0;
    }
}

// Hace: selecciona el registro asignado o uno auxiliar; recibe: variable v y registro aux; genera: indice del registro de destino.
static int destino(int v, int aux)
{
    return A->reg[v] >= 0 ? A->reg[v] : aux;
}

// Hace: almacena el resultado de una variable derramada; recibe: variable v y registro r; genera: escritura a memoria si no tiene registro asignado.
static void guardar(int v, int r)
{
    if (A->reg[v] < 0) {
        mem_st(r, R_FP, asig_hogar(F, A, v), R_SRC_A);
        E->spills_insertados++;
    }
}

// Hace: invierte un valor booleano de cero o uno; recibe: registro r; genera: instrucciones que reemplazan su valor por el complementario.
static void flip(int r)
{
    emit(OP_RESTI, r, 1, r);
    emit(OP_MULI, r, -1, r);
}

// Hace: traduce una operacion binaria TAC a instrucciones de maquina; recibe: instruccion t; genera: instrucciones IR y guarda el resultado.
static void bajar_bin(const Tac *t)
{
    int d = destino(t->d, R_AUX);
    int a = fuente(t->a, R_SRC_A);
    int b;
    long c = t->b.v;
    int cte = t->b.k == OP_C && cabe15(c);

    switch (t->bop) {
    case B_ADD: case B_SUB: case B_MUL: case B_DIV:
        if (cte) {
            Op o = t->bop == B_ADD ? OP_SUMI : t->bop == B_SUB ? OP_RESTI : t->bop == B_MUL ? OP_MULI : OP_DIVI;
            emit(o, a, (int)c, d);
        } else {
            Op o = t->bop == B_ADD ? OP_SUM : t->bop == B_SUB ? OP_REST : t->bop == B_MUL ? OP_MUL : OP_DIV;
            b = fuente(t->b, R_SRC_B);
            emit(o, a, b, d);
        }
        break;
    case B_MOD:
        b = fuente(t->b, R_SRC_B);
        emit(OP_DIV, a, b, R_CONST);
        emit(OP_MUL, R_CONST, b, R_CONST);
        emit(OP_REST, a, R_CONST, d);
        break;
    case B_AND:
        b = fuente(t->b, R_SRC_B);
        emit(OP_OLY, a, b, d);
        break;
    case B_OR:
        b = fuente(t->b, R_SRC_B);
        emit(OP_OLO, a, b, d);
        break;
    case B_SHL:
        if (cte) {
            emit(OP_DLII, a, (int)c, d);
        } else {
            b = fuente(t->b, R_SRC_B);
            emit(OP_DLI, a, b, d);
        }
        break;
    case B_LT:
        b = fuente(t->b, R_SRC_B);
        emit(OP_COMP, a, b, d);
        break;
    case B_GT:
        b = fuente(t->b, R_SRC_B);
        emit(OP_COMP, b, a, d);
        break;
    case B_LE:
        b = fuente(t->b, R_SRC_B);
        emit(OP_COMP, b, a, d);
        flip(d);
        break;
    case B_GE:
        b = fuente(t->b, R_SRC_B);
        emit(OP_COMP, a, b, d);
        flip(d);
        break;
    case B_EQ: case B_NE:
        b = fuente(t->b, R_SRC_B);
        emit(OP_REST, a, b, R_CONST);
        emit(OP_COMP, 0, R_CONST, R_SRC_B);    /* 0 < x */
        emit(OP_COMP, R_CONST, 0, d);          /* x < 0 */
        emit(OP_OLO, d, R_SRC_B, d);           /* x != 0 */
        if (t->bop == B_EQ) {
            flip(d);
        }
        break;
    default:
        break;
    }
    guardar(t->d, d);
}

// Hace: traduce una bifurcacion condicional TAC; recibe: instruccion t; genera: instruccion de salto segun la comparacion.
static void bajar_bcond(const Tac *t)
{
    int a = fuente(t->a, R_SRC_A);
    int b = fuente(t->b, R_SRC_B);
    switch (t->bop) {
    case B_EQ: emit_salto(OP_SIG, a, b, t->etq); break;
    case B_NE: emit_salto(OP_SNIG, a, b, t->etq); break;
    case B_LT: emit_salto(OP_SMQ, a, b, t->etq); break;
    case B_GE: emit_salto(OP_SMI, a, b, t->etq); break;
    case B_GT: emit_salto(OP_SMQ, b, a, t->etq); break;
    case B_LE: emit_salto(OP_SMI, b, a, t->etq); break;
    default: break;
    }
}

// Hace: traduce una llamada conservando valores vivos en registros; recibe: instruccion t e indice k; genera: guardados, llamada y restauraciones.
static void bajar_call(const Tac *t, int k)
{
    int v;
    for (v = 0; v < F->nv; v++) {
        if (A->reg[v] >= 0 && A->ini[v] < 2 * k && A->fin[v] > 2 * k + 1 && v != t->d) {
            mem_st(A->reg[v], R_FP, A->slot[v], R_SRC_A);
            E->spills_insertados++;
        }
    }
    emit_salto(OP_SYE, 0, 0, t->etq);
    if (t->d >= 0) {
        int d = destino(t->d, R_AUX);
        mover(d, R_RET);
        guardar(t->d, d);
    }
    for (v = 0; v < F->nv; v++) {
        if (A->reg[v] >= 0 && A->ini[v] < 2 * k && A->fin[v] > 2 * k + 1 && v != t->d) {
            mem_ld(A->reg[v], R_FP, A->slot[v]);
            E->spills_insertados++;
        }
    }
}

// Hace: traduce una instruccion TAC segun su tipo; recibe: t, posicion k, etiqueta epilogo y bandera ultimo; genera: instrucciones o etiquetas IR.
static void bajar(const Tac *t, int k, const char *epilogo, int ultimo)
{
    int d, a, b;

    cur_linea = t->linea;
    switch (t->op) {
    case TAC_CONST:
        d = destino(t->d, R_AUX);
        const_load(d, t->imm);
        guardar(t->d, d);
        break;
    case TAC_COPY:
        if (t->a.k == OP_V && t->a.v == t->d) {
            break;
        }
        if (A->reg[t->d] < 0 && t->a.k == OP_R) {
            guardar(t->d, (int)t->a.v);
            break;
        }
        a = fuente(t->a, R_SRC_A);
        if (A->reg[t->d] < 0) {
            guardar(t->d, a);
        } else {
            mover(A->reg[t->d], a);
        }
        break;
    case TAC_BIN:
        bajar_bin(t);
        break;
    case TAC_UN:
        d = destino(t->d, R_AUX);
        a = fuente(t->a, R_SRC_A);
        if (t->bop == U_NEG) {
            emit(OP_REST, 0, a, d);
        } else {
            emit(OP_RESTI, a, 1, d);
            emit(OP_MULI, d, -1, d);
        }
        guardar(t->d, d);
        break;
    case TAC_LOAD:
        /* parametro en memoria: su hogar es la misma celda en la que llego */
        if (A->reg[t->d] < 0 && t->a.k == OP_R && t->a.v == R_FP && asig_hogar(F, A, t->d) == t->imm) {
            break;
        }
        a = fuente(t->a, R_SRC_A);
        d = destino(t->d, R_AUX);
        mem_ld(d, a, t->imm);
        guardar(t->d, d);
        break;
    case TAC_STORE:
        a = fuente(t->a, R_SRC_A);
        b = fuente(t->b, R_SRC_B);
        mem_st(b, a, t->imm, R_CONST);
        break;
    case TAC_LABEL:
        etiqueta(t->etq);
        break;
    case TAC_JMP:
        emit_salto(OP_S, 0, 0, t->etq);
        break;
    case TAC_BCOND:
        bajar_bcond(t);
        break;
    case TAC_JZ:
        emit_salto(OP_SIG, fuente(t->a, R_SRC_A), 0, t->etq);
        break;
    case TAC_JNZ:
        emit_salto(OP_SNIG, fuente(t->a, R_SRC_A), 0, t->etq);
        break;
    case TAC_PARAM:
        a = fuente(t->a, R_SRC_A);
        mem_st(a, R_SP, -4L * (t->imm + 1), R_CONST);
        break;
    case TAC_CALL:
        bajar_call(t, k);
        break;
    case TAC_RET:
        if (!epilogo) {
            emit(OP_FIN, 0, 0, 0);
            break;
        }
        if (t->a.k != OP_NADA) {
            mover(R_RET, fuente(t->a, R_SRC_A));
        }
        if (!ultimo) {
            emit_salto(OP_S, 0, 0, epilogo);
        }
        break;
    case TAC_HALT:
        emit(OP_FIN, 0, 0, 0);
        break;
    case TAC_AUTH:
        emit(OP_AUTH, fuente(t->a, R_SRC_A), 0, 0);
        break;
    case TAC_LOGOUT:
        emit(OP_LOGOUT, 0, 0, 0);
        break;
    case TAC_RDSR:
        d = destino(t->d, R_AUX);
        emit(OP_RDSR, d, 0, 0);
        guardar(t->d, d);
        break;
    case TAC_LOADKEY:
        emit(OP_LOADKEY, fuente(t->a, R_SRC_A), (int)t->imm, (int)t->imm2);
        break;
    case TAC_CLEAR:
        if (A->reg[t->d] >= 0) {
            emit(OP_SUM, 0, 0, A->reg[t->d]);
        } else {
            guardar(t->d, 0);
        }
        break;
    case TAC_FROUND: {
        Item *it;
        int dl = destino(t->d, R_AUX), dr = destino(t->d2, R_CONST);
        a = fuente(t->a, R_SRC_A);
        b = fuente(t->b, R_SRC_B);
        it = nuevo_item();
        it->ins.op = OP_FROUND;
        it->ins.a = a;
        it->ins.b = b;
        it->ins.c = (int)t->imm;
        it->ins.d = (int)t->imm2;
        it->ins.e = dl;
        it->ins.f = dr;
        it->ins.linea = cur_linea;
        E->ir_instrucciones++;
        guardar(t->d, dl);
        guardar(t->d2, dr);
        break;
    }
    default:
        break;
    }
}

// Hace: traduce una funcion TAC e implementa su prologo y epilogo; recibe: funcion fn; genera: secuencia IR completa de esa funcion.
static void bajar_func(TacFunc *fn)
{
    char nombre[128], epilogo[160];
    long np = fn->sim ? fn->sim->nparams : 0;
    int k;

    F = fn;
    cur_linea = fn->n ? fn->v[0].linea : 1;
    if (!fn->sim) {
        etiqueta("_inicio");
        const_load(R_GP, MEM_GP);
        const_load(R_SP, MEM_STACK_TOP);
        emit(OP_SUM, R_SP, 0, R_FP);
        if (A->marco) {
            if (cabe15(A->marco)) {
                emit(OP_SUMI, R_SP, (int)-A->marco, R_SP);
            } else {
                const_load(R_CONST, A->marco);
                emit(OP_REST, R_SP, R_CONST, R_SP);
            }
        }
        for (k = 0; k < fn->n; k++) {
            bajar(&fn->v[k], k, NULL, 0);
        }
        return;
    }
    snprintf(nombre, sizeof nombre, "f_%s", fn->nombre);
    snprintf(epilogo, sizeof epilogo, "f_%s_fin", fn->nombre);
    etiqueta(nombre);
    mem_st(R_FP, R_SP, -(4 * np + 8), R_CONST);
    mem_st(R_RA, R_SP, -(4 * np + 4), R_CONST);
    emit(OP_SUM, R_SP, 0, R_FP);
    if (cabe15(A->marco)) {
        emit(OP_SUMI, R_SP, (int)-A->marco, R_SP);
    } else {
        const_load(R_CONST, A->marco);
        emit(OP_REST, R_SP, R_CONST, R_SP);
    }
    for (k = 0; k < fn->n; k++) {
        bajar(&fn->v[k], k, epilogo, k == fn->n - 1);
    }
    etiqueta(epilogo);
    mem_ld(R_RA, R_FP, -(4 * np + 4));
    mem_ld(R_CONST, R_FP, -(4 * np + 8));
    emit(OP_SUM, R_FP, 0, R_SP);
    emit(OP_SUM, R_CONST, 0, R_FP);
    emit(OP_SRG, R_RA, 0, 0);
}

// Hace: convierte el programa TAC asignando registros por funcion; recibe: programa p, configuracion c, IR out y estadisticas e; genera: IR de maquina y devuelve 0.
int ir_bajar(TacProg *p, const OptCfg *c, IR *out, EstadOpt *e)
{
    int i;
    ir = out;
    E = e;
    for (i = 0; i < p->n; i++) {
        Asig a;
        opt_regalloc(&p->f[i], c, &a, e);
        A = &a;
        bajar_func(&p->f[i]);
        asig_liberar(&a);
    }
    return 0;
}
