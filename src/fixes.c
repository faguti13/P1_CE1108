/* Correcciones rápidas para el IDE JAF: ';', '(', ')', '{' y '}'.
 * Analiza texto sin modificarlo; ignora comentarios y cadenas.
 * Heurísticas conservadoras: no sustituye al parser de Bison. */
#include "fixes.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estructura de un búfer dinámico para almacenar texto que crece según se necesite.
typedef struct { char *s; size_t len, cap; } Buf;

// Añade un carácter al búfer dinámico y expande la memoria (realloc) si se llena.
static void put(Buf *b, int c) {
    if (b->len + 2 > b->cap) {
        size_t cap = b->cap ? b->cap * 2 : 256;
        char *p = realloc(b->s, cap);
        if (!p) exit(2);
        b->s = p; b->cap = cap;
    }
    b->s[b->len++] = (char)c; b->s[b->len] = 0;
}

// Abre un archivo, lee todo su contenido byte por byte en el búfer y lo retorna como texto.
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    Buf b = {0}; int c;
    while ((c = fgetc(f)) != EOF) put(&b, c);
    fclose(f);
    if (!b.s) return strdup("");
    return b.s;
}

// Convierte caracteres especiales (comillas, saltos de línea, etc.) a secuencias de escape JSON.
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

// Imprime un objeto JSON en el archivo de salida detallando la corrección sugerida.
static void emit(FILE *f, int *count, const char *path, int line, int col,
                 const char *text, const char *reason) {
    if ((*count)++ > 0) fputc(',', f);
    fputs("{\"file\":\"", f); escaped(f, path);
    fprintf(f, "\",\"line\":%d,\"col\":%d,\"kind\":\"insert\",\"text\":\"", line, col);
    escaped(f, text);
    fputs("\",\"detail\":\"", f); escaped(f, reason);
    fputs("\"}", f);
}

// Comprueba si el texto comienza con una palabra clave exacta y no es parte de un identificador.
static int word(const char *s, const char *w) {
    size_t n = strlen(w);
    return !strncmp(s, w, n) && !(isalnum((unsigned char)s[n]) || s[n] == '_');
}

// Identifica si la línea es la cabecera de un bloque (if, else, while, for, o definición de función).
static int block_header(const char *s) {
    if (word(s,"si") || word(s,"sino") || word(s,"mientras") || word(s,"para")) return 1;
    return (word(s,"ent") || word(s,"real") || word(s,"doble") ||
            word(s,"bool") || word(s,"caract") || word(s,"cad") ||
            word(s,"vacio")) && strstr(s, " def ") != NULL;
}

// Revisa si la palabra inicial de una línea arranca una sentencia válida (variables, control, etc.).
static int statement_start(const char *s) {
    return word(s,"ent") || word(s,"real") || word(s,"doble") ||
           word(s,"bool") || word(s,"cad") || word(s,"caract") ||
           word(s,"list") || word(s,"retorno") || word(s,"terminar") ||
           word(s,"llamar") || word(s,"si") || word(s,"sino") ||
           word(s,"mientras") || word(s,"para") || word(s,"vacio") ||
           word(s,"#import");
}

// Devuelve 1 si la línea parece una instrucción independiente y no la continuación de otra.
static int looks_statement(const char *s) {
    if (statement_start(s)) return 1;
    if (!(isalpha((unsigned char)*s) || *s == '_')) return 0;
    while (isalnum((unsigned char)*s) || *s == '_') s++;
    while (*s == ' ' || *s == '\t') s++;
    return *s == '=' || *s == '[' || (*s == '+' && s[1] == '=') ||
           (*s == '-' && s[1] == '=');
}

// Indica si un operador final sugiere que la instrucción continúa en la siguiente línea.
static int continuation_end(char c) {
    return c == '=' || c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '%' || c == ',' || c == '!' || c == '&' || c == '|' ||
           c == '<' || c == '>' || c == '(' || c == '[' || c == ':';
}

