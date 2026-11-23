#include "sim_ciclos.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEM_BYTES 0x10000
#define CRED 0xCAFE1234u
#define LAT_ESCRITURA 3
#define CICLOS_VACIADO 5

typedef struct Pend {
    int reg;
    uint32_t val;
    long listo;
} Pend;

typedef struct Maquina {
    uint32_t r[32];
    Pend pend[64];
    int npend;
    uint8_t *mem;
    uint32_t boveda[4][4];
    uint32_t sr;
    long ciclo;
    /* L1: 64 conjuntos x 2 vias; L2: 128 conjuntos x 4 vias (LRU por edad) */
    int l1_tag[64][2], l1_val[64][2], l1_lru[64];
    int l2_tag[128][4], l2_val[128][4];
    long l2_edad[128][4];
    long reloj_l2;
} Maquina;

// Hace: rota a la izquierda una palabra de 32 bits; recibe: x, r; genera: valor o indicador calculado.
static uint32_t rol(uint32_t x, int r) { return (x << r) | (x >> (32 - r)); }

// Hace: calcula la funcion interna Feistel; recibe: r, k; genera: valor o indicador calculado.
static uint32_t feistel_f(uint32_t r, uint32_t k) { return (rol(r, 5) + k) ^ rol(r, 13); }

// Hace: confirma operaciones pendientes de la maquina; recibe: m; genera: efectos sobre estructuras o salida.
static void confirmar(Maquina *m)
{
    int i, j = 0;
    for (i = 0; i < m->npend; i++) {
        if (m->pend[i].listo <= m->ciclo) {
            m->r[m->pend[i].reg] = m->pend[i].val;
        } else {
            m->pend[j++] = m->pend[i];
        }
    }
    m->npend = j;
}

// Hace: lee un registro simulando las dependencias; recibe: m, reg, e; genera: valor o indicador calculado.
static uint32_t leer(Maquina *m, int reg, EstadSim *e)
{
    int i;
    if (reg == 0) {
        return 0;
    }
    for (i = 0; i < m->npend; i++) {
        if (m->pend[i].reg == reg) {
            e->riesgos++;
            if (getenv("SIM_TRAZA")) {
                fprintf(stderr, "riesgo: ciclo %ld lee r%d (listo en %ld)\n", m->ciclo, reg, m->pend[i].listo);
            }
            break;
        }
    }
    return m->r[reg];
}

/* Las escrituras del bundle quedan en espera hasta que todos sus slots leyeron. */
static Pend escrituras_bundle[8];
static int n_escrituras_bundle;

// Hace: programa una escritura de registro; recibe: m, reg, v; genera: efectos sobre estructuras o salida.
static void escribir(Maquina *m, int reg, uint32_t v)
{
    if (reg == 0 || n_escrituras_bundle >= 8) {
        return;
    }
    escrituras_bundle[n_escrituras_bundle].reg = reg;
    escrituras_bundle[n_escrituras_bundle].val = v;
    escrituras_bundle[n_escrituras_bundle].listo = m->ciclo + LAT_ESCRITURA;
    n_escrituras_bundle++;
}

// Hace: publica las escrituras pendientes; recibe: m; genera: efectos sobre estructuras o salida.
static void publicar(Maquina *m)
{
    int i;
    for (i = 0; i < n_escrituras_bundle && m->npend < 64; i++) {
        m->pend[m->npend++] = escrituras_bundle[i];
    }
    n_escrituras_bundle = 0;
}

// Hace: lee una palabra de memoria simulada; recibe: m, a; genera: valor o indicador calculado.
static uint32_t mem_leer(Maquina *m, uint32_t a)
{
    a &= MEM_BYTES - 4;
    return ((uint32_t)m->mem[a] << 24) | ((uint32_t)m->mem[a + 1] << 16) | ((uint32_t)m->mem[a + 2] << 8) | m->mem[a + 3];
}

