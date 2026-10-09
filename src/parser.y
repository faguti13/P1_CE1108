%code requires {
#include "comun.h"
}

%code {
#include "ast.h"

#include <stdlib.h>
#include <string.h>

static ValorSemantico P(Nodo *nodo)
{
    ValorSemantico valor;
    memset(&valor, 0, sizeof valor);
    valor.nodo = nodo;
    return valor;
}

/* Quita las comillas de un literal de cadena.
 * TODO: interpretar secuencias de escape. */
static char *nombre_de_cadena(const char *lexema)
{
    size_t largo;
    char *nombre;

    if (!lexema) {
        return strdup("");
    }
    largo = strlen(lexema);
    if (largo >= 2 && lexema[0] == '"' && lexema[largo - 1] == '"') {
        nombre = malloc(largo - 1);
        if (!nombre) {
            return NULL;
        }
        memcpy(nombre, lexema + 1, largo - 2);
        nombre[largo - 2] = '\0';
        return nombre;
    }
    return strdup(lexema);
}
}

%define api.value.type {ValorSemantico}
%define parse.error verbose

%start programa

/*
 * Numeros identicos a src/tokens.h. Un %token de un caracter ('+', ';')
 * no hace falta: bison los acepta escritos entre comillas.
 *
 * UMENOS no lo devuelve el lexer. Solo le da precedencia al menos unario
 * para que `3 - - 1` reduzca como resta de un negativo, no como dos restas.
 */
%token TK_ENT          258 "ent"
%token TK_REAL         259 "real"
%token TK_DOBLE        260 "doble"
%token TK_BOOL         261 "bool"
%token TK_CARACT       262 "caract"
%token TK_CAD          263 "cad"
%token TK_VACIO        264 "vacio"
%token TK_LIST         265 "list"
%token TK_VERDADERO    266 "verdadero"
%token TK_FALSO        267 "falso"
%token TK_DEF          268 "def"
%token TK_LLAMAR       269 "llamar"
%token TK_RETORNO      270 "retorno"
%token TK_TERMINAR     271 "terminar"
%token TK_IMPORT       272 "#import"
%token TK_SI           273 "si"
%token TK_SINO         274 "sino"
%token TK_MIENTRAS     275 "mientras"
%token TK_PARA         276 "para"
%token TK_IDENT        277 "identificador"
%token TK_ENTERO       278 "entero"
%token TK_CADENA       279 "cadena"
%token TK_CARACTER     280 "caracter"
%token TK_IGUAL        281 "=="
%token TK_DISTINTO     282 "!="
%token TK_MENOR_IGUAL  283 "<="
%token TK_MAYOR_IGUAL  284 ">="
%token TK_AND          285 "&&"
%token TK_OR           286 "||"
%token TK_MAS_IGUAL    287 "+="
%token TK_MENOS_IGUAL  288 "-="
%token TK_ERROR        289 "error lexico"
%token UMENOS          290

/*
 * Precedencia y asociatividad de la Propuesta de sintaxis de JAF,
 * seccion 5.2. En bison la ultima declaracion aprieta mas. La
 * Documentacion del compilador, seccion 3.2, pide esta misma recursion
 * a la izquierda, declarada con %left y %right.
 */
%left TK_OR
%left TK_AND
%left TK_IGUAL TK_DISTINTO
%left '<' '>' TK_MENOR_IGUAL TK_MAYOR_IGUAL
%left '+' '-'
%left '*' '/' '%'
%precedence UMENOS '!'

%%

/*
 * programa ::= importacion programa | declaraciones
 * La directiva solo puede ir al inicio: importaciones se reduce antes
 * del cuerpo. Un #import despues de una funcion no entra en esta forma.
 */
programa
    : importaciones elementos
        {
            ast_raiz = nodo_programa($1.nodo, $2.nodo);
            $$ = P(ast_raiz);
        }
    ;

importaciones
    : %empty
        { $$ = P(NULL); }
    | importaciones importacion
        { $$ = P(nodo_enlazar($1.nodo, $2.nodo)); }
    ;