// Indica si el inicio de una línea parece continuar la expresión de la línea anterior.
static int continuation_start(const char *s) {
    return *s == '+' || *s == '-' || *s == '*' || *s == '/' || *s == '%' ||
           *s == '&' || *s == '|' || *s == '.' || *s == ',' || *s == ')' ||
           *s == ']' || *s == '=' || *s == '<' || *s == '>';
}

// Reemplaza comentarios y cadenas por espacios para analizarlos sin borrar saltos de línea ni columnas.
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

// Remueve espacios, tabulaciones 
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r') s++;
    return s;
}

// Avanza en el texto hasta encontrar el próximo carácter con código (se salta espacios y saltos de línea).
static const char *next_code(const char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    return s;
}

// Reconoce palabras clave de cabecera ("si", "mientras", "def", etc.) y devuelve qué tipo de sintaxis requieren.
static int header_kind(const char *s, size_t *after_keyword) {
    const char *p = s;
    if (word(p, "sino")) {
        p += 4;
        while (*p == ' ' || *p == '\t') p++;
        if (word(p, "si")) {
            *after_keyword = (size_t)(p + 2 - s);
            return 1;
        }
        *after_keyword = (size_t)(p - s);
        return 2;
    }
    const char *words[] = {"si", "mientras", "para"};
    for (size_t i = 0; i < 3; i++) {
        if (word(p, words[i])) {
            *after_keyword = strlen(words[i]);
            return 1;
        }
    }
    if (word(p,"ent") || word(p,"real") || word(p,"doble") ||
        word(p,"bool") || word(p,"caract") || word(p,"cad") ||
        word(p,"vacio")) {
        while (isalnum((unsigned char)*p) || *p == '_') p++;
        while (*p == ' ' || *p == '\t') p++;
        if (word(p, "def")) {
            *after_keyword = (size_t)(p + 3 - s);
            return 1;
        }
    }
    return 0;
}