// Hace: escribe una palabra en memoria simulada; recibe: m, a, v; genera: efectos sobre estructuras o salida.
static void mem_escribir(Maquina *m, uint32_t a, uint32_t v)
{
    a &= MEM_BYTES - 4;
    m->mem[a] = (uint8_t)(v >> 24);
    m->mem[a + 1] = (uint8_t)(v >> 16);
    m->mem[a + 2] = (uint8_t)(v >> 8);
    m->mem[a + 3] = (uint8_t)v;
}

/* Penalizacion estimada del acceso en la jerarquia L1 + L2 (WB/WA, inclusiva). */
// Hace: contabiliza y simula un acceso de cache; recibe: m, a, e; genera: valor o indicador calculado.
static long cache(Maquina *m, uint32_t a, EstadSim *e)
{
    uint32_t la = (a & 0xFFFF) >> 5;
    int s1 = la & 63, t1 = (int)(la >> 6);
    int s2 = la & 127, t2 = (int)(la >> 7);
    int w, v;
    long pen = 0;

    for (w = 0; w < 2; w++) {
        if (m->l1_val[s1][w] && m->l1_tag[s1][w] == t1) {
            m->l1_lru[s1] = !w;
            return 0;
        }
    }
    e->l1_fallos++;
    m->reloj_l2++;
    for (w = 0; w < 4; w++) {
        if (m->l2_val[s2][w] && m->l2_tag[s2][w] == t2) {
            break;
        }
    }
    if (w == 4) {
        e->l2_fallos++;
        pen = SIM_PEN_L2_MISS;
        v = 0;
        for (w = 0; w < 4; w++) {
            if (!m->l2_val[s2][w]) {
                v = w;
                break;
            }
            if (m->l2_edad[s2][w] < m->l2_edad[s2][v]) {
                v = w;
            }
        }
        /* inclusiva: la victima de L2 sale tambien de L1 */
        if (m->l2_val[s2][v]) {
            uint32_t vla = ((uint32_t)m->l2_tag[s2][v] << 7) | (uint32_t)s2;
            int vs = vla & 63, vt = (int)(vla >> 6), k;
            for (k = 0; k < 2; k++) {
                if (m->l1_val[vs][k] && m->l1_tag[vs][k] == vt) {
                    m->l1_val[vs][k] = 0;
                }
            }
        }
        m->l2_val[s2][v] = 1;
        m->l2_tag[s2][v] = t2;
        w = v;
    } else {
        pen = SIM_PEN_L2_HIT;
    }
    m->l2_edad[s2][w] = m->reloj_l2;
    v = m->l1_val[s1][0] ? (m->l1_val[s1][1] ? m->l1_lru[s1] : 1) : 0;
    m->l1_val[s1][v] = 1;
    m->l1_tag[s1][v] = t1;
    m->l1_lru[s1] = !v;
    return pen;
}

// Hace: busca una instruccion por direccion; recibe: p, dir; genera: valor o indicador calculado.
static int indice_de(const Programa *p, long dir)
{
    int lo = 0, hi = p->n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (p->b[mid].dir == dir) {
            return mid;
        }
        if (p->b[mid].dir < dir) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return -1;
}