importacion
    : TK_IMPORT TK_CADENA ';'
        {
            char *nombre = nombre_de_cadena($2.texto);
            Nodo *nodo = nodo_nuevo(NODO_IMPORTACION, nombre);
            free(nombre);
            free($2.texto);
            $$ = P(nodo);
        }
    ;

elementos
    : %empty
        { $$ = P(NULL); }
    | elementos elemento
        { $$ = P(nodo_enlazar($1.nodo, $2.nodo)); }
    ;

elemento
    : funcion
    | sentencia
    ;

/* ------------------------------------------------------------------ */
/* Tipos. vacio solo es tipo de retorno: no sale de tipo_escalar,     */
/* asi que `vacio n = 0` no deriva. list va delante de un escalar.    */
/* ------------------------------------------------------------------ */

tipo
    : tipo_escalar
        { $$ = $1; }
    | TK_LIST tipo_escalar
        {
            Nodo *tipo = nodo_nuevo(NODO_TIPO, "list");
            nodo_hijo(tipo, $2.nodo);
            $$ = P(tipo);
        }
    ;

tipo_escalar
    : TK_ENT     { $$ = P(nodo_en(NODO_TIPO, "ent", $1.linea, $1.col)); }
    | TK_REAL    { $$ = P(nodo_en(NODO_TIPO, "real", $1.linea, $1.col)); }
    | TK_DOBLE   { $$ = P(nodo_en(NODO_TIPO, "doble", $1.linea, $1.col)); }
    | TK_BOOL    { $$ = P(nodo_en(NODO_TIPO, "bool", $1.linea, $1.col)); }
    | TK_CARACT  { $$ = P(nodo_en(NODO_TIPO, "caract", $1.linea, $1.col)); }
    | TK_CAD     { $$ = P(nodo_en(NODO_TIPO, "cad", $1.linea, $1.col)); }
    ;

tipo_retorno
    : tipo
        { $$ = $1; }
    | TK_VACIO
        { $$ = P(nodo_en(NODO_TIPO, "vacio", $1.linea, $1.col)); }
    ;

literal_bool
    : TK_VERDADERO { Nodo *n = nodo_en(NODO_LITERAL, "verdadero", $1.linea, $1.col); n->val = 1; $$ = P(n); }
    | TK_FALSO     { $$ = P(nodo_en(NODO_LITERAL, "falso", $1.linea, $1.col)); }
    ;

/* ------------------------------------------------------------------ */
/* Funcion, parametros, retorno, llamar y terminar.                   */
/* Los parametros y los argumentos recursan a la izquierda.           */
/* ------------------------------------------------------------------ */

funcion
    : tipo_retorno TK_DEF TK_IDENT '(' parametros ')' bloque
        {
            Nodo *nodo = nodo_funcion($1.nodo, $3.texto, $5.nodo, $7.nodo);
            nodo->linea = $3.linea;
            nodo->col = $3.col;
            free($3.texto);
            $$ = P(nodo);
        }
    ;

parametros
    : %empty
        { $$ = P(NULL); }
    | parametros_ne
        { $$ = $1; }
    ;

parametros_ne
    : parametro
        { $$ = $1; }
    | parametros_ne ',' parametro
        { $$ = P(nodo_enlazar($1.nodo, $3.nodo)); }
    ;

parametro
    : tipo TK_IDENT
        {
            Nodo *nodo = nodo_parametro($1.nodo, $2.texto);
            nodo->linea = $2.linea;
            nodo->col = $2.col;
            free($2.texto);
            $$ = P(nodo);
        }
    ;

sentencia_retorno
    : TK_RETORNO '(' expresion ')' ';'
        {
            Nodo *nodo = nodo_nuevo(NODO_RETORNO, NULL);
            nodo_hijo(nodo, $3.nodo);
            $$ = P(nodo);
        }
    | TK_RETORNO ';'
        { $$ = P(nodo_nuevo(NODO_RETORNO, NULL)); }
    ;

