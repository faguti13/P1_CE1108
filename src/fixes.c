/* Correcciones rapidas para el IDE JAF: ';', ')' y '}'.
 * Analiza texto sin modificarlo; ignora comentarios y cadenas.
 * Heuristicas conservadoras: no sustituye al parser de Bison. */
#include "fixes.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { char *s; size_t len, cap; } Buf;
static void put(Buf *b, int c) {
    if (b->len + 2 > b->cap) {
        size_t cap = b->cap ? b->cap * 2 : 256;
        char *p = realloc(b->s, cap);
        if (!p) exit(2);
        b->s = p; b->cap = cap;
    }
    b->s[b->len++] = (char)c; b->s[b->len] = 0;
}
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    Buf b = {0}; int c;
    while ((c = fgetc(f)) != EOF) put(&b, c);
    fclose(f);
    if (!b.s) return strdup("");
    return b.s;
}
static void escaped(FILE *f, const char *s) {
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { fputc('\\', f); fputc(c, f); }
        else if (c == '\n') fputs("\\n", f);
        else if (c == '\r') fputs("\\r", f);
        else if (c == '\t') fputs("\\t", f);
        else if (c < 32) fprintf(f, "\\u%04x", c);
        else fputc(c, f);
    }
}
static void emit(FILE *f, int *count, const char *path, int line, int col,
                 const char *text, const char *reason) {
    if ((*count)++ > 0) fputc(',', f);
    fputs("{\"file\":\"", f); escaped(f, path);
    fprintf(f, "\",\"line\":%d,\"col\":%d,\"kind\":\"insert\",\"text\":\"", line, col);
    escaped(f, text);
    fputs("\",\"detail\":\"", f); escaped(f, reason);
    fputs("\"}", f);
}
static int word(const char *s, const char *w) {
    size_t n = strlen(w);
    return !strncmp(s, w, n) && !(isalnum((unsigned char)s[n]) || s[n] == '_');
}
static int block_header(const char *s) {
    if (word(s,"si") || word(s,"sino") || word(s,"mientras") || word(s,"para")) return 1;
    return (word(s,"ent") || word(s,"real") || word(s,"doble") ||
            word(s,"bool") || word(s,"caract") || word(s,"cad") ||
            word(s,"vacio")) && strstr(s, " def ") != NULL;
}
static int statement_start(const char *s) {
    return word(s,"ent") || word(s,"real") || word(s,"doble") ||
           word(s,"bool") || word(s,"cad") || word(s,"caract") ||
           word(s,"list") || word(s,"retorno") || word(s,"terminar") ||
           word(s,"llamar") || word(s,"si") || word(s,"sino") ||
           word(s,"mientras") || word(s,"para") || word(s,"vacio") ||
           word(s,"#import");
}
/* Devuelve si la linea parece una sentencia independiente, no la
 * continuacion de una expresion. */
static int looks_statement(const char *s) {
    if (statement_start(s)) return 1;
    if (!(isalpha((unsigned char)*s) || *s == '_')) return 0;
    while (isalnum((unsigned char)*s) || *s == '_') s++;
    while (*s == ' ' || *s == '\t') s++;
    return *s == '=' || *s == '[' || (*s == '+' && s[1] == '=') ||
           (*s == '-' && s[1] == '=');
}
static int continuation_end(char c) {
    return c == '=' || c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '%' || c == ',' || c == '!' || c == '&' || c == '|' ||
           c == '<' || c == '>' || c == '(' || c == '[' || c == ':';
}
static int continuation_start(const char *s) {
    return *s == '+' || *s == '-' || *s == '*' || *s == '/' || *s == '%' ||
           *s == '&' || *s == '|' || *s == '.' || *s == ',' || *s == ')' ||
           *s == ']' || *s == '=' || *s == '<' || *s == '>';
}
/* Enmascara comentarios y literales dejando saltos de linea y columnas.
 * El comentario de bloque de JAF se abre/cierra con una raya Unicode. */
