#ifndef COMUN_H
#define COMUN_H

#include <stdio.h>

struct Nodo;

/* Valor que viaja del lexer al parser. No se llama YYSTYPE a proposito:
 * flex, si ve ese nombre, tambien define yylval y choca con bison. */
typedef struct ValorSemantico {
    char *texto;          /* lexema en el heap; NULL en palabras reservadas */
    long entero;          /* valor de un literal entero */
    struct Nodo *nodo;    /* nodo ya reducido, solo lo llena el parser */
} ValorSemantico;

extern ValorSemantico yylval;
extern FILE *yyin;
extern int yylineno;

extern char *yytext;
extern int yylex(void);
extern int yyparse(void);
extern void yyerror(const char *mensaje);

/* Columna del caracter que se va a leer, y columna donde empezo el token. */
extern int yycol;
extern int yycol_token;

/* -v enciende la traza de lo que el lexer descarta (comentarios). */
extern int traza_lexer;

/* 1 si hubo un error lexico, incluido un comentario sin cerrar. */
extern int lexer_hubo_error;

#endif
