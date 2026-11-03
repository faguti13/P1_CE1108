/*
 * clones.c — Deteccion de clones Tipo I / Tipo II entre archivos.
 *
 * Tokeniza cada .oly con el lexer JAF, segmenta por cuerpos de funcion
 * (def … { … }) y compara huellas entre archivos distintos.
 */

#include "clones.h"

#include "comun.h"
#include "tokens.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_TOKENS   12
#define MAX_CLONES   200
#define HASH_BUCKETS 1024

typedef struct {
    int tk;
    int line;
    char *lexema; /* NULL si no hace falta para Tipo I (keywords/ops) */
} CloneTok;

typedef struct {
    const char *file;
    int line;
    int end;
    int ntok;
    CloneTok *toks; /* vista al array del archivo; no liberar por separado */
    unsigned h1;    /* Tipo I */
    unsigned h2;    /* Tipo II */
} CloneUnit;

typedef struct {
    CloneUnit **items;
    int n;
    int cap;
} Bucket;

static void json_escape(FILE *f, const char *s)
{
    int c;
    if (!s) {
        return;
    }
    for (; (c = (unsigned char)*s); s++) {
        if (c == '"' || c == '\\') {
            fputc('\\', f);
            fputc(c, f);
        } else if (c == '\n') {
            fputs("\\n", f);
        } else {
            fputc(c, f);
        }
    }
}

static unsigned fnv1a_u(unsigned h, unsigned v)
{
    h ^= v & 0xffu;
    h *= 16777619u;
    h ^= (v >> 8) & 0xffu;
    h *= 16777619u;
    h ^= (v >> 16) & 0xffu;
    h *= 16777619u;
    h ^= (v >> 24) & 0xffu;
    h *= 16777619u;
    return h;
}

static unsigned fnv1a_str(unsigned h, const char *s)
{
    if (!s) {
        return fnv1a_u(h, 0);
    }
    for (; *s; s++) {
        h ^= (unsigned char)*s;
        h *= 16777619u;
    }
    return h;
}

/* Codigo normalizado para Tipo II. */
static int tk_norm(int tk)
{
    switch (tk) {
    case TK_IDENT:
        return -1;
    case TK_ENTERO:
        return -2;
    case TK_CADENA:
        return -3;
    case TK_CARACTER:
        return -4;
    default:
        return tk;
    }
}

static int necesita_lexema(int tk)
{
    return tk == TK_IDENT || tk == TK_ENTERO || tk == TK_CADENA
        || tk == TK_CARACTER || tk == TK_ERROR;
}

static CloneTok *tokenizar(const char *ruta, int *out_n)
{
    FILE *fp;
    CloneTok *arr = NULL;
    int n = 0, cap = 0;
    int token;

    *out_n = 0;
    fp = fopen(ruta, "r");
    if (!fp) {
        return NULL;
    }
    lexer_reiniciar(fp);
    yyin = fp;

    while ((token = yylex()) != 0) {
        CloneTok t;
        t.tk = token;
        t.line = yylineno;
        t.lexema = NULL;
        if (necesita_lexema(token) && yytext) {
            t.lexema = strdup(yytext);
        }
        free(yylval.texto);
        yylval.texto = NULL;

        if (n >= cap) {
            int ncap = cap ? cap * 2 : 64;
            CloneTok *na = realloc(arr, (size_t)ncap * sizeof(CloneTok));
            if (!na) {
                break;
            }
            arr = na;
            cap = ncap;
        }
        arr[n++] = t;
    }

    fclose(fp);
    yyin = NULL;
    *out_n = n;
    return arr;
}

static void liberar_toks(CloneTok *arr, int n)
{
    int i;
    if (!arr) {
        return;
    }
    for (i = 0; i < n; i++) {
        free(arr[i].lexema);
    }
    free(arr);
}

static void hash_unit(CloneUnit *u)
{
    int i;
    unsigned h1 = 2166136261u;
    unsigned h2 = 2166136261u;
    for (i = 0; i < u->ntok; i++) {
        int tk = u->toks[i].tk;
        h1 = fnv1a_u(h1, (unsigned)tk);
        h1 = fnv1a_str(h1, u->toks[i].lexema ? u->toks[i].lexema : "");
        h2 = fnv1a_u(h2, (unsigned)tk_norm(tk));
    }
    u->h1 = h1;
    u->h2 = h2;
}

