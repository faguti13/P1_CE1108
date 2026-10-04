#ifndef TOKENS_H
#define TOKENS_H

#include "comun.h"

/*
 * Codigos de token del lenguaje JAF.
 *
 * Los valores empiezan en 258 para no pisar los tokens de un caracter
 * (';', '+', '{', ...), que el lexer devuelve como su codigo ASCII.
 * parser.y repite estos numeros en cada %token: si se cambia uno,
 * hay que cambiar el otro. Un desajuste se ve al parsear, porque el
 * parser no reconoceria lo que el lexer devuelve.
 *
 * Nombres internos, los de la Documentacion del compilador, seccion 2.1:
 *   DataE DataR DataD DataB DataC DataSTR DataV DataL
 *   BoolV BoolF  DEF CALL RTN BRK IMP
 */

/* Tipos. real, doble, caract y cad se reconocen aqui. El ISA no tiene
 * punto flotante, y la Documentacion del compilador, seccion 2.1, deja
 * para la fase semantica el rechazo de caract y cad. */
#define TK_ENT          258   /* DataE   ent      */
#define TK_REAL         259   /* DataR   real     */
#define TK_DOBLE        260   /* DataD   doble    */
#define TK_BOOL         261   /* DataB   bool     */
#define TK_CARACT       262   /* DataC   caract   */
#define TK_CAD          263   /* DataSTR cad      */
#define TK_VACIO        264   /* DataV   vacio    */
#define TK_LIST         265   /* DataL   list     */

/* Literales booleanos. */
#define TK_VERDADERO    266   /* BoolV */
#define TK_FALSO        267   /* BoolF */

/* Funciones, finalizacion e importacion. */
#define TK_DEF          268   /* DEF  def      */
#define TK_LLAMAR       269   /* CALL llamar   */
#define TK_RETORNO      270   /* RTN  retorno  */
#define TK_TERMINAR     271   /* BRK  terminar */
#define TK_IMPORT       272   /* IMP  #import  */

/* Palabras de control. En el lexer van antes del identificador. */
#define TK_SI           273   /* IF  si       */
#define TK_SINO         274   /* ELS sino     */
#define TK_MIENTRAS     275   /* WHL mientras */
#define TK_PARA         276   /* FOR para     */

/* Identificadores y literales. */
#define TK_IDENT        277   /* IdVAR  */
#define TK_ENTERO       278   /* LitEnt */
#define TK_CADENA       279   /* LitCad, comillas incluidas */
#define TK_CARACTER     280   /* LitCar, comillas incluidas */

/* Operadores de dos caracteres. */
#define TK_IGUAL        281   /* == */
#define TK_DISTINTO     282   /* != */
#define TK_MENOR_IGUAL  283   /* <= */
#define TK_MAYOR_IGUAL  284   /* >= */
#define TK_AND          285   /* && */
#define TK_OR           286   /* || */
#define TK_MAS_IGUAL    287   /* += */
#define TK_MENOS_IGUAL  288   /* -= */

/* Caracter que no pertenece a ningun token, o comentario sin cerrar. */
#define TK_ERROR        289

/* Nombre interno para el volcado. Los ASCII los resuelve tokens.c. */
const char *nombre_token(int token);

#endif
