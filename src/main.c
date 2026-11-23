/*
 * main.c - Línea de comandos del compilador JAF y orden de las fases.
 *   jaf [opciones] <fuente.oly> [-o salida]
 * Con un .oly: léxico y sintáctico (yyparse) -> semántico -> codegen (IR)
 *   -> planificador (bundles) -> resolución de etiquetas y codificación -> .bin
 *   (y además .asm, .mem o -p según las opciones).
 * Con -x la entrada es un .asm: se ensambla y se salta directo a la resolución
 *   y la codificación.
 */
#include "ast.h"
#include "codegen.h"
#include "emisor.h"
#include "ensamblador.h"
#include "ide.h"
#include "isa.h"
#include "linker.h"
#include "opt.h"
#include "planificador.h"
#include "semantico.h"
#include "sim_ciclos.h"
#include "tokens.h"
#include "ide.h"

#include <stdlib.h>
#include <string.h>

/* Variables que comparte el lexer generado por flex: 
-   columna actual
-   columna del último token
-   traza de comentarios (-v) 
-   bandera de error léxico. */
int yycol = 1;
int yycol_token = 1;
int traza_lexer = 0;
int lexer_hubo_error = 0;
int lexer_silencio_ide = 0;

/* Bandera de error de sintaxis y nombre del fuente para los mensajes. */
static int hubo_sintaxis = 0;
static const char *archivo_actual = "(entrada)";

 /* Imprime los mensajes de ayuda para todas las opciones*/
static void uso(const char *programa)
{
    fprintf(stderr,
            "uso: %s [opciones] <fuente.oly>\n"
            "\n"
            "  -o <salida>  archivo binario de salida (por defecto <fuente>.bin)\n"
            "  -s           escribe tambien el ensamblador generado (<salida>.asm)\n"
            "  --mem        escribe tambien la imagen para el simulador (<salida>.mem)\n"
            "  -t           imprime la tabla de tokens del fuente\n"
            "  -m           imprime la tabla de simbolos\n"
            "  -p           imprime los paquetes (bundles) en ensamblador y en binario\n"
            "  -x           la entrada es ensamblador (.asm): lo ensambla a binario\n"
            "  -v           modo detallado: fases, estadisticas y comentarios descartados\n"
            "  --arbol      imprime el arbol sintactico y termina\n"
            "  --json       imprime el arbol en JSON y termina (jsoncrack.com)\n"
            "  --ide        JSON para el IDE: diagnosticos, fixes, sugerencias y clones (varios .oly)\n"
            "  --ide-todo   JSON del IDE + simbolos, AST, TAC, bundles y metricas (un .oly;\n"
            "               acepta las opciones de optimizacion)\n"
            "  --tabla-json imprime la tabla de simbolos en JSON\n"
            "  --tokens     igual que -t\n"
            "  -h           esta ayuda\n"
            "\n"
            "  Optimizacion (Proyecto II; sin estas opciones se usa el generador de P1):\n"
            "  -O0          tuberia TAC sin optimizar (virtuales en memoria)\n"
            "  -O1          -O0 + --spill --dce\n"
            "  -O2          -O1 + --unroll=auto --rename --reorder\n"
            "  --unroll=K   desenrolla lazos para canonicos K veces (K>=2 o auto)\n"
            "  --dce        elimina codigo muerto e inalcanzable\n"
            "  --rename     renombra definiciones dentro del bloque (quita WAW/WAR)\n"
            "  --reorder    reordena cada bloque por altura del DAG\n"
            "  --spill      asigna registros r8..r23 y derrama al marco (fp)\n"
            "  --max-regs=N registros asignables con --spill (1..16, por defecto 16)\n"
            "  --p1         fuerza el generador de P1 aunque haya opciones de optimizacion\n"
            "  -i, --ir     imprime el TAC final;  --ir-raw: el TAC sin optimizar\n"
            "  --cfg        imprime bloques basicos, lazos y vivacidad\n"
            "  --estad      imprime las estadisticas de optimizacion en JSON (stdout)\n"
            "  --sim-ciclos ejecuta el binario en el simulador rapido: ciclos, IPC y\n"
            "               una estimacion de L1+L2 (la cifra exacta es la de tb_top)\n"
            "\n",
            programa);
}

 /* Detecta cuando se presenta un error de sintaxis */
