/* ISA Feistel4-VLIW v1.1 vista desde el compilador:
 -   tabla de instrucciones (mnemónico y slot de cada una)
 -   registros que lee y escribe cada instrucción
 -   texto en ensamblador y codificación a palabras de 32 bits
 -   fases 5 y 6: direcciones, etiquetas, saltos y escritura del .bin y .mem
   La usan la compilación normal y el ensamblador propio (-x). */
#include "isa.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Tabla de instrucciones:
 *   REG      a=rs1 b=rs2 c=rd
 *   IMM      a=rs1 b=imm c=rd
 *   CP       a=rbase b=offset c=rd        AP  a=rbase b=offset c=rs
 *   SIG..SMQ a=rs1 b=rs2 etq              S, SYE etq        SRG a=rs
 *   LOADKEY  a=rs b=llave c=subllave
 *   FROUND   a=rsL b=rsR c=llave d=ronda e=rdL f=rdR
 *   AUTH a=rs     RDSR a=rd
 */

/* Datos de cada instrucción: mnemónico y slot fijo (-1 para NOP). */
typedef struct Info {
    const char *nombre;
    int slot;
} Info;

/* Tabla indexada por el enum Op, el orden de isa.h debe coincidir. */
static const Info tabla[OP_CANTIDAD] = {
    [OP_SUM] = {"SUM", 0},   [OP_REST] = {"REST", 0}, [OP_MUL] = {"MUL", 0},
    [OP_DIV] = {"DIV", 0},   [OP_OLY] = {"OLY", 0},   [OP_OLO] = {"OLO", 0},
    [OP_LOE] = {"LOE", 0},   [OP_DLI] = {"DLI", 0},   [OP_DLD] = {"DLD", 0},
    [OP_COMP] = {"COMP", 0},
    [OP_SUMI] = {"SUMI", 0}, [OP_RESTI] = {"RESTI", 0}, [OP_MULI] = {"MULI", 0},
    [OP_DIVI] = {"DIVI", 0}, [OP_DLII] = {"DLII", 0}, [OP_DLDI] = {"DLDI", 0},
    [OP_CP] = {"CP", 1},     [OP_AP] = {"AP", 1},
    [OP_SIG] = {"SIG", 2},   [OP_SNIG] = {"SNIG", 2}, [OP_SMI] = {"SMI", 2},
    [OP_SMQ] = {"SMQ", 2},   [OP_S] = {"S", 2},       [OP_SYE] = {"SYE", 2},
    [OP_SRG] = {"SRG", 2},   [OP_FIN] = {"FIN", 2},
    [OP_LOADKEY] = {"LOADKEY", 3}, [OP_FROUND] = {"FROUND", 3},
    [OP_AUTH] = {"AUTH", 3}, [OP_LOGOUT] = {"LOGOUT", 3}, [OP_RDSR] = {"RDSR", 3},
    [OP_NOP] = {"NOP", -1},
};

/* NOP de cada slot:
 -   ALU 0x78
 -   LSU 0x7A
 -   BRU 0x7B
 -   Cripto 0x7C */
static const uint32_t nops[4] = {0x78, 0x7A, 0x7B, 0x7C};

/* Devuelve el mnemónico de la instrucción, o "?" si no es válida. */
const char *op_nombre(Op op)
{
    return (op >= 0 && op < OP_CANTIDAD) ? tabla[op].nombre : "?";
}

/* Busca un mnemónico en la tabla sin importar mayúsculas.
   Devuelve OP_CANTIDAD si no existe. */
Op op_buscar(const char *nombre)
{
    int i;
    for (i = 0; i < OP_CANTIDAD; i++) {
        const char *a = tabla[i].nombre;
        const char *b = nombre;
        while (*a && *b && toupper((unsigned char)*b) == *a) {
            a++;
            b++;
        }
        if (!*a && !*b) {
            return (Op)i;
        }
    }
    return OP_CANTIDAD;
}

/* Devuelve el slot fijo de la instrucción:
 -   0 ALU
 -   1 LSU
 -   2 BRU
 -   3 cripto y seguridad */
int op_slot(Op op)
{
    return tabla[op].slot;
}

/* Indica si la instrucción es un salto con etiqueta (SIG, SNIG, SMI, SMQ, S y SYE).
   SRG y FIN no llevan etiqueta. */
