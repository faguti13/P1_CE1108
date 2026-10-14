%{
#include "tokens.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * Analizador lexico de JAF.
 *
 * Flex recorre el fuente de izquierda a derecha y, en cada posicion,
 * se queda con la coincidencia mas larga. Si dos reglas cubren el
 * mismo texto y miden lo mismo, gana la que aparece primero en este
 * archivo. Por eso `ent` es DataE y `entero` es IdVAR: la segunda
 * palabra es mas larga. Y por eso `si` esta antes de la regla del
 * identificador: miden lo mismo, y la palabra reservada tiene que ganar.
 *
 * El orden de estos grupos es el de la Documentacion del compilador,
 * seccion 3.1.
 *
 * Lecturas abiertas
 * -----------------
 * Una lectura abierta es una condicion de arranque exclusiva: el lexer
 * sale de INITIAL y sigue consumiendo hasta un cierre, aunque haya
 * saltos de linea.
 *
 * 1. COM_ML. Se abre con una raya (U+2014, U+2013 o U+2015) y se cierra
 *    con la segunda raya. No anida. Un guion simple o doble no la abre:
 *    `-` y `--` siguen siendo el operador menos. Un `$` adentro es
 *    texto del comentario, no un comentario de una linea.
 *    La expresion de la tabla (`—[^-]*—`) no sirve tal cual: `[^-]`
 *    incluye a la propia raya y, por ser greedy, se comeria desde la
 *    primera raya hasta la ultima del archivo. La condicion de arranque
 *    cierra en la segunda.
 * 2. Terminar. Si el archivo se acaba dentro de COM_ML, la lectura no
 *    llega a cerrarse: se reporta error y se devuelve TK_ERROR. La
 *    palabra `terminar` no abre esta lectura: es el token BRK, junto
 *    con las palabras de funcion.
 */

static int com_ml_linea;

static void marcar(void)
{
    yycol_token = yycol;
    yycol += (int)yyleng;
}

static void limpiar_valor(void)
{
    yylval.texto = NULL;
    yylval.entero = 0;
    yylval.nodo = NULL;
    yylval.linea = yylineno;
    yylval.col = yycol_token;
}

static int devolver(int token)
{
    limpiar_valor();
    return token;
}

static int devolver_texto(int token)
{
    yylval.texto = strdup(yytext);
    yylval.entero = 0;
    yylval.nodo = NULL;
    yylval.linea = yylineno;
    yylval.col = yycol_token;
    return token;
}

static void error_lexico(const char *mensaje)
{
    fprintf(stderr, "error lexico:%d:%d: %s '%s'\n",
            yylineno, yycol_token, mensaje, yytext);
    lexer_hubo_error = 1;
}
%}

%option noyywrap
%option nounput
%option noinput
%option nodefault
%option yylineno
%option pointer
%option 8bit
%option warn

%x COM_ML

%%

    /* ------------------------------------------------------------------ */
    /* 1. Espacios, tabuladores y saltos de linea. No producen token.     */
    /* ------------------------------------------------------------------ */

[ \t\r]+        { marcar(); }
\n              { yycol = 1; }

    /* ------------------------------------------------------------------ */
    /* 2. Comentario de varias lineas. Lectura abierta COM_ML.            */
    /*    La raya de cierre esta en las reglas <COM_ML> de mas abajo.     */
    /* ------------------------------------------------------------------ */

"—"|"–"|"―"    {
                    yycol_token = yycol;
                    yycol += 1;
                    com_ml_linea = yylineno;
                    if (traza_lexer) {
                        fprintf(stderr, "lectura abierta ComM en la linea %d\n",
                                com_ml_linea);
                    }
                    BEGIN(COM_ML);
                }

<COM_ML>"—"|<COM_ML>"–"|<COM_ML>"―"    {
                    yycol += 1;
                    if (traza_lexer) {
                        fprintf(stderr, "descartado ComM lineas %d-%d\n",
                                com_ml_linea, yylineno);
                    }
                    BEGIN(INITIAL);
                }

<COM_ML>\n      { yycol = 1; }
<COM_ML>.       { yycol += 1; }

<COM_ML><<EOF>> {
                    fprintf(stderr,
                            "error lexico:%d:%d: comentario de varias lineas "
                            "abierto en la linea %d sin terminar\n",
                            yylineno, yycol, com_ml_linea);
                    lexer_hubo_error = 1;
                    BEGIN(INITIAL);
                    return devolver(TK_ERROR);
                }

    /* ------------------------------------------------------------------ */
    /* 3. Comentario de una linea. De $ hasta el fin de linea, sin        */
    /*    consumir el salto: asi el conteo de lineas queda en la regla 1. */
    /* ------------------------------------------------------------------ */

\$[^\n]*        {
                    marcar();
                    if (traza_lexer) {
                        fprintf(stderr, "descartado Com1 linea %d\n", yylineno);
                    }
                }

    /* ------------------------------------------------------------------ */
    /* 4. Operadores de dos caracteres. Van antes que el de un caracter    */
    /*    de la misma familia, para que `==` no se parta en dos `=`.       */
    /*    TODO: literal de caracter que continua en la linea siguiente     */
    /*    despues de '\'.                                                  */
    /* ------------------------------------------------------------------ */