// Hace: simula el programa y contabiliza ciclos; recibe: p, max_ciclos, e, mem_ext; genera: estado o cantidad calculada.
int sim_ciclos(const Programa *p, long max_ciclos, EstadSim *e, unsigned char *mem_ext)
{
    Maquina *m = calloc(1, sizeof *m);
    int *destino = malloc((size_t)(p->n ? p->n : 1) * sizeof(int));
    int pc = 0, i;
    long extra_cache = 0;

    memset(e, 0, sizeof *e);
    n_escrituras_bundle = 0;
    m->mem = mem_ext ? mem_ext : calloc(MEM_BYTES, 1);
    if (p->datos && p->n_datos > 0) {
        memcpy(m->mem, p->datos, (size_t)(p->n_datos < MEM_BYTES ? p->n_datos : MEM_BYTES));
    }
    for (i = 0; i < p->n; i++) {
        const Ins *s = &p->b[i].ins[2];
        destino[i] = -1;
        if (p->b[i].usado[2] && s->etq) {
            long d = prog_buscar_etiqueta(p, s->etq);
            destino[i] = d >= 0 ? indice_de(p, d) : -1;
        }
    }

    while (pc >= 0 && pc < p->n && m->ciclo < max_ciclos) {
        const Bundle *b = &p->b[pc];
        int salta = 0, siguiente = pc + 1, fin = 0, k;

        confirmar(m);
        for (k = 0; k < 4; k++) {
            e->instrucciones += b->usado[k];
        }
        /* slot 0: ALU */
        if (b->usado[0]) {
            const Ins *s = &b->ins[0];
            uint32_t a = leer(m, s->a, e), y = 0;
            int es_imm = s->op >= OP_SUMI && s->op <= OP_DLDI;
            uint32_t o2 = es_imm ? (uint32_t)s->b : leer(m, s->b, e);
            int32_t sa = (int32_t)a, sb = (int32_t)o2;
            switch (s->op) {
            case OP_SUM: case OP_SUMI: y = a + o2; break;
            case OP_REST: case OP_RESTI: y = a - o2; break;
            case OP_MUL: case OP_MULI: y = a * o2; break;
            case OP_DIV: case OP_DIVI:
                y = o2 == 0 ? 0xFFFFFFFFu : (sa == INT32_MIN && sb == -1) ? (uint32_t)INT32_MIN : (uint32_t)(sa / sb);
                break;
            case OP_OLY: y = a & o2; break;
            case OP_OLO: y = a | o2; break;
            case OP_LOE: y = a ^ o2; break;
            case OP_DLI: case OP_DLII: y = a << (o2 & 31); break;
            case OP_DLD: case OP_DLDI: y = a >> (o2 & 31); break;
            case OP_COMP: y = sa < sb; break;
            default: break;
            }
            escribir(m, s->c, y);
        }
        /* slot 1: LSU */
        if (b->usado[1]) {
            const Ins *s = &b->ins[1];
            uint32_t dir = leer(m, s->a, e) + (uint32_t)s->b;
            if (s->op == OP_CP) {
                escribir(m, s->c, mem_leer(m, dir));
                e->lecturas++;
            } else {
                mem_escribir(m, dir, leer(m, s->c, e));
                e->escrituras++;
            }
            extra_cache += cache(m, dir, e);
        }
        /* slot 3: cripto y seguridad */
        if (b->usado[3]) {
            const Ins *s = &b->ins[3];
            switch (s->op) {
            case OP_AUTH:
                m->sr = leer(m, s->a, e) == CRED ? 1u : (2u << 2) | 2u;
                break;
            case OP_LOGOUT:
                m->sr &= ~1u;
                break;
            case OP_RDSR:
                escribir(m, s->a, m->sr);
                break;
            case OP_LOADKEY:
                if (m->sr & 1u) {
                    m->boveda[s->b & 3][s->c & 3] = leer(m, s->a, e);
                } else {
                    m->sr = (m->sr & ~0x1Cu) | 2u | (1u << 2);
                }
                break;
            case OP_FROUND:
                if (m->sr & 1u) {
                    uint32_t l = leer(m, s->a, e), r = leer(m, s->b, e);
                    escribir(m, s->e, r);
                    escribir(m, s->f, l ^ feistel_f(r, m->boveda[s->c & 3][s->d & 3]));
                } else {
                    m->sr = (m->sr & ~0x1Cu) | 2u | (1u << 2);
                }
                break;
            default:
                break;
            }
        }
        /* slot 2: control (se resuelve en EX; descarta los 2 bundles siguientes si salta) */
        if (b->usado[2]) {
            const Ins *s = &b->ins[2];
            switch (s->op) {
            case OP_SIG: salta = leer(m, s->a, e) == leer(m, s->b, e); break;
            case OP_SNIG: salta = leer(m, s->a, e) != leer(m, s->b, e); break;
            case OP_SMI: salta = (int32_t)leer(m, s->a, e) >= (int32_t)leer(m, s->b, e); break;
            case OP_SMQ: salta = (int32_t)leer(m, s->a, e) < (int32_t)leer(m, s->b, e); break;
            case OP_S: salta = 1; break;
            case OP_SYE:
                salta = 1;
                escribir(m, R_RA, (uint32_t)(b->dir + 16));
                break;
            case OP_SRG: {
                long d = (long)(leer(m, s->a, e) & ~15u);
                salta = 1;
                siguiente = indice_de(p, d);
                break;
            }
            case OP_FIN: fin = 1; break;
            default: break;
            }
            if (salta && s->op != OP_SRG) {
                siguiente = destino[pc];
            }
        }
        publicar(m);
        m->ciclo++;
        e->bundles++;
        if (fin) {
            e->terminado = 1;
            break;
        }
        if (salta) {
            m->ciclo += 2;
            e->saltos_tomados++;
        }
        pc = siguiente;
    }
    /* reset + vaciado del pipeline despues de FIN (calibrado contra tb_top MODE=0) */
    e->ciclos = m->ciclo + CICLOS_VACIADO;
    e->ciclos_cache = e->ciclos + extra_cache;
    e->sr = m->sr;
    if (!mem_ext) {
        free(m->mem);
    }
    free(m);
    free(destino);
    return e->terminado ? 0 : 1;
}