sentencia_llamar
    : TK_LLAMAR TK_IDENT '(' argumentos ')' ';'
        {
            Nodo *nodo = nodo_llamada($2.texto, $4.nodo);
            nodo->linea = $2.linea;
            nodo->col = $2.col;
            free($2.texto);
            $$ = P(nodo);
        }
    ;

expr_llamar
    : TK_LLAMAR TK_IDENT '(' argumentos ')'
        {
            Nodo *nodo = nodo_llamada($2.texto, $4.nodo);
            nodo->linea = $2.linea;
            nodo->col = $2.col;
            free($2.texto);
            $$ = P(nodo);
        }
    ;

argumentos
    : %empty
        { $$ = P(NULL); }
    | argumentos_ne
        { $$ = $1; }
    ;

argumentos_ne
    : expresion
        { $$ = $1; }
    | argumentos_ne ',' expresion
        { $$ = P(nodo_enlazar($1.nodo, $3.nodo)); }
    ;

sentencia_terminar
    : TK_TERMINAR ';'
        { $$ = P(nodo_nuevo(NODO_TERMINAR, NULL)); }
    ;

/*
 * Un bloque es el codigo entre llaves. La visibilidad no se decide
 * aqui: una variable declarada adentro no se ve afuera.
 * TODO: tabla de simbolos con ambito de bloque.
 */
bloque
    : '{' sentencias '}'
        { $$ = P(nodo_bloque($2.nodo)); }
    ;

sentencias
    : %empty
        { $$ = P(NULL); }
    | sentencias sentencia
        { $$ = P(nodo_enlazar($1.nodo, $2.nodo)); }
    ;

sentencia
    : sentencia_retorno
    | sentencia_llamar
    | sentencia_terminar
    | declaracion
    | asignacion
    | sentencia_si
    | sentencia_mientras
    | sentencia_para
    ;

/*
 * Declaracion de un escalar o de una lista. Formas (Propuesta de
 * sintaxis de JAF, secciones 4.1 y 4.2):
 *   ent x;   ent x = e;   list ent v[10];   list ent m[3][4];
 *   list ent v = [1, 2, 3];   list ent m = [[1, 2], [3, 4]];
 * Las listas generan NODO_DECL_LISTA con hijos: tipo, dimensiones, valor.
 */
declaracion
    : tipo TK_IDENT ';'
        {
            Nodo *nodo = nodo_en(NODO_DECLARACION, $2.texto, $2.linea, $2.col);
            nodo_hijo(nodo, $1.nodo);
            free($2.texto);
            $$ = P(nodo);
        }
    | tipo TK_IDENT '=' expresion ';'
        {
            Nodo *nodo = nodo_en(NODO_DECLARACION, $2.texto, $2.linea, $2.col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, $4.nodo);
            free($2.texto);
            $$ = P(nodo);
        }
    | tipo TK_IDENT dimensiones ';'
        {
            Nodo *nodo = nodo_en(NODO_DECL_LISTA, $2.texto, $2.linea, $2.col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, nodo_lista("dimensiones", $3.nodo));
            nodo_hijo(nodo, nodo_vacio("valor"));
            free($2.texto);
            $$ = P(nodo);
        }
    | tipo TK_IDENT '=' literal_lista ';'
        {
            Nodo *nodo = nodo_en(NODO_DECL_LISTA, $2.texto, $2.linea, $2.col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, nodo_vacio("dimensiones"));
            nodo_hijo(nodo, $4.nodo);
            free($2.texto);
            $$ = P(nodo);
        }
    ;

dimensiones
    : '[' expresion ']'
        { $$ = $2; }
    | '[' expresion ']' '[' expresion ']'
        { $$ = P(nodo_enlazar($2.nodo, $5.nodo)); }
    ;

literal_lista
    : '[' elementos_lista ']'
        {
            Nodo *nodo = nodo_nuevo(NODO_LISTA_LIT, NULL);
            nodo_hijos_cadena(nodo, $2.nodo);
            $$ = P(nodo);
        }
    ;