"=="            { marcar(); return devolver(TK_IGUAL); }
"!="            { marcar(); return devolver(TK_DISTINTO); }
"<="            { marcar(); return devolver(TK_MENOR_IGUAL); }
">="            { marcar(); return devolver(TK_MAYOR_IGUAL); }
"&&"            { marcar(); return devolver(TK_AND); }
"||"            { marcar(); return devolver(TK_OR); }
"+="            { marcar(); return devolver(TK_MAS_IGUAL); }
"-="            { marcar(); return devolver(TK_MENOS_IGUAL); }

    /* ------------------------------------------------------------------ */
    /* 5. Palabras de tipo, literales verdadero y falso, palabras de      */
    /*    funcion, terminar e #import, en ese orden.                       */
    /*    # y la palabra import son un solo token.                         */
    /* ------------------------------------------------------------------ */

"ent"           { marcar(); return devolver(TK_ENT); }
"real"          { marcar(); return devolver(TK_REAL); }
"doble"         { marcar(); return devolver(TK_DOBLE); }
"bool"          { marcar(); return devolver(TK_BOOL); }
"caract"        { marcar(); return devolver(TK_CARACT); }
"cad"           { marcar(); return devolver(TK_CAD); }
"vacio"         { marcar(); return devolver(TK_VACIO); }
"list"          { marcar(); return devolver(TK_LIST); }

"verdadero"     { marcar(); return devolver(TK_VERDADERO); }
"falso"         { marcar(); return devolver(TK_FALSO); }

"def"           { marcar(); return devolver(TK_DEF); }
"llamar"        { marcar(); return devolver(TK_LLAMAR); }
"retorno"       { marcar(); return devolver(TK_RETORNO); }
"terminar"      { marcar(); return devolver(TK_TERMINAR); }

"#import"[A-Za-z0-9_]+  {
                    marcar();
                    error_lexico("directiva mal formada");
                    return devolver_texto(TK_ERROR);
                }
"#import"       { marcar(); return devolver(TK_IMPORT); }

    /* ------------------------------------------------------------------ */
    /* 6. Palabras de control, antes del identificador. Miden lo mismo    */
    /*    que un nombre; si el identificador fuera primero, se las        */
    /*    comeria. Lista de la Propuesta de sintaxis de JAF, seccion 2.   */
    /* ------------------------------------------------------------------ */

"si"            { marcar(); return devolver(TK_SI); }
"sino"          { marcar(); return devolver(TK_SINO); }
"mientras"      { marcar(); return devolver(TK_MIENTRAS); }
"para"          { marcar(); return devolver(TK_PARA); }

    /* ------------------------------------------------------------------ */
    /* 7. Identificadores, literales, delimitadores y enteros.            */
    /*    El identificador no empieza con digito. El lenguaje distingue   */
    /*    mayusculas: Ent no es ent.                                      */
    /*    Literales enteros: decimales y hexadecimales (0x...).            */
    /*    TODO: literales reales, secuencias de escape y cadenas de       */
    /*    varias lineas.                                                  */
    /* ------------------------------------------------------------------ */

[A-Za-z_][A-Za-z0-9_]*  { marcar(); return devolver_texto(TK_IDENT); }

\"([^\"\\\n]|\\.)*\"    { marcar(); return devolver_texto(TK_CADENA); }
\"([^\"\\\n]|\\.)*      {
                    marcar();
                    error_lexico("cadena sin terminar");
                    return devolver_texto(TK_ERROR);
                }

\'([^\'\\\n]|\\.)\'     { marcar(); return devolver_texto(TK_CARACTER); }
\'([^\'\\\n]|\\.)*      {
                    marcar();
                    error_lexico("caracter sin terminar");
                    return devolver_texto(TK_ERROR);
                }

0[xX][0-9A-Fa-f]+|[0-9]+   {
                    char *fin = NULL;
                    unsigned long valor;

                    marcar();
                    errno = 0;
                    valor = strtoul(yytext, &fin, 0);
                    if (errno == ERANGE || valor > 0xFFFFFFFFUL) {
                        error_lexico("entero fuera de rango (32 bits)");
                    }
                    yylval.texto = strdup(yytext);
                    yylval.entero = (long)valor;
                    yylval.nodo = NULL;
                    yylval.linea = yylineno;
                    yylval.col = yycol_token;
                    return TK_ENTERO;
                }

"+"             { marcar(); return devolver('+'); }
"-"             { marcar(); return devolver('-'); }
"*"             { marcar(); return devolver('*'); }
"/"             { marcar(); return devolver('/'); }
"%"             { marcar(); return devolver('%'); }
"="             { marcar(); return devolver('='); }
"<"             { marcar(); return devolver('<'); }
">"             { marcar(); return devolver('>'); }
"!"             { marcar(); return devolver('!'); }

";"             { marcar(); return devolver(';'); }
","             { marcar(); return devolver(','); }
"("             { marcar(); return devolver('('); }
")"             { marcar(); return devolver(')'); }
"{"             { marcar(); return devolver('{'); }
"}"             { marcar(); return devolver('}'); }
"["             { marcar(); return devolver('['); }
"]"             { marcar(); return devolver(']'); }
"."             { marcar(); return devolver('.'); }
":"             { marcar(); return devolver(':'); }

.               {
                    marcar();
                    error_lexico("caracter no reconocido");
                    return devolver_texto(TK_ERROR);
                }

%%