int op_es_salto(Op op)
{
    return op >= OP_SIG && op <= OP_SYE;
}

/* Devuelve la palabra NOP del slot (0 a 3). */
uint32_t nop_slot(int slot)
{
    return nops[slot];
}

/* Indican si la instrucción es de ALU entre registros o con inmediato. */
static int es_reg_alu(Op op) { return op >= OP_SUM && op <= OP_COMP; }
static int es_imm_alu(Op op) { return op >= OP_SUMI && op <= OP_DLDI; }

/* Llena la lista de registros que la instrucción lee y la de los que escribe.
   r0 se quita de las dos listas porque siempre vale cero. La usan:
 -   el planificador, para las dependencias RAW, WAW y WAR entre bundles
 -   el ensamblador, para que dos slots de un bundle no escriban el mismo registro */
void ins_rw(const Ins *i, int *lee, int *n_lee, int *escribe, int *n_escribe)
{
    int nl = 0, ne = 0;

    if (es_reg_alu(i->op)) {
        lee[nl++] = i->a;
        lee[nl++] = i->b;
        escribe[ne++] = i->c;
    } else if (es_imm_alu(i->op)) {
        lee[nl++] = i->a;
        escribe[ne++] = i->c;
    } else {
        switch (i->op) {
        case OP_CP:
            lee[nl++] = i->a;
            escribe[ne++] = i->c;
            break;
        case OP_AP:
            lee[nl++] = i->a;
            lee[nl++] = i->c;
            break;
        case OP_SIG: case OP_SNIG: case OP_SMI: case OP_SMQ:
            lee[nl++] = i->a;
            lee[nl++] = i->b;
            break;
        case OP_SYE:
            escribe[ne++] = R_RA;
            break;
        case OP_SRG:
        case OP_AUTH:
        case OP_LOADKEY:
            lee[nl++] = i->a;
            break;
        case OP_FROUND:
            lee[nl++] = i->a;
            lee[nl++] = i->b;
            escribe[ne++] = i->e;
            escribe[ne++] = i->f;
            break;
        case OP_RDSR:
            escribe[ne++] = i->a;
            break;
        default:
            break;
        }
    }
    /* r0 no es una dependencia real. */
    {
        int k, m = 0;
        for (k = 0; k < nl; k++) {
            if (lee[k] != 0) {
                lee[m++] = lee[k];
            }
        }
        nl = m;
        m = 0;
        for (k = 0; k < ne; k++) {
            if (escribe[k] != 0) {
                escribe[m++] = escribe[k];
            }
        }
        ne = m;
    }
    *n_lee = nl;
    *n_escribe = ne;
}

/* Escribe la instrucción como texto en ensamblador, con el destino al final.
   Es lo que aparece en el .asm (-s), en -p y en los mensajes de error. */
void ins_texto(const Ins *i, char *buf, size_t n)
{
    const char *m = op_nombre(i->op);

    if (es_reg_alu(i->op)) {
        snprintf(buf, n, "%s r%d, r%d, r%d", m, i->a, i->b, i->c);
    } else if (es_imm_alu(i->op)) {
        snprintf(buf, n, "%s r%d, %d, r%d", m, i->a, i->b, i->c);
    } else {
        switch (i->op) {
        case OP_CP:
        case OP_AP:
            snprintf(buf, n, "%s %d(r%d), r%d", m, i->b, i->a, i->c);
            break;
        case OP_SIG: case OP_SNIG: case OP_SMI: case OP_SMQ:
            snprintf(buf, n, "%s r%d, r%d, %s", m, i->a, i->b, i->etq ? i->etq : "?");
            break;
        case OP_S:
        case OP_SYE:
            snprintf(buf, n, "%s %s", m, i->etq ? i->etq : "?");
            break;
        case OP_SRG:
        case OP_AUTH:
        case OP_RDSR:
            snprintf(buf, n, "%s r%d", m, i->a);
            break;
        case OP_LOADKEY:
            snprintf(buf, n, "%s r%d, %d, %d", m, i->a, i->b, i->c);
            break;
        case OP_FROUND:
            snprintf(buf, n, "%s r%d, r%d, %d, %d, r%d, r%d", m, i->a, i->b, i->c, i->d, i->e, i->f);
            break;
        default:
            snprintf(buf, n, "%s", m);
            break;
        }
    }
}