void yyerror(const char *mensaje)
{
    hubo_sintaxis = 1;
    const char *arch = ast_archivo ? ast_archivo : archivo_actual;
    fprintf(stderr, "{ERROR 02}:%s:%d:%d:\"sintaxis: %s\"\n", arch, yylineno, yycol_token, mensaje);
    ide_registrar_error(arch, yylineno, yycol_token, 2, mensaje, "Error");
}

 /* Abre el archivo fuente como entrada del lexer y reinicia el estado de:
 -   línea
 -   columna
 -   banderas de error 
 -   raíz del AST.*/
static int abrir(const char *ruta)
{
    yyin = fopen(ruta, "r");
    if (!yyin) {
        fprintf(stderr, "no se pudo abrir '%s'\n", ruta);
        return 0;
    }
    yylineno = 1;
    yycol = 1;
    yycol_token = 1;
    lexer_hubo_error = 0;
    hubo_sintaxis = 0;
    ast_raiz = NULL;
    ast_archivo = ruta;
    return 1;
}

/* Opción -t: lee todos los tokens del archivo fuente y los imprime con:
 -   línea:columna
 -   nombre del token
 -   lexema
   No parsea el programa. Devuelve 1 si hubo un error léxico. */
static int volcar_tokens(const char *ruta)
{
    int token;
    int cantidad = 0;

    printf("Archivo: %s\n", ruta);
    printf("%8s  %-16s  %s\n", "linea:col", "token", "lexema");
    while ((token = yylex()) != 0) {
        printf("%4d:%-4d  %-16s  %s\n",
               yylineno, yycol_token, nombre_token(token), yytext);
        free(yylval.texto);
        yylval.texto = NULL;
        cantidad++;
    }
    printf("%d tokens\n", cantidad);
    return lexer_hubo_error ? 1 : 0;
}

/* Lee el archivo completo a memoria y lo termina en '\0'.
   Se usa con -x para pasarle el .asm al ensamblador. */
static char *leer_archivo(const char *ruta)
{
    FILE *f = fopen(ruta, "rb");
    long n;
    char *buf;

    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc((size_t)n + 1);
    if (buf && fread(buf, 1, (size_t)n, f) == (size_t)n) {
        buf[n] = '\0';
    } else {
        free(buf);
        buf = NULL;
    }
    fclose(f);
    return buf;
}

/* Devuelve una copia de la ruta sin la extensión (prog.oly -> prog).
   Se usa para armar los nombres de salida .bin, .asm y .mem. */
static char *base_de(const char *ruta)
{
    char *b = strdup(ruta);
    char *p = strrchr(b, '.');
    char *s = strrchr(b, '/');
    if (p && (!s || p > s)) {
        *p = '\0';
    }
    return b;
}

/* Fases 5 y 6:
 -   asigna las direcciones de los bundles y revisa etiquetas repetidas (ERROR 40)
 -   calcula los saltos y codifica cada slot (ERROR 41)
 -   escribe el .asm (-s) e imprime los paquetes (-p) si se pidieron
 -   escribe el .bin y, con --mem, la imagen .mem para el simulador
   Devuelve 0 si todo salió bien y 1 si algo falló. */
static int generar_salidas(Programa *p, const char *salida, int escribir_mem, int mostrar_paquetes,
                           int verboso, const char *asm_ruta, const char *origen)
{
    char err[256];
    uint8_t *code;
    char *ruta_mem = NULL;

    if (prog_resolver(p, err, sizeof err)) {
        fprintf(stderr, "{ERROR 40}:%s:0:0:\"%s\"\n", origen, err);
        return 1;
    }
    code = calloc((size_t)(p->n ? p->n : 1), 16);
    if (prog_codificar(p, code, err, sizeof err)) {
        fprintf(stderr, "{ERROR 41}:%s:0:0:\"%s\"\n", origen, err);
        free(code);
        return 1;
    }
    if (asm_ruta) {
        FILE *f = fopen(asm_ruta, "w");
        if (!f) {
            fprintf(stderr, "no se pudo escribir '%s'\n", asm_ruta);
            free(code);
            return 1;
        }
        emitir_asm(p, f, origen);
        fclose(f);
        if (verboso) {
            fprintf(stderr, "[jaf] ensamblador: %s\n", asm_ruta);
        }
    }
    if (mostrar_paquetes) {
        emitir_paquetes(p, code, stdout);
    }
    if (prog_escribir_bin(p, code, salida)) {
        fprintf(stderr, "no se pudo escribir '%s'\n", salida);
        free(code);
        return 1;
    }
    if (verboso) {
        fprintf(stderr, "[jaf] binario: %s (%d bundles, %d bytes de codigo)\n", salida, p->n, p->n * 16);
    }
    if (escribir_mem) {
        char *b = base_de(salida);
        ruta_mem = malloc(strlen(b) + 5);
        sprintf(ruta_mem, "%s.mem", b);
        free(b);
        if (prog_escribir_mem(p, code, ruta_mem)) {
            fprintf(stderr, "no se pudo escribir '%s'\n", ruta_mem);
            free(code);
            free(ruta_mem);
            return 1;
        }
        if (verboso) {
            fprintf(stderr, "[jaf] imagen de memoria: %s\n", ruta_mem);
        }
        free(ruta_mem);
    }
    free(code);
    return 0;
}

