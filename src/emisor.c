/* Salidas legibles del programa ya planificado:
 -   emitir_asm: archivo .asm de la opción -s
 -   emitir_paquetes: vista de la opción -p (dirección, hexadecimal y ensamblador) */
#include "emisor.h"

#include <string.h>

/* Opción -s: escribe el programa en ensamblador con:
 -   dos líneas de encabezado que empiezan con ';'
 -   las etiquetas de cada bundle ("nombre:")
 -   una línea por bundle con las instrucciones de sus slots separadas por " | "
   Los slots vacíos no se escriben y un bundle vacío se escribe como "NOP".
   El .asm se puede volver a ensamblar con jaf -x o con vliwasm.py. */
void emitir_asm(const Programa *p, FILE *f, const char *origen)
{
    int i, s, k;

    fprintf(f, "; generado por jaf desde %s\n", origen);
    fprintf(f, "; un bundle por linea, slots 0..3 separados por '|'\n");
    for (i = 0; i < p->n; i++) {
        const Bundle *b = &p->b[i];
        int primero = 1;
        char buf[160];

        for (k = 0; k < b->n_etq; k++) {
            fprintf(f, "%s:\n", b->etiquetas[k]);
        }
        fputs("    ", f);
        for (s = 0; s < 4; s++) {
            if (!b->usado[s]) {
                continue;
            }
            ins_texto(&b->ins[s], buf, sizeof buf);
            fprintf(f, "%s%s", primero ? "" : " | ", buf);
            primero = 0;
        }
        if (primero) {
            fputs("NOP", f);
        }
        fputc('\n', f);
    }
}

/* Opción -p: imprime por cada bundle:
 -   sus etiquetas
 -   la dirección en hexadecimal
 -   las 4 palabras de 32 bits ya codificadas (slots 0 a 3)
 -   después de ';', las instrucciones en ensamblador */
void emitir_paquetes(const Programa *p, const uint8_t *code, FILE *f)
{
    int i, s, k;

    for (i = 0; i < p->n; i++) {
        const Bundle *b = &p->b[i];
        char buf[160];
        int primero = 1;

        for (k = 0; k < b->n_etq; k++) {
            fprintf(f, "%s:\n", b->etiquetas[k]);
        }
        fprintf(f, "  %08lx  ", b->dir);
        for (s = 0; s < 4; s++) {
            const uint8_t *w = code + i * 16 + s * 4;
            fprintf(f, "%02x%02x%02x%02x ", w[0], w[1], w[2], w[3]);
        }
        fputs(" ; ", f);
        for (s = 0; s < 4; s++) {
            if (!b->usado[s]) {
                continue;
            }
            ins_texto(&b->ins[s], buf, sizeof buf);
            fprintf(f, "%s%s", primero ? "" : " | ", buf);
            primero = 0;
        }
        if (primero) {
            fputs("NOP", f);
        }
        fputc('\n', f);
    }
}
