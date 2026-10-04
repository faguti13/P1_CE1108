#include "ast.h"
#include "tokens.h"

#include <stdlib.h>
#include <string.h>

int yycol = 1;
int yycol_token = 1;
int traza_lexer = 0;
int lexer_hubo_error = 0;

static void uso(const char *programa)
{
    fprintf(stderr,
            "uso: %s [-v] [--arbol] <archivo.oly>\n"
            "  sin --arbol   imprime los tokens del fuente\n"
            "  --arbol       parsea e imprime el arbol sintactico\n"
            "  -v            avisa los comentarios que el lexer descarta\n",
            programa);
}

void yyerror(const char *mensaje)
{
    fprintf(stderr, "error de sintaxis:%d:%d: %s\n",
            yylineno, yycol_token, mensaje);
}

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
    ast_raiz = NULL;
    return 1;
}

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

static int imprimir_arbol(const char *ruta)
{
    int estado = yyparse();

    if (estado == 0 && ast_raiz && !lexer_hubo_error) {
        printf("Archivo: %s\n", ruta);
        nodo_imprimir(ast_raiz, 0);
    }
    nodo_liberar(ast_raiz);
    ast_raiz = NULL;
    if (estado != 0 || lexer_hubo_error) {
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int arbol = 0;
    int estado;
    const char *ruta = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            traza_lexer = 1;
        } else if (strcmp(argv[i], "--arbol") == 0) {
            arbol = 1;
        } else if (argv[i][0] == '-') {
            uso(argv[0]);
            return 2;
        } else if (ruta) {
            uso(argv[0]);
            return 2;
        } else {
            ruta = argv[i];
        }
    }

    if (!ruta) {
        uso(argv[0]);
        return 2;
    }
    if (!abrir(ruta)) {
        return 1;
    }

    estado = arbol ? imprimir_arbol(ruta) : volcar_tokens(ruta);
    fclose(yyin);
    yyin = NULL;
    return estado;
}