static void bucket_add(Bucket *b, CloneUnit *u)
{
    if (b->n >= b->cap) {
        int ncap = b->cap ? b->cap * 2 : 4;
        CloneUnit **ni = realloc(b->items, (size_t)ncap * sizeof(CloneUnit *));
        if (!ni) {
            return;
        }
        b->items = ni;
        b->cap = ncap;
    }
    b->items[b->n++] = u;
}

static int same_type1(const CloneUnit *a, const CloneUnit *b)
{
    int i;
    if (a->ntok != b->ntok) {
        return 0;
    }
    for (i = 0; i < a->ntok; i++) {
        if (a->toks[i].tk != b->toks[i].tk) {
            return 0;
        }
        {
            const char *la = a->toks[i].lexema ? a->toks[i].lexema : "";
            const char *lb = b->toks[i].lexema ? b->toks[i].lexema : "";
            if (necesita_lexema(a->toks[i].tk) && strcmp(la, lb) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

static int same_type2(const CloneUnit *a, const CloneUnit *b)
{
    int i;
    if (a->ntok != b->ntok) {
        return 0;
    }
    for (i = 0; i < a->ntok; i++) {
        if (tk_norm(a->toks[i].tk) != tk_norm(b->toks[i].tk)) {
            return 0;
        }
    }
    return 1;
}

/* Extrae unidades = cuerpos de funcion a partir de TK_DEF … { … }. */
static CloneUnit *segmentar(const char *file, CloneTok *toks, int ntok, int *out_n)
{
    CloneUnit *units = NULL;
    int nu = 0, cap = 0;
    int i;

    *out_n = 0;
    for (i = 0; i < ntok; i++) {
        int j, k, brace, start, end_line, len;

        if (toks[i].tk != TK_DEF) {
            continue;
        }
        /* Buscar '{' de apertura del cuerpo. */
        j = i + 1;
        while (j < ntok && toks[j].tk != '{') {
            j++;
        }
        if (j >= ntok) {
            continue;
        }
        brace = 1;
        k = j + 1;
        while (k < ntok && brace > 0) {
            if (toks[k].tk == '{') {
                brace++;
            } else if (toks[k].tk == '}') {
                brace--;
            }
            k++;
        }
        if (brace != 0) {
            continue; /* cuerpo sin cerrar */
        }
        /* Unidad: desde TK_DEF inclusive hasta '}' de cierre inclusive. */
        start = i;
        end_line = toks[k - 1].line;
        len = k - start;
        if (len < MIN_TOKENS) {
            i = k - 1;
            continue;
        }
        if (nu >= cap) {
            int ncap = cap ? cap * 2 : 8;
            CloneUnit *nu_arr = realloc(units, (size_t)ncap * sizeof(CloneUnit));
            if (!nu_arr) {
                break;
            }
            units = nu_arr;
            cap = ncap;
        }
        units[nu].file = file;
        units[nu].line = toks[start].line;
        units[nu].end = end_line;
        units[nu].ntok = len;
        units[nu].toks = &toks[start];
        hash_unit(&units[nu]);
        nu++;
        i = k - 1;
    }
    *out_n = nu;
    return units;
}

static void emitir_par(FILE *f, int first, int type,
                       const CloneUnit *a, const CloneUnit *b)
{
    if (!first) {
        fputs(",\n", f);
    }
    fputs("    {\"type\":", f);
    fprintf(f, "%d", type);
    fputs(", \"a\":{\"file\":\"", f);
    json_escape(f, a->file);
    fprintf(f, "\",\"line\":%d,\"end\":%d}", a->line, a->end);
    fputs(", \"b\":{\"file\":\"", f);
    json_escape(f, b->file);
    fprintf(f, "\",\"line\":%d,\"end\":%d}}", b->line, b->end);
}

void clones_emitir_json(FILE *f, char **archivos, int n)
{
    CloneTok **toks_por_arch = NULL;
    int *ntoks = NULL;
    CloneUnit **units_por_arch = NULL;
    int *nunits = NULL;
    Bucket buckets1[HASH_BUCKETS];
    Bucket buckets2[HASH_BUCKETS];
    int i, emitidos = 0, primero = 1;
    /* Pares ya emitidos (evitar duplicar Tipo II si ya salió Tipo I).
     * Clave simple: punteros de unidad ordenados. */
    CloneUnit **seen_a = NULL;
    CloneUnit **seen_b = NULL;
    int nseen = 0, cseen = 0;

    if (n < 2) {
        fputs("[]", f);
        return;
    }

    memset(buckets1, 0, sizeof buckets1);
    memset(buckets2, 0, sizeof buckets2);

    toks_por_arch = calloc((size_t)n, sizeof(CloneTok *));
    ntoks = calloc((size_t)n, sizeof(int));
    units_por_arch = calloc((size_t)n, sizeof(CloneUnit *));
    nunits = calloc((size_t)n, sizeof(int));
    if (!toks_por_arch || !ntoks || !units_por_arch || !nunits) {
        fputs("[]", f);
        goto cleanup_partial;
    }

    for (i = 0; i < n; i++) {
        toks_por_arch[i] = tokenizar(archivos[i], &ntoks[i]);
        units_por_arch[i] = segmentar(archivos[i], toks_por_arch[i],
                                      ntoks[i], &nunits[i]);
    }

    /* Indexar Tipo I y Tipo II. */
    for (i = 0; i < n; i++) {
        int u;
        for (u = 0; u < nunits[i]; u++) {
            CloneUnit *cu = &units_por_arch[i][u];
            bucket_add(&buckets1[cu->h1 % HASH_BUCKETS], cu);
            bucket_add(&buckets2[cu->h2 % HASH_BUCKETS], cu);
        }
    }

    fputs("[\n", f);

    /* Tipo I primero. */
    for (i = 0; i < HASH_BUCKETS && emitidos < MAX_CLONES; i++) {
        Bucket *b = &buckets1[i];
        int x, y;
        for (x = 0; x < b->n && emitidos < MAX_CLONES; x++) {
            for (y = x + 1; y < b->n && emitidos < MAX_CLONES; y++) {
                CloneUnit *a = b->items[x];
                CloneUnit *bunit = b->items[y];
                if (a->file == bunit->file || strcmp(a->file, bunit->file) == 0) {
                    continue;
                }
                if (a->h1 != bunit->h1 || !same_type1(a, bunit)) {
                    continue;
                }
                emitir_par(f, primero, 1, a, bunit);
                primero = 0;
                emitidos++;
                if (nseen >= cseen) {
                    int nc = cseen ? cseen * 2 : 16;
                    CloneUnit **na = realloc(seen_a, (size_t)nc * sizeof(CloneUnit *));
                    CloneUnit **nb = realloc(seen_b, (size_t)nc * sizeof(CloneUnit *));
                    if (!na || !nb) {
                        free(na);
                        free(nb);
                        goto after_pairs;
                    }
                    seen_a = na;
                    seen_b = nb;
                    cseen = nc;
                }
                seen_a[nseen] = a;
                seen_b[nseen] = bunit;
                nseen++;
            }
        }
    }

    /* Tipo II: misma estructura, no reportados ya como Tipo I. */
    for (i = 0; i < HASH_BUCKETS && emitidos < MAX_CLONES; i++) {
        Bucket *b = &buckets2[i];
        int x, y;
        for (x = 0; x < b->n && emitidos < MAX_CLONES; x++) {
            for (y = x + 1; y < b->n && emitidos < MAX_CLONES; y++) {
                CloneUnit *a = b->items[x];
                CloneUnit *bunit = b->items[y];
                int s, ya = 0;
                if (a->file == bunit->file || strcmp(a->file, bunit->file) == 0) {
                    continue;
                }
                if (a->h2 != bunit->h2 || !same_type2(a, bunit)) {
                    continue;
                }
                /* Si también es Tipo I, ya se reportó. */
                if (same_type1(a, bunit)) {
                    continue;
                }
                for (s = 0; s < nseen; s++) {
                    if ((seen_a[s] == a && seen_b[s] == bunit)
                        || (seen_a[s] == bunit && seen_b[s] == a)) {
                        ya = 1;
                        break;
                    }
                }
                if (ya) {
                    continue;
                }
                emitir_par(f, primero, 2, a, bunit);
                primero = 0;
                emitidos++;
            }
        }
    }

after_pairs:
    if (emitidos == 0) {
        fputs("]", f);
    } else {
        fputs("\n  ]", f);
    }

    free(seen_a);
    free(seen_b);

    for (i = 0; i < HASH_BUCKETS; i++) {
        free(buckets1[i].items);
        free(buckets2[i].items);
    }
    for (i = 0; i < n; i++) {
        free(units_por_arch[i]);
        liberar_toks(toks_por_arch[i], ntoks[i]);
    }
    free(units_por_arch);
    free(nunits);
    free(toks_por_arch);
    free(ntoks);
    return;

cleanup_partial:
    free(toks_por_arch);
    free(ntoks);
    free(units_por_arch);
    free(nunits);
}