/* Indica si v cabe en un entero con signo de 'bits' bits (15 bits: -16384..16383). */
static int cabe(long v, int bits)
{
    long lo = -(1L << (bits - 1));
    long hi = (1L << (bits - 1)) - 1;
    return v >= lo && v <= hi;
}

/* Deja el mensaje de error en err y devuelve 1. */
#define FALLA(...) do { snprintf(err, n_err, __VA_ARGS__); return 1; } while (0)

/* Indica si r es un registro válido (r0..r31). */
static int reg_ok(int r)
{
    return r >= 0 && r <= 31;
}

/* Arma la palabra de 32 bits de una instrucción según su tipo (opcode en [2:0] y funct en [6:3] en todas):
 -   REG: rs1 [21:17], rs2 [16:12], rd [11:7]
 -   IMM, CP y AP: inmediato partido (bit 14 en [31], bits 13..0 en [25:12]),
     rs1 [30:26], rd [11:7]
 -   SIG..SMQ: desplazamiento de 15 bits en [31:17], rs1 [16:12], rs2 [11:7]
 -   S y SYE: desplazamiento de 25 bits en [31:7]; SRG: rs [16:12]; FIN: 0x3B
 -   LOADKEY, FROUND, AUTH, LOGOUT y RDSR: formatos de los tipos F y S de isa.md
   Revisa registros, rangos de inmediatos, llaves y rondas. delta es el salto
   en bundles, ya calculado por prog_codificar. */
int ins_codificar(const Ins *i, long delta, uint32_t *w, char *err, size_t n_err)
{
    uint32_t im;
    int f;

    if (es_reg_alu(i->op)) {
        if (!reg_ok(i->a) || !reg_ok(i->b) || !reg_ok(i->c)) {
            FALLA("%s: registro fuera de r0..r31", op_nombre(i->op));
        }
        f = i->op - OP_SUM;
        *w = ((uint32_t)i->a << 17) | ((uint32_t)i->b << 12) | ((uint32_t)i->c << 7) | ((uint32_t)f << 3);
        return 0;
    }
    if (es_imm_alu(i->op)) {
        f = i->op - OP_SUMI;
        if (!reg_ok(i->a) || !reg_ok(i->c)) {
            FALLA("%s: registro fuera de r0..r31", op_nombre(i->op));
        }
        if (i->op == OP_DLII || i->op == OP_DLDI) {
            if (i->b < 0 || i->b > 31) {
                FALLA("%s: desplazamiento inmediato fuera de 0..31", op_nombre(i->op));
            }
        } else if (!cabe(i->b, 15)) {
            FALLA("%s: inmediato %d fuera de rango (15 bits con signo)", op_nombre(i->op), i->b);
        }
        im = (uint32_t)i->b & 0x7FFF;
        *w = (((im >> 14) & 1u) << 31) | ((uint32_t)i->a << 26) | ((im & 0x3FFFu) << 12) |
             ((uint32_t)i->c << 7) | ((uint32_t)f << 3) | 1u;
        return 0;
    }
    switch (i->op) {
    case OP_CP:
    case OP_AP:
        if (!reg_ok(i->a) || !reg_ok(i->c)) {
            FALLA("%s: registro fuera de r0..r31", op_nombre(i->op));
        }
        if (!cabe(i->b, 15)) {
            FALLA("%s: offset %d fuera de rango (15 bits con signo)", op_nombre(i->op), i->b);
        }
        im = (uint32_t)i->b & 0x7FFF;
        *w = (((im >> 14) & 1u) << 31) | ((uint32_t)i->a << 26) | ((im & 0x3FFFu) << 12) |
             ((uint32_t)i->c << 7) | ((uint32_t)(i->op == OP_AP) << 3) | 2u;
        return 0;
    case OP_SIG: case OP_SNIG: case OP_SMI: case OP_SMQ:
        if (!reg_ok(i->a) || !reg_ok(i->b)) {
            FALLA("%s: registro fuera de r0..r31", op_nombre(i->op));
        }
        if (!cabe(delta, 15)) {
            FALLA("%s: salto de %ld bundles fuera del rango de 15 bits", op_nombre(i->op), delta);
        }
        *w = (((uint32_t)delta & 0x7FFFu) << 17) | ((uint32_t)i->a << 12) | ((uint32_t)i->b << 7) |
             ((uint32_t)(i->op - OP_SIG) << 3) | 3u;
        return 0;
    case OP_S:
    case OP_SYE:
        if (!cabe(delta, 25)) {
            FALLA("%s: salto de %ld bundles fuera del rango de 25 bits", op_nombre(i->op), delta);
        }
        *w = (((uint32_t)delta & 0x1FFFFFFu) << 7) | ((uint32_t)(i->op == OP_S ? 4 : 5) << 3) | 3u;
        return 0;
    case OP_SRG:
        if (!reg_ok(i->a)) {
            FALLA("SRG: registro fuera de r0..r31");
        }
        *w = ((uint32_t)i->a << 12) | (6u << 3) | 3u;
        return 0;
    case OP_FIN:
        *w = (7u << 3) | 3u;
        return 0;
    case OP_LOADKEY:
        if (i->b < 0 || i->b > 3 || i->c < 0 || i->c > 3) {
            FALLA("LOADKEY: llave y subllave deben estar en 0..3");
        }
        if (!reg_ok(i->a)) {
            FALLA("LOADKEY: registro fuera de r0..r31");
        }
        *w = ((uint32_t)i->c << 19) | ((uint32_t)i->b << 17) | ((uint32_t)i->a << 7) | 4u;
        return 0;
    case OP_FROUND:
        if (i->c < 0 || i->c > 3 || i->d < 0 || i->d > 3) {
            FALLA("FROUND: llave y ronda deben estar en 0..3");
        }
        if (!reg_ok(i->a) || !reg_ok(i->b) || !reg_ok(i->e) || !reg_ok(i->f)) {
            FALLA("FROUND: registro fuera de r0..r31");
        }
        if (i->e == i->f) {
            FALLA("FROUND: rd_L y rd_R deben ser distintos");
        }
        *w = ((uint32_t)i->e << 26) | ((uint32_t)i->f << 21) | ((uint32_t)i->d << 19) |
             ((uint32_t)i->c << 17) | ((uint32_t)i->a << 12) | ((uint32_t)i->b << 7) | (1u << 3) | 4u;
        return 0;
    case OP_AUTH:
        *w = ((uint32_t)i->a << 7) | 5u;
        return 0;
    case OP_LOGOUT:
        *w = (1u << 3) | 5u;
        return 0;
    case OP_RDSR:
        *w = ((uint32_t)i->a << 7) | (2u << 3) | 5u;
        return 0;
    default:
        FALLA("instruccion no codificable");
    }
}

