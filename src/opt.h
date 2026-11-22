#ifndef OPT_H
#define OPT_H

#include "ast.h"
#include "codegen.h"
#include "ir_tac.h"

/*
-   AST (--unroll) -> TAC -> CFG/vivacidad -> --dce -> --rename -> --reorder
-   -> asignacion de registros (--spill o todo en memoria) -> IR -> planificador
- Sin ninguna bandera de optimizacion jaf usa el generador de P1 (codegen.c).
 */

typedef struct OptCfg {
    int activa; // usar la tuberia TAC
    int unroll; // 0 = no, K >= 2, -1 = auto
    int dce;
    int rename;
    int reorder;
    int spill; // asignacion lineal a registros (si no, todo en memoria)
    int max_regs; // registros asignables desde r8 (1..16)
    int ver_ir_crudo;  // --ir-raw: TAC antes de las pasadas
    int ver_ir; // --ir / -i: TAC final
    int ver_cfg; // --cfg: bloques, lazos y vivacidad del TAC final
} OptCfg;

typedef struct EstadOpt {
    long instrucciones_antes; // TAC recien generado
    long instrucciones_despues; // TAC al terminar las pasadas
    long eliminadas_dce;
    long lazos_desenrollados;
    long iteraciones_unroll; // copias extra del cuerpo
    long reordenadas; // instrucciones que cambiaron de posicion
    long renombradas; // definiciones con virtual nuevo
    long virtuales_derramados;
    long spills_insertados; // cargas/guardas de derrame y de llamadas
    long ir_instrucciones; // instrucciones de maquina antes del planificador
    double tiempo_ms;
} EstadOpt;

// Resultado de la asignacion de registros de una funcion.
typedef struct Asig {
    int *reg; // registro fisico del virtual o -1 (vive en memoria)
    long *slot; // desplazamiento desde fp del hogar en memoria (0 = sin hogar)
    int *ini, *fin; // intervalo de vida en posiciones 2*i (lectura) y 2*i+1 (escritura)
    long zona; // inicio de los huecos de temporales
    long marco; 
    int nslots;
} Asig;

void opt_unroll(Nodo *raiz, int k, EstadOpt *e);
void opt_dce(TacFunc *fn, EstadOpt *e);
void opt_rename(TacFunc *fn, EstadOpt *e);
void opt_reorder(TacFunc *fn, EstadOpt *e);
void opt_regalloc(TacFunc *fn, const OptCfg *c, Asig *a, EstadOpt *e);
void asig_liberar(Asig *a);
long asig_hogar(TacFunc *fn, Asig *a, int v);

// Baja el TAC (ya asignado) al IR de instrucciones con registros fisicos.
int ir_bajar(TacProg *p, const OptCfg *c, IR *ir, EstadOpt *e);

// Corre todas las pasadas sobre el AST analizado y deja el IR listo para planificar.
int opt_compilar(Nodo *raiz, const char *archivo, const OptCfg *c, IR *ir, EstadOpt *e, TacProg *tac_out);
void opt_imprimir_estad(const EstadOpt *e, FILE *f);
void opt_estad_json(const EstadOpt *e, FILE *f);

#endif