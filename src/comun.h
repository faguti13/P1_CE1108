#ifndef COMUN_H
#define COMUN_H

#include <stdio.h>

// En este archivo se declaran datos, variables y funciones que se comparten entrew archivos

struct Nodo;

/* Valor que viaja del lexer al parser. No se llama YYSTYPE a proposito:
 * flex, si ve ese nombre, tambien define yylval y choca con bison. */
typedef struct ValorSemantico {
    char *texto;          /* lexema en el heap; NULL en palabras reservadas */
    long entero;          /* valor de un literal entero */
    struct Nodo *nodo;    /* nodo ya reducido, solo lo llena el parser */
    int linea;            // posicion del token (lo llena el lexer)
    int col;
} ValorSemantico;

extern ValorSemantico yylval; // El lexer la llena, parser la utiliza.
extern FILE *yyin; // Apunta al archivo de entrada que analizará el lexer.
extern int yylineno; // Indica la línea actual durante el análisis léxico.

extern char *yytext;
extern int yylex(void);
extern int yyparse(void);
extern void yyerror(const char *mensaje);

// Columna del caracter que se va a leer, y columna donde empezo el token. 
extern int yycol;
extern int yycol_token;

/* -v enciende la traza de lo que el lexer descarta (comentarios). */
extern int traza_lexer;

/* 1 si hubo un error lexico, incluido un comentario sin cerrar. */
extern int lexer_hubo_error;

#endif