/* -m imprime la tabla en texto; --tabla-json, en JSON (la misma que usa el IDE). */
static void imprimir_tabla(int modo)
{
    if (modo == 2) {
        ide_simbolos_json(stdout);
        putchar('\n');
    } else {
        sem_imprimir_tabla(stdout);
    }
}

/* Punto de entrada del compilador:
 -   lee las opciones y sus alias
 -   arma los nombres de salida (<fuente>.bin y <salida>.asm)
 -   con -x ensambla el .asm y salta a generar_salidas
 -   con -t imprime los tokens y con --arbol imprime el AST
 -   fase 3: análisis semántico (-m imprime la tabla de símbolos)
 -   fases 4 a 6: genera el código intermedio, lo planifica en bundles y escribe las salidas
   Con -v muestra cada fase. Devuelve 0 si compiló, 1 si hubo errores y 2 si
   la línea de comandos es inválida. */
int main(int argc, char **argv)
{
    ide_limpiar_errores();
    const char *ruta = NULL;
    const char *salida = NULL;
    int opt_tokens = 0, opt_arbol = 0, opt_json = 0, opt_tabla = 0, opt_paquetes = 0, opt_asm = 0;
    int opt_x = 0, opt_mem = 0, opt_ide = 0, verboso = 0;
    char *salida_def = NULL;
    char *asm_ruta = NULL;
    char *rutas[32];
    int n_rutas = 0;
    int i, estado = 0;
    OptCfg oc;
    int opt_p1 = 0, opt_estad = 0, opt_sim = 0;

    memset(&oc, 0, sizeof oc);
    oc.max_regs = 16;
    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-O0") || !strcmp(a, "-O1") || !strcmp(a, "-O2")) {
            oc.activa = 1;
            if (a[2] >= '1') {
                oc.spill = oc.dce = 1;
            }
            if (a[2] >= '2') {
                oc.unroll = -1;
                oc.rename = oc.reorder = 1;
            }
            continue;
        } else if (!strncmp(a, "--unroll=", 9)) {
            oc.activa = 1;
            oc.unroll = !strcmp(a + 9, "auto") ? -1 : atoi(a + 9);
            if (oc.unroll == 0 || oc.unroll == 1) {
                oc.unroll = 0;
            } else if (oc.unroll < -1 || oc.unroll > 64) {
                fprintf(stderr, "--unroll espera K entre 2 y 64 o auto\n");
                return 2;
            }
            continue;
        } else if (!strcmp(a, "--unroll")) {
            oc.activa = 1;
            oc.unroll = -1;
            continue;
        } else if (!strcmp(a, "--dce")) {
            oc.activa = oc.dce = 1;
            continue;
        } else if (!strcmp(a, "--rename")) {
            oc.activa = oc.rename = 1;
            continue;
        } else if (!strcmp(a, "--reorder")) {
            oc.activa = oc.reorder = 1;
            continue;
        } else if (!strcmp(a, "--spill")) {
            oc.activa = oc.spill = 1;
            continue;
        } else if (!strncmp(a, "--max-regs=", 11)) {
            oc.max_regs = atoi(a + 11);
            if (oc.max_regs < 1 || oc.max_regs > 16) {
                fprintf(stderr, "--max-regs espera un valor entre 1 y 16\n");
                return 2;
            }
            continue;
        } else if (!strcmp(a, "-i") || !strcmp(a, "--ir")) {
            oc.activa = oc.ver_ir = 1;
            continue;
        } else if (!strcmp(a, "--ir-raw")) {
            oc.activa = oc.ver_ir_crudo = 1;
            continue;
        } else if (!strcmp(a, "--cfg")) {
            oc.activa = oc.ver_cfg = 1;
            continue;
        } else if (!strcmp(a, "--p1")) {
            opt_p1 = 1;
            continue;
        } else if (!strcmp(a, "--estad")) {
            opt_estad = 1;
            continue;
        } else if (!strcmp(a, "--sim-ciclos")) {
            opt_sim = 1;
            continue;
        }
        if (!strcmp(a, "-h") || !strcmp(a, "~ay") || !strcmp(a, "--help")) {
            uso(argv[0]);
            return 0;
        } else if (!strcmp(a, "-v") || !strcmp(a, "~fp")) {
            verboso = 1;
            traza_lexer = 1;
        } else if (!strcmp(a, "-t") || !strcmp(a, "~as") || !strcmp(a, "--tokens")) {
            opt_tokens = 1;
        } else if (!strcmp(a, "--arbol")) {
            opt_arbol = 1;
        } else if (!strcmp(a, "--json")) {
            opt_json = 1;
        } else if (!strcmp(a, "--ide")) {
            opt_ide = 1;
        } else if (!strcmp(a, "--ide-todo")) {
            opt_ide = 2;
        } else if (!strcmp(a, "--tabla-json")) {
            opt_tabla = 2;
        } else if (!strcmp(a, "-m")) {
            opt_tabla = 1;
        } else if (!strcmp(a, "-p")) {
            opt_paquetes = 1;
        } else if (!strcmp(a, "-s")) {
            opt_asm = 1;
        } else if (!strcmp(a, "-x")) {
            opt_x = 1;
        } else if (!strcmp(a, "--mem")) {
            opt_mem = 1;
        } else if (!strcmp(a, "-o") || !strcmp(a, "~gc")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "%s necesita un archivo\n", a);
                return 2;
            }
            salida = argv[++i];
        } else if (!strcmp(a, "~bd") || !strcmp(a, "~c")) {
            fprintf(stderr, "aviso: la opcion %s no esta soportada en esta version; se ignora\n", a);
            if (!strcmp(a, "~bd") && i + 1 < argc) {
                i++;
            }
        } else if (a[0] == '-' || a[0] == '~') {
            fprintf(stderr, "opcion desconocida: %s\n", a);
            uso(argv[0]);
            return 2;
        } else if (n_rutas >= 32) {
            fprintf(stderr, "demasiados archivos\n");
            return 2;
        } else {
            rutas[n_rutas++] = (char *)a;
        }
    }
    if (opt_ide == 2) {
        Nodo *raiz = NULL;
        if (n_rutas != 1) {
            fprintf(stderr, "uso: %s --ide-todo <a.oly> [opciones de optimizacion]\n", argv[0]);
            return 2;
        }
        archivo_actual = rutas[0];
        if (abrir(rutas[0])) {
            estado = yyparse();
            fclose(yyin);
            yyin = NULL;
            if (estado == 0 && !lexer_hubo_error && !hubo_sintaxis && ast_raiz
                && enlazar(ast_raiz, rutas[0], 0) == 0) {
                raiz = ast_raiz;
            }
        }
        oc.activa = 1;
        return ide_todo(raiz, rutas[0], &oc);
    }
    if (opt_ide) {
        if (n_rutas < 1) {
            fprintf(stderr, "uso: %s --ide <a.oly> [b.oly ...]\n", argv[0]);
            return 2;
        }
        // Analizar los archivos para que se registren los errores
        for (int k = 0; k < n_rutas; k++) {
            archivo_actual = rutas[k];
            if (abrir(rutas[k])) {
                yyparse();
                fclose(yyin);
                yyin = NULL;
                if (ast_raiz) {
                    sem_analizar(ast_raiz, rutas[k]);
                    nodo_liberar(ast_raiz);
                    ast_raiz = NULL;
                }
            }
        }
        ide_emitir_json(rutas, n_rutas);
        return 0;
    }
    if (n_rutas != 1) {
        uso(argv[0]);
        return 2;
    }
    ruta = rutas[0];
    if (!salida) {
        char *b = base_de(ruta);
        salida_def = malloc(strlen(b) + 5);
        sprintf(salida_def, "%s.bin", b);
        free(b);
        salida = salida_def;
    }
    if (opt_asm) {
        char *b = base_de(salida);
        asm_ruta = malloc(strlen(b) + 5);
        sprintf(asm_ruta, "%s.asm", b);
        free(b);
    }

    /* ---- entrada en ensamblador ---- */
    if (opt_x) {
        char err[256];
        Programa p;
        char *texto = leer_archivo(ruta);
        if (!texto) {
            fprintf(stderr, "no se pudo abrir '%s'\n", ruta);
            return 1;
        }
        if (verboso) {
            fprintf(stderr, "[jaf] ensamblando %s\n", ruta);
        }
        if (ensamblar_texto(texto, &p, err, sizeof err)) {
            fprintf(stderr, "%s: error: %s\n", ruta, err);
            return 1;
        }
        return generar_salidas(&p, salida, opt_mem, opt_paquetes, verboso, NULL, ruta);
    }

    archivo_actual = ruta;
    if (!abrir(ruta)) {
        return 1;
    }
    if (opt_tokens) {
        estado = volcar_tokens(ruta);
        fclose(yyin);
        return estado;
    }

    /* ---- fases 1 y 2: léxico y sintáctico ---- */
    if (verboso) {
        fprintf(stderr, "[jaf] analisis lexico y sintactico de %s\n", ruta);
    }
    estado = yyparse();
    fclose(yyin);
    yyin = NULL;
    if (estado != 0 || lexer_hubo_error || hubo_sintaxis || !ast_raiz) {
        return 1;
    }
    /* ---- enlazado de #import (antes del semantico) ---- */
    if (enlazar(ast_raiz, ruta, verboso) > 0) {
        return 1;
    }
    if (opt_arbol) {
        printf("Archivo: %s\n", ruta);
        nodo_imprimir(ast_raiz, 0);
        return 0;
    }
    if (opt_json) {
        nodo_json(stdout, ast_raiz);
        return 0;
    }

    /* ---- fase 3: semántico ---- */
    if (verboso) {
        fprintf(stderr, "[jaf] analisis semantico\n");
    }
    if (sem_analizar(ast_raiz, ruta) > 0) {
        if (opt_tabla) {
            imprimir_tabla(opt_tabla);
        }
        fprintf(stderr, "%d error(es) semantico(s)\n", sem.errores);
        return 1;
    }
    if (opt_tabla) {
        imprimir_tabla(opt_tabla);
    }

    /* ---- fases 4 a 6: código, planificación, codificación ---- */
    {
        IR ir;
        Programa p;
        EstadPlan est;
        EstadOpt eo;
        EstadSim es;

        memset(&ir, 0, sizeof ir);
        memset(&eo, 0, sizeof eo);
        if (oc.activa && !opt_p1) {
            if (verboso) {
                fprintf(stderr, "[jaf] tuberia TAC:%s%s%s%s%s\n", oc.unroll ? " unroll" : "", oc.dce ? " dce" : "",
                        oc.rename ? " rename" : "", oc.reorder ? " reorder" : "", oc.spill ? " spill" : " (memoria)");
            }
            if (opt_compilar(ast_raiz, ruta, &oc, &ir, &eo, NULL) > 0) {
                return 1;
            }
            if (verboso) {
                opt_imprimir_estad(&eo, stderr);
            }
        } else if (gen_programa(ast_raiz, &ir, ruta) > 0) {
            return 1;
        }
        if (verboso) {
            fprintf(stderr, "[jaf] codigo intermedio: %d elementos\n", ir.n);
        }
        planificar(&ir, &p, &est);
        if (verboso) {
            fprintf(stderr, "[jaf] planificacion: %ld instrucciones en %ld bundles (%.1f%% de los slots)\n",
                    est.instrucciones, est.bundles,
                    est.bundles ? 100.0 * (double)est.slots_usados / (4.0 * (double)est.bundles) : 0.0);
        }
        estado = generar_salidas(&p, salida, opt_mem, opt_paquetes, verboso, asm_ruta, ruta);
        if (estado == 0 && opt_sim) {
            sim_ciclos(&p, 50000000L, &es, NULL);
            if (!opt_estad) {
                sim_imprimir(&es, stdout);
            }
        }
        if (estado == 0 && opt_estad) {
            printf("{\"archivo\": \"%s\", \"bundles\": %ld, \"instrucciones\": %ld, \"slots_usados\": %ld, \"opt\": ", ruta,
                   est.bundles, est.instrucciones, est.slots_usados);
            opt_estad_json(&eo, stdout);
            if (opt_sim) {
                printf(", \"sim\": ");
                sim_json(&es, stdout);
            }
            printf("}\n");
        }
    }
    free(salida_def);
    free(asm_ruta);
    return estado;
}