// Hace: presenta las estadisticas de la simulacion; recibe: e, f; genera: efectos sobre estructuras o salida.
void sim_imprimir(const EstadSim *e, FILE *f)
{
    long acc = e->lecturas + e->escrituras;
    fprintf(f, "sim-ciclos: %s\n", e->terminado ? "FIN alcanzado" : "NO termino (limite de ciclos o PC fuera del codigo)");
    fprintf(f, "  memoria ideal : %ld ciclos, %ld bundles, %ld instrucciones, IPC %.3f, %ld saltos tomados\n", e->ciclos,
            e->bundles, e->instrucciones, e->ciclos ? (double)e->instrucciones / (double)e->ciclos : 0.0,
            e->saltos_tomados);
    fprintf(f, "  con L1+L2 (est): %ld ciclos, IPC %.3f, %ld accesos, L1 hit %.2f%%, L2 hit %.2f%%\n", e->ciclos_cache,
            e->ciclos_cache ? (double)e->instrucciones / (double)e->ciclos_cache : 0.0, acc,
            acc ? 100.0 * (double)(acc - e->l1_fallos) / (double)acc : 0.0,
            e->l1_fallos ? 100.0 * (double)(e->l1_fallos - e->l2_fallos) / (double)e->l1_fallos : 0.0);
    fprintf(f, "  riesgos RAW sin resolver: %ld, SR final = 0x%08X\n", e->riesgos, e->sr);
}

// Hace: exporta estadisticas en formato JSON; recibe: e, f; genera: efectos sobre estructuras o salida.
void sim_json(const EstadSim *e, FILE *f)
{
    fprintf(f,
            "{\"terminado\": %d, \"ciclos\": %ld, \"bundles\": %ld, \"instrucciones\": %ld, \"ipc\": %.4f, "
            "\"saltos_tomados\": %ld, \"lecturas\": %ld, \"escrituras\": %ld, \"l1_fallos\": %ld, \"l2_fallos\": %ld, "
            "\"ciclos_cache\": %ld, \"riesgos\": %ld, \"sr\": %u}",
            e->terminado, e->ciclos, e->bundles, e->instrucciones,
            e->ciclos ? (double)e->instrucciones / (double)e->ciclos : 0.0, e->saltos_tomados, e->lecturas,
            e->escrituras, e->l1_fallos, e->l2_fallos, e->ciclos_cache, e->riesgos, e->sr);
}