/* Deja el programa vacío, sin bundles ni datos. */
void prog_iniciar(Programa *p)
{
    memset(p, 0, sizeof *p);
}

/* Agrega un bundle vacío al final y le asigna su dirección.
   El arreglo crece al doble cuando se llena. */
Bundle *prog_nuevo_bundle(Programa *p)
{
    if (p->n == p->cap) {
        p->cap = p->cap ? p->cap * 2 : 256;
        p->b = realloc(p->b, (size_t)p->cap * sizeof(Bundle));
        if (!p->b) {
            fprintf(stderr, "sin memoria\n");
            exit(1);
        }
    }
    memset(&p->b[p->n], 0, sizeof(Bundle));
    p->b[p->n].dir = (long)p->n * ISA_BUNDLE_BYTES;
    return &p->b[p->n++];
}

/* Agrega una etiqueta a un bundle (un bundle puede tener varias). */
void bundle_etiqueta(Bundle *b, const char *etq)
{
    b->etiquetas = realloc(b->etiquetas, (size_t)(b->n_etq + 1) * sizeof(char *));
    b->etiquetas[b->n_etq++] = strdup(etq);
}

/* Devuelve la dirección del bundle que tiene la etiqueta, o -1 si no existe. */
long prog_buscar_etiqueta(const Programa *p, const char *etq)
{
    int i, k;
    for (i = 0; i < p->n; i++) {
        for (k = 0; k < p->b[i].n_etq; k++) {
            if (strcmp(p->b[i].etiquetas[k], etq) == 0) {
                return p->b[i].dir;
            }
        }
    }
    return -1;
}

