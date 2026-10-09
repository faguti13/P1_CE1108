#ifndef ISA_H
#define ISA_H

/*
 * Definicion de la ISA Feistel4-VLIW v1.1 para el compilador JAF.
 * Contrato completo en vliw_grupo2/docs/isa.md y compiler-integration.md.
 *
 * Este modulo lo comparten el generador (planificador, emisor) y el
 * ensamblador propio (jaf -x): ambos producen la misma estructura
 * Programa y la misma codificacion.
 */

#include <stddef.h>
#include <stdint.h>

typedef enum Op {
    /* slot 0: ALU registro */
    OP_SUM, OP_REST, OP_MUL, OP_DIV, OP_OLY, OP_OLO, OP_LOE, OP_DLI, OP_DLD, OP_COMP,
    /* slot 0: ALU inmediato */
    OP_SUMI, OP_RESTI, OP_MULI, OP_DIVI, OP_DLII, OP_DLDI,
    /* slot 1: memoria */
    OP_CP, OP_AP,
    /* slot 2: control */
    OP_SIG, OP_SNIG, OP_SMI, OP_SMQ, OP_S, OP_SYE, OP_SRG, OP_FIN,
    /* slot 3: cripto y seguridad */
    OP_LOADKEY, OP_FROUND, OP_AUTH, OP_LOGOUT, OP_RDSR,
    OP_NOP,
    OP_CANTIDAD
} Op;

/* Registros con nombre del ABI. */
#define R_ZERO 0
#define R_RET  1
#define R_GP   28
#define R_SP   29
#define R_FP   30
#define R_RA   31

/* Constantes de la ISA y del mapa de memoria. */
#define ISA_BUNDLE_BYTES 16
#define ISA_IMM_MIN (-16384)
#define ISA_IMM_MAX 16383
#define MEM_GP        0x10000L
#define MEM_STACK_TOP 0x100000L
#define MEM_DATA_BASE 0x10000L

typedef struct Ins {
    Op op;
    int a, b, c, d, e, f;   /* operandos en el orden del ensamblador, ver isa.c */
    char *etq;              /* etiqueta destino de los saltos */
    int linea;              /* linea del fuente (para mensajes) */
} Ins;

typedef struct Bundle {
    Ins ins[4];
    int usado[4];
    char **etiquetas;       /* etiquetas que apuntan a este bundle */
    int n_etq;
    long dir;               /* direccion de byte, asignada al resolver */
} Bundle;

typedef struct Programa {
    Bundle *b;
    int n, cap;
    uint8_t *datos;         /* seccion .data (solo la produce jaf -x) */
    long n_datos;
} Programa;

/* Instrucciones. */
const char *op_nombre(Op op);
Op op_buscar(const char *nombre);          /* OP_CANTIDAD si no existe; ignora mayusculas */
int op_slot(Op op);
int op_es_salto(Op op);                    /* salto con etiqueta (SIG..SYE) */
void ins_rw(const Ins *i, int *lee, int *n_lee, int *escribe, int *n_escribe);
void ins_texto(const Ins *i, char *buf, size_t n);
/* Codifica; `delta` es (destino - (dir + 16)) / 16 para los saltos. 0 si va bien. */
int ins_codificar(const Ins *i, long delta, uint32_t *palabra, char *err, size_t n_err);
uint32_t nop_slot(int slot);

/* Programa / bundles. */
void prog_iniciar(Programa *p);
Bundle *prog_nuevo_bundle(Programa *p);
void bundle_etiqueta(Bundle *b, const char *etq);
int prog_resolver(Programa *p, char *err, size_t n_err);   /* direcciones + etiquetas */
long prog_buscar_etiqueta(const Programa *p, const char *etq);   /* direccion o -1 */

/* Codigo maquina: code debe tener 16*n bytes. Devuelve 0 si va bien. */
int prog_codificar(Programa *p, uint8_t *code, char *err, size_t n_err);
int prog_escribir_bin(const Programa *p, const uint8_t *code, const char *ruta);
int prog_escribir_mem(const Programa *p, const uint8_t *code, const char *ruta);

#endif