elementos_lista
    : elemento_lista
        { $$ = $1; }
    | elementos_lista ',' elemento_lista
        { $$ = P(nodo_enlazar($1.nodo, $3.nodo)); }
    ;

elemento_lista
    : expresion
        { $$ = $1; }
    | literal_lista
        { $$ = $1; }
    ;

/* Acceso por indice: nombre[i] o nombre[i][j]. */
indice
    : TK_IDENT '[' expresion ']'
        {
            Nodo *nodo = nodo_en(NODO_INDICE, $1.texto, $1.linea, $1.col);
            nodo_hijo(nodo, $3.nodo);
            free($1.texto);
            $$ = P(nodo);
        }
    | TK_IDENT '[' expresion ']' '[' expresion ']'
        {
            Nodo *nodo = nodo_en(NODO_INDICE, $1.texto, $1.linea, $1.col);
            nodo_hijo(nodo, $3.nodo);
            nodo_hijo(nodo, $6.nodo);
            free($1.texto);
            $$ = P(nodo);
        }
    ;

destino
    : TK_IDENT
        {
            Nodo *nodo = nodo_en(NODO_IDENT, $1.texto, $1.linea, $1.col);
            free($1.texto);
            $$ = P(nodo);
        }
    | indice
        { $$ = $1; }
    ;

/*
 * Asignacion simple y compuesta (+=, -=). El destino es un
 * identificador o un indice. La forma sin ';' se reutiliza en para.
 */
asignacion_sp
    : destino '=' expresion
        {
            Nodo *nodo = nodo_en(NODO_ASIGNACION, "=", $1.nodo->linea, $1.nodo->col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, $3.nodo);
            $$ = P(nodo);
        }
    | destino TK_MAS_IGUAL expresion
        {
            Nodo *nodo = nodo_en(NODO_ASIGNACION, "+=", $1.nodo->linea, $1.nodo->col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, $3.nodo);
            $$ = P(nodo);
        }
    | destino TK_MENOS_IGUAL expresion
        {
            Nodo *nodo = nodo_en(NODO_ASIGNACION, "-=", $1.nodo->linea, $1.nodo->col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, $3.nodo);
            $$ = P(nodo);
        }
    ;

asignacion
    : asignacion_sp ';'
        { $$ = $1; }
    ;

/*
 * si / sino si / sino. El cuerpo es un bloque, asi que el sino
 * pertenece al si cuya llave acaba de cerrarse. Propuesta de sintaxis
 * de JAF, seccion 6.2.
 */
sentencia_si
    : TK_SI '(' expresion ')' bloque si_resto
        {
            Nodo *nodo = nodo_nuevo(NODO_SI, NULL);
            nodo_hijo(nodo, $3.nodo);
            nodo_hijo(nodo, $5.nodo);
            nodo_hijo(nodo, $6.nodo);
            $$ = P(nodo);
        }
    ;

si_resto
    : %empty
        { $$ = P(NULL); }
    | TK_SINO TK_SI '(' expresion ')' bloque si_resto
        {
            Nodo *nodo = nodo_nuevo(NODO_SI, NULL);
            nodo_hijo(nodo, $4.nodo);
            nodo_hijo(nodo, $6.nodo);
            nodo_hijo(nodo, $7.nodo);
            $$ = P(nodo);
        }
    | TK_SINO bloque
        { $$ = $2; }
    ;

/*
 * mientras (condicion) bloque. Propuesta de sintaxis de JAF, seccion 6.3.
 * para ( inicio ; condicion ; actualizacion ) bloque, seccion 6.4.
 */
sentencia_mientras
    : TK_MIENTRAS '(' expresion ')' bloque
        {
            Nodo *nodo = nodo_nuevo(NODO_MIENTRAS, NULL);
            nodo_hijo(nodo, $3.nodo);
            nodo_hijo(nodo, $5.nodo);
            $$ = P(nodo);
        }
    ;

sentencia_para
    : TK_PARA '(' para_inicio ';' expresion ';' para_actualiza ')' bloque
        {
            Nodo *nodo = nodo_nuevo(NODO_PARA, NULL);
            nodo_hijo(nodo, $3.nodo);
            nodo_hijo(nodo, $5.nodo);
            nodo_hijo(nodo, $7.nodo);
            nodo_hijo(nodo, $9.nodo);
            $$ = P(nodo);
        }
    ;

