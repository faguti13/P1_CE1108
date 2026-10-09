#ifndef AST_H
#define AST_H

/*
 * Arbol sintactico.
 *
 * Cada reduccion de bison construye un nodo y le cuelga lo ya reducido.
 * Las listas (parametros, argumentos, sentencias) se enlazan por `sig`,
 * de izquierda a derecha: el parser es LALR y esa recursion es a la
 * izquierda, como describe la Documentacion del compilador, seccion 3.2.
 *
 * Lo que la gramatica aun no cubre esta marcado con TODO en parser.y.
 */

typedef enum TipoNodo {
    NODO_PROGRAMA,
    NODO_LISTA,
    NODO_IMPORTACION,
    NODO_FUNCION,
    NODO_PARAMETRO,
    NODO_TIPO,
    NODO_BLOQUE,
    NODO_RETORNO,
    NODO_LLAMADA,
    NODO_TERMINAR,
    NODO_SI,
    NODO_MIENTRAS,
    NODO_DECLARACION,
    NODO_ASIGNACION,
    NODO_EXP,
    NODO_IDENT,
    NODO_LITERAL
} TipoNodo;

typedef struct Nodo {
    TipoNodo tipo;
    char *texto;
    struct Nodo *hijo;   /* primer hijo */
    struct Nodo *sig;    /* siguiente hermano */
} Nodo;

extern Nodo *ast_raiz;

Nodo *nodo_nuevo(TipoNodo tipo, const char *texto);
Nodo *nodo_enlazar(Nodo *lista, Nodo *elem);
void nodo_hijo(Nodo *padre, Nodo *hijo);
void nodo_hijos_cadena(Nodo *padre, Nodo *cadena);

Nodo *nodo_lista(const char *etiqueta, Nodo *cadena);
Nodo *nodo_programa(Nodo *importaciones, Nodo *cuerpo);
Nodo *nodo_funcion(Nodo *tipo, const char *nombre, Nodo *parametros, Nodo *cuerpo);
Nodo *nodo_parametro(Nodo *tipo, const char *nombre);
Nodo *nodo_llamada(const char *nombre, Nodo *argumentos);
Nodo *nodo_bloque(Nodo *sentencias);
Nodo *nodo_binario(const char *op, Nodo *izq, Nodo *der);
Nodo *nodo_unario(const char *op, Nodo *expr);

void nodo_imprimir(const Nodo *nodo, int nivel);
void nodo_liberar(Nodo *nodo);

#endif