// Analiza línea por línea el archivo para verificar el balance de parentesis, llaves y la presencia de ';'.
static void analyze(FILE *out, int *count, const char *path) {
    char *src = read_file(path);
    if (!src) return;
    char *masked = mask_source(src);
    int braces = 0, parens = 0, brackets = 0, line = 1;
    char previous_last = 0;
    char *p = masked;
    while (*p) {
        char *start = p;
        char *end = strchr(p, '\n');
        if (!end) end = p + strlen(p);
        size_t n = (size_t)(end - start);
        char *cur = strndup(start, n);
        char *first = trim(cur);
        const char *next = next_code(*end ? end + 1 : end);
        size_t stop = n;
        while (stop && (cur[stop-1] == ' ' || cur[stop-1] == '\t' || cur[stop-1] == '\r')) stop--;

        if (stop && *first) {
            char last = cur[stop-1];
            int line_paren_start = parens, line_bracket_start = brackets;
            int line_has_open_brace = 0, line_has_close_brace = 0;
            size_t first_open_brace = (size_t)-1;
            
            // Cuenta la apertura y cierre de símbolos ( ( ), [ ], { } ) en la línea actual.
            for (size_t i = 0; i < stop; i++) {
                switch (cur[i]) {
                case '(': parens++; break;
                case ')':
                    if (parens > 0) parens--;
                    break;
                case '[': brackets++; break;
                case ']': if (brackets > 0) brackets--; break;
                case '{':
                    line_has_open_brace = 1;
                    if (first_open_brace == (size_t)-1) first_open_brace = i;
                    braces++;
                    break;
                case '}':
                    line_has_close_brace = 1;
                    if (braces > 0) braces--;
                    break;
                }
            }

            size_t keyword_len = 0;
            int header = header_kind(first, &keyword_len);
            
            // Sugiere cerrar el bloque 'si' anterior si encuentra un 'sino' sin llave de cierre previo.
            if (word(first, "sino") && previous_last != '}' && braces > 0) {
                emit(out, count, path, line, (int)(first - cur) + 1,
                     "} ", "Cerrar bloque 'si' antes de 'sino'");
                braces--;
            }
            
            size_t first_offset = (size_t)(first - cur);
            int has_open_paren = strchr(first, '(') != NULL;
            int has_close_paren = strchr(first, ')') != NULL;
            int multiline = 0;
            
            // Detecta si una cabecera de control está dividida en varias líneas.
            if (header == 1 && !has_close_paren &&
                (continuation_end(last) || *next == ')' ||
                 *next == '&' || *next == '|' || *next == ',' ||
                 (*next == '{' && last == '(')))
                multiline = 1;
            if (header && !line_has_open_brace && *next == '{' && has_close_paren) {
                multiline = 1;
            }

            // Sugiere agregar '(' de apertura si la cabecera solo tiene el de cierre ')'.
            if (header == 1 && !has_open_paren && has_close_paren) {
                size_t pos = first_offset + keyword_len;
                emit(out, count, path, line, (int)pos + 1, "(",
                     "Agregar '(' de la cabecera");
            }

            // Sugiere agregar los ')' faltantes al final de una cabecera.
            if (header == 1 && has_open_paren && parens > line_paren_start &&
                brackets == line_bracket_start && !multiline) {
                int missing = parens - line_paren_start;
                size_t pos = first_open_brace != (size_t)-1 ? first_open_brace : stop;
                if (pos == stop && last == ';') pos--;
                while (pos && isspace((unsigned char)cur[pos-1])) pos--;
                for (int k = 0; k < missing; k++) {
                    emit(out, count, path, line, (int)pos + 1, ")",
                         "Cerrar parentesis de la cabecera");
                    parens--;
                }
            }

            // Sugiere agregar '{' al final de una cabecera que no la incluye.
            if (header && !line_has_open_brace && !multiline &&
                *next != '{' && brackets == line_bracket_start) {
                int header_complete = header == 2 ||
                    (has_close_paren && parens == line_paren_start) ||
                    (has_open_paren && !continuation_end(last) && parens == line_paren_start);
                if (header_complete) {
                    size_t pos = stop;
                    emit(out, count, path, line, (int)pos + 1, " {",
                         "Agregar '{' al bloque obligatorio");
                    braces++;
                }
            }

            // Sugiere cerrar paréntesis abiertos en expresiones fuera de cabeceras.
            if (!header && !line_has_open_brace && !line_has_close_brace &&
                parens > line_paren_start && brackets == line_bracket_start &&
                (last == ';' || ((!*next || looks_statement(next)) && !continuation_end(last))) &&
                (last == ';' || (!*next && looks_statement(first)))) {
                size_t pos = stop - (last == ';' ? 1 : 0);
                emit(out, count, path, line, (int)pos + 1, ")", "Cerrar parentesis");
                parens--;
            }

            // Sugiere agregar ';' al final de líneas que correspondan a una instrucción completa.
            if (!line_has_open_brace && !line_has_close_brace &&
                brackets == 0 && parens == 0 &&
                !continuation_end(last) && last != ';' && last != '}' &&
                !block_header(first) &&
                (looks_statement(first) || (line_bracket_start > 0 && last == ']')) &&
                (looks_statement(next) || !*next || *next == '}' ||
                 (line_bracket_start > 0 && last == ']')) &&
                !continuation_start(next)) {
                emit(out, count, path, line, (int)stop + 1, ";",
                     "Agregar terminador de sentencia ';'");
            }
            previous_last = last;
        }
        free(cur);
        p = *end ? end + 1 : end;
        line++;
    }
    
    // trata de cerrar todas las llaves '{' que quedaron abiertas al terminar el archivo.
    for (int i = 0; i < braces; i++) {
        emit(out, count, path, line, 1, "}\n",
             "Cerrar bloque sin llave de cierre");
    }
    free(masked);
    free(src);
}

// Función principal expuesta: toma la lista de archivos, los analiza y genera el arreglo JSON resultante.
void fixes_emitir_json(FILE *out, char **archivos, int n) {
    int count = 0;
    fputc('[', out);
    for (int i = 0; i < n; i++) analyze(out, &count, archivos[i]);
    fputc(']', out);
}