para_inicio
    : %empty
        { $$ = P(nodo_vacio("inicio")); }
    | tipo TK_IDENT '=' expresion
        {
            Nodo *nodo = nodo_en(NODO_DECLARACION, $2.texto, $2.linea, $2.col);
            nodo_hijo(nodo, $1.nodo);
            nodo_hijo(nodo, $4.nodo);
            free($2.texto);
            $$ = P(nodo);
        }
    | asignacion_sp
        { $$ = $1; }
    ;

para_actualiza
    : %empty
        { $$ = P(nodo_vacio("actualizacion")); }
    | asignacion_sp
        { $$ = $1; }
    ;

/*
 * Expresiones aritmeticas, relacionales y logicas. Los parentesis no
 * crean nodo: solo cambian el orden de reduccion.
 * TODO: acceso por indice (`nombre[i]`) y literal de lista.
 */
expresion
    : primario
        { $$ = $1; }
    | expresion TK_OR expresion
        { $$ = P(nodo_binario("||", $1.nodo, $3.nodo)); }
    | expresion TK_AND expresion
        { $$ = P(nodo_binario("&&", $1.nodo, $3.nodo)); }
    | expresion TK_IGUAL expresion
        { $$ = P(nodo_binario("==", $1.nodo, $3.nodo)); }
    | expresion TK_DISTINTO expresion
        { $$ = P(nodo_binario("!=", $1.nodo, $3.nodo)); }
    | expresion '<' expresion
        { $$ = P(nodo_binario("<", $1.nodo, $3.nodo)); }
    | expresion '>' expresion
        { $$ = P(nodo_binario(">", $1.nodo, $3.nodo)); }
    | expresion TK_MENOR_IGUAL expresion
        { $$ = P(nodo_binario("<=", $1.nodo, $3.nodo)); }
    | expresion TK_MAYOR_IGUAL expresion
        { $$ = P(nodo_binario(">=", $1.nodo, $3.nodo)); }
    | expresion '+' expresion
        { $$ = P(nodo_binario("+", $1.nodo, $3.nodo)); }
    | expresion '-' expresion
        { $$ = P(nodo_binario("-", $1.nodo, $3.nodo)); }
    | expresion '*' expresion
        { $$ = P(nodo_binario("*", $1.nodo, $3.nodo)); }
    | expresion '/' expresion
        { $$ = P(nodo_binario("/", $1.nodo, $3.nodo)); }
    | expresion '%' expresion
        { $$ = P(nodo_binario("%", $1.nodo, $3.nodo)); }
    | '!' expresion
        { $$ = P(nodo_unario("!", $2.nodo)); }
    | '-' expresion %prec UMENOS
        { $$ = P(nodo_unario("neg", $2.nodo)); }
    ;

primario
    : literal_bool
        { $$ = $1; }
    | TK_IDENT
        {
            Nodo *nodo = nodo_en(NODO_IDENT, $1.texto, $1.linea, $1.col);
            free($1.texto);
            $$ = P(nodo);
        }
    | indice
        { $$ = $1; }
    | TK_ENTERO
        {
            Nodo *nodo = nodo_en(NODO_LITERAL, $1.texto, $1.linea, $1.col);
            nodo->val = $1.entero;
            free($1.texto);
            $$ = P(nodo);
        }
    | TK_CADENA
        {
            Nodo *nodo = nodo_en(NODO_LITERAL, $1.texto, $1.linea, $1.col);
            free($1.texto);
            $$ = P(nodo);
        }
    | TK_CARACTER
        {
            Nodo *nodo = nodo_en(NODO_LITERAL, $1.texto, $1.linea, $1.col);
            free($1.texto);
            $$ = P(nodo);
        }
    | expr_llamar
        { $$ = $1; }
    | '(' expresion ')'
        { $$ = $2; }
    ;

%%