/* Fase 5: recalcula la dirección de cada bundle y revisa que ninguna etiqueta este repetida. */
int prog_resolver(Programa *p, char *err, size_t n_err)
{
    int i, k, j;
    for (i = 0; i < p->n; i++) {
        p->b[i].dir = (long)i * ISA_BUNDLE_BYTES;
        for (k = 0; k < p->b[i].n_etq; k++) {
            for (j = 0; j < i; j++) {
                int m;
                for (m = 0; m < p->b[j].n_etq; m++) {
                    if (strcmp(p->b[j].etiquetas[m], p->b[i].etiquetas[k]) == 0) {
                        FALLA("etiqueta repetida '%s'", p->b[i].etiquetas[k]);
                    }
                }
            }
        }
    }
    return 0;
}

/* Fases 5 y 6: recorre cada slot de cada bundle y:
 -   si está vacío, escribe el NOP de su slot
 -   si es un salto, calcula el desplazamiento: (destino - (dirección + 16)) / 16
 -   codifica la instrucción con ins_codificar
   Cada palabra se guarda en big-endian en code[bundle*16 + slot*4]. */
int prog_codificar(Programa *p, uint8_t *code, char *err, size_t n_err)
{
    int i, s;
    for (i = 0; i < p->n; i++) {
        for (s = 0; s < 4; s++) {
            uint32_t w;
            long delta = 0;
            if (!p->b[i].usado[s]) {
                w = nop_slot(s);
            } else {
                const Ins *ins = &p->b[i].ins[s];
                char msg[160];
                if (op_es_salto(ins->op)) {
                    long dest = ins->etq ? prog_buscar_etiqueta(p, ins->etq) : -1;
                    if (dest < 0) {
                        FALLA("linea %d: etiqueta no definida '%s'", ins->linea, ins->etq ? ins->etq : "?");
                    }
                    delta = (dest - (p->b[i].dir + 16)) / 16;
                }
                if (ins_codificar(ins, delta, &w, msg, sizeof msg)) {
                    FALLA("linea %d: %s", ins->linea, msg);
                }
            }
            code[i * 16 + s * 4 + 0] = (uint8_t)(w >> 24);
            code[i * 16 + s * 4 + 1] = (uint8_t)(w >> 16);
            code[i * 16 + s * 4 + 2] = (uint8_t)(w >> 8);
            code[i * 16 + s * 4 + 3] = (uint8_t)w;
        }
    }
    return 0;
}

/* Escribe un entero de 32 bits en big-endian. */
static void escribir_u32(FILE *f, uint32_t v)
{
    fputc((int)(v >> 24), f);
    fputc((int)(v >> 16), f);
    fputc((int)(v >> 8), f);
    fputc((int)v, f);
}

/* Escribe el archivo .bin:
 -   encabezado de 32 bytes ("VLW1", versión, entrada, base y tamaño del código,
     base y tamaño de los datos, tope de la pila)
 -   el código
 -   los datos (solo con -x) */
int prog_escribir_bin(const Programa *p, const uint8_t *code, const char *ruta)
{
    FILE *f = fopen(ruta, "wb");
    if (!f) {
        return 1;
    }
    fwrite("VLW1", 1, 4, f);
    escribir_u32(f, 1);                              /* versión */
    escribir_u32(f, 0);                              /* entrada */
    escribir_u32(f, 0);                              /* base del código */
    escribir_u32(f, (uint32_t)p->n * 16);            /* tamaño del código */
    escribir_u32(f, MEM_DATA_BASE);                  /* base de datos */
    escribir_u32(f, (uint32_t)p->n_datos);           /* tamaño de datos */
    escribir_u32(f, MEM_STACK_TOP);                  /* tope de pila */
    fwrite(code, 1, (size_t)p->n * 16, f);
    if (p->n_datos) {
        fwrite(p->datos, 1, (size_t)p->n_datos, f);
    }
    return fclose(f) ? 1 : 0;
}

/* Opción --mem: escribe la imagen para $readmemh del simulador, un byte en hexadecimal por línea. */
int prog_escribir_mem(const Programa *p, const uint8_t *code, const char *ruta)
{
    FILE *f = fopen(ruta, "w");
    long i;
    if (!f) {
        return 1;
    }
    fprintf(f, "@00000000\n");
    for (i = 0; i < (long)p->n * 16; i++) {
        fprintf(f, "%02x\n", code[i]);
    }
    if (p->n_datos) {
        fprintf(f, "@%08lx\n", (long)MEM_DATA_BASE);
        for (i = 0; i < p->n_datos; i++) {
            fprintf(f, "%02x\n", p->datos[i]);
        }
    }
    return fclose(f) ? 1 : 0;
}