static char *mask_source(const char *s) {
    char *dst = strdup(s);
    int state = 0, escape = 0;
    for (size_t i = 0; s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        int dash = (c == 0xe2 && (unsigned char)s[i+1] == 0x80 &&
                    ((unsigned char)s[i+2] == 0x94 || (unsigned char)s[i+2] == 0x93 || (unsigned char)s[i+2] == 0x95));
        if (state == 1) {
            if (c == '\n') state = 0;
            else dst[i] = ' ';
        } else if (state == 2) {
            if (dash) { dst[i] = dst[i+1] = dst[i+2] = ' '; i += 2; state = 0; }
            else if (c != '\n') dst[i] = ' ';
        } else if (state == 3 || state == 4) {
            if (c != '\n') dst[i] = ' ';
            if (escape) escape = 0;
            else if (c == '\\') escape = 1;
            else if (c == (state == 3 ? '"' : '\'')) state = 0;
        } else if (dash) {
            dst[i] = dst[i+1] = dst[i+2] = ' '; i += 2; state = 2;
        } else if (c == '$') { dst[i] = ' '; state = 1; }
        else if (c == '"' || c == '\'') { dst[i] = ' '; state = c == '"' ? 3 : 4; }
    }
    return dst;
}
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r') s++;
    return s;
}
static void analyze(FILE *out, int *count, const char *path) {
    char *src = read_file(path);
    if (!src) return;
    char *masked = mask_source(src);
    /* Pila de llaves: cada entrada contiene linea/columna de apertura. */
    int brace_line[1024], brace_col[1024], braces = 0;
    int parens = 0, brackets = 0, line = 1, last_content_line = 1;
    char *p = masked;
    while (*p) {
        char *start = p;
        char *end = strchr(p, '\n');
        if (!end) end = p + strlen(p);
        size_t n = (size_t)(end - start);
        char *cur = strndup(start, n);
        char *next = *end ? trim(end + 1) : "";
        char *first = trim(cur);
        size_t stop = n;
        while (stop && (cur[stop-1] == ' ' || cur[stop-1] == '\t' || cur[stop-1] == '\r')) stop--;
        if (stop) {
            char last = cur[stop-1];
            last_content_line = line;
            int line_paren_start = parens, line_bracket_start = brackets;
            int line_has_open_brace = 0, line_has_close_brace = 0;
            int unmatched_open_paren = 0;
            for (size_t i = 0; i < stop; i++) {
                switch (cur[i]) {
                case '(': parens++; unmatched_open_paren++; break;
                case ')': if (parens > 0) parens--; if (unmatched_open_paren > 0) unmatched_open_paren--; break;
                case '[': brackets++; break;
                case ']': if (brackets > 0) brackets--; break;
                case '{':
                    line_has_open_brace = 1;
                    if (braces < 1024) { brace_line[braces] = line; brace_col[braces] = (int)i+1; braces++; }
                    break;
                case '}': line_has_close_brace = 1; if (braces > 0) braces--; break;
                }
            }
            /* Si falta ')' justo antes de '{', su insercion es segura
             * para cabeceras de si/mientras/para/def. */
            char *curly = strchr(cur, '{');
            if (curly && block_header(first) && parens > line_paren_start && brackets == line_bracket_start) {
                size_t pos = (size_t)(curly - cur);
                while (pos && isspace((unsigned char)cur[pos-1])) pos--;
                emit(out, count, path, line, (int)pos + 1, ")", "Cerrar parentesis antes de '{'");
                parens--;
            } else if (!line_has_open_brace && !line_has_close_brace &&
                       parens > line_paren_start && brackets == line_bracket_start &&
                       (last == ';' || ((!*next || looks_statement(next)) && !continuation_end(last)))) {
                /* Evitar inferir ')' en llamadas repartidas en varias lineas. */
                if (last == ';' || (!*next && looks_statement(first)) ||
                    (block_header(first) && looks_statement(next))) {
                    size_t pos = stop - (last == ';' ? 1 : 0);
                    emit(out, count, path, line, (int)pos+1, ")", "Cerrar parentesis");
                    parens--;
                }
            }
            /* ';' al final de sentencia (no en cabeceras de bloque, ni
             * entre partes de una expresion/lista multilínea). */
            if (!line_has_open_brace && !line_has_close_brace &&
                brackets == 0 && parens == 0 &&
                !continuation_end(last) && last != ';' && last != '}' &&
                !block_header(first) &&
                (looks_statement(first) || (line_bracket_start > 0 && last == ']')) &&
                (looks_statement(next) || !*next || *next == '}' ||
                 (line_bracket_start > 0 && last == ']')) &&
                !continuation_start(next)) {
                emit(out, count, path, line, (int)stop+1, ";", "Agregar terminador de sentencia ';'");
            }
        }
        free(cur);
        p = *end ? end + 1 : end;
        line++;
    }
    /* Solo cerrar llaves al final: es una propuesta, no garantiza
     * que el bloque debiera cerrarse en ese punto. */
    if (braces > 0) {
        int eof_line = line;
        /* Cuando el archivo ya termina con salto de linea, insertar en
         * la linea siguiente (columna 1); si no, crear linea nueva. */
        for (int i = braces-1; i >= 0; i--) {
            (void)brace_line[i]; (void)brace_col[i];
            emit(out, count, path, eof_line, 1, "}\n", "Cerrar bloque '{' sin llave de cierre");
        }
    }
    (void)last_content_line;
    free(masked); free(src);
}
void fixes_emitir_json(FILE *out, char **archivos, int n) {
    int count = 0;
    fputc('[', out);
    for (int i = 0; i < n; i++) analyze(out, &count, archivos[i]);
    fputc(']', out);
}
