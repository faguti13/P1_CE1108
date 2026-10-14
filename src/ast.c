#include "ast.h"
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Nodo *ast_raiz = NULL;

Nodo *nodo_nuevo(TipoNodo tipo, const char *texto)
{
    Nodo *nodo = calloc(1, sizeof(*nodo));
    if (!nodo) {
        return NULL;
    }
    nodo->tipo = tipo;
    nodo->linea = yylineno;
    nodo->col = yycol_token;
    if (texto) {
        nodo->texto = strdup(texto);
    }
    return nodo;
}

Nodo *nodo_en(TipoNodo tipo, const char *texto, int linea, int col)
{
    Nodo *nodo = nodo_nuevo(tipo, texto);
    if (nodo) {
        nodo->linea = linea;
        nodo->col = col;
    }
    return nodo;
}

Nodo *nodo_vacio(const char *etiqueta)
{
    return nodo_nuevo(NODO_LISTA, etiqueta);
}

Nodo *nodo_enlazar(Nodo *lista, Nodo *elem)
{
    Nodo *cursor;

    if (!elem) {
        return lista;
    }
    if (!lista) {
        return elem;
    }
    cursor = lista;
    while (cursor->sig) {
        cursor = cursor->sig;
    }
    cursor->sig = elem;
    return lista;
}

void nodo_hijo(Nodo *padre, Nodo *hijo)
{
    if (!padre || !hijo) {
        return;
    }
    if (!padre->hijo) {
        padre->hijo = hijo;
        return;
    }
    (void)nodo_enlazar(padre->hijo, hijo);
}

void nodo_hijos_cadena(Nodo *padre, Nodo *cadena)
{
    while (cadena) {
        Nodo *siguiente = cadena->sig;
        cadena->sig = NULL;
        nodo_hijo(padre, cadena);
        cadena = siguiente;
    }
}

Nodo *nodo_lista(const char *etiqueta, Nodo *cadena)
{
    Nodo *lista = nodo_nuevo(NODO_LISTA, etiqueta);
    nodo_hijos_cadena(lista, cadena);
    return lista;
}

Nodo *nodo_programa(Nodo *importaciones, Nodo *cuerpo)
{
    Nodo *programa = nodo_nuevo(NODO_PROGRAMA, NULL);
    nodo_hijo(programa, nodo_lista("importaciones", importaciones));
    nodo_hijo(programa, nodo_lista("cuerpo", cuerpo));
    return programa;
}

Nodo *nodo_funcion(Nodo *tipo, const char *nombre, Nodo *parametros, Nodo *cuerpo)
{
    Nodo *funcion = nodo_nuevo(NODO_FUNCION, nombre);
    nodo_hijo(funcion, tipo);
    nodo_hijo(funcion, nodo_lista("parametros", parametros));
    nodo_hijo(funcion, cuerpo);
    return funcion;
}

Nodo *nodo_parametro(Nodo *tipo, const char *nombre)
{
    Nodo *parametro = nodo_nuevo(NODO_PARAMETRO, nombre);
    nodo_hijo(parametro, tipo);
    return parametro;
}

Nodo *nodo_llamada(const char *nombre, Nodo *argumentos)
{
    Nodo *llamada = nodo_nuevo(NODO_LLAMADA, nombre);
    nodo_hijos_cadena(llamada, argumentos);
    return llamada;
}

Nodo *nodo_bloque(Nodo *sentencias)
{
    Nodo *bloque = nodo_nuevo(NODO_BLOQUE, NULL);
    nodo_hijos_cadena(bloque, sentencias);
    return bloque;
}

Nodo *nodo_binario(const char *op, Nodo *izq, Nodo *der)
{
    Nodo *expresion = nodo_nuevo(NODO_EXP, op);
    nodo_hijo(expresion, izq);
    nodo_hijo(expresion, der);
    return expresion;
}

Nodo *nodo_unario(const char *op, Nodo *expr)
{
    Nodo *expresion = nodo_nuevo(NODO_EXP, op);
    nodo_hijo(expresion, expr);
    return expresion;
}

static const char *nombre_nodo(TipoNodo tipo)
{
    switch (tipo) {
    case NODO_PROGRAMA:     return "Programa";
    case NODO_LISTA:        return "Lista";
    case NODO_IMPORTACION:  return "Importacion";
    case NODO_FUNCION:      return "Funcion";
    case NODO_PARAMETRO:    return "Parametro";
    case NODO_TIPO:         return "Tipo";
    case NODO_BLOQUE:       return "Bloque";
    case NODO_RETORNO:      return "Retorno";
    case NODO_LLAMADA:      return "Llamada";
    case NODO_TERMINAR:     return "Terminar";
    case NODO_SI:           return "Si";
    case NODO_MIENTRAS:     return "Mientras";
    case NODO_DECLARACION:  return "Declaracion";
    case NODO_ASIGNACION:   return "Asignacion";
    case NODO_EXP:          return "Exp";
    case NODO_IDENT:        return "Ident";
    case NODO_LITERAL:      return "Literal";
    case NODO_PARA:         return "Para";
    case NODO_INDICE:       return "Indice";
    case NODO_LISTA_LIT:    return "ListaLiteral";
    case NODO_DECL_LISTA:   return "DeclaracionLista";
    }
    return "Nodo";
}

static const Nodo *hijo_en(const Nodo *nodo, int indice)
{
    const Nodo *hijo = nodo ? nodo->hijo : NULL;
    while (hijo && indice > 0) {
        hijo = hijo->sig;
        indice--;
    }
    return hijo;
}

static void imprimir_rol(const char *rol, const Nodo *nodo, int nivel)
{
    printf("%*s%s:\n", nivel, "", rol);
    if (!nodo) {
        printf("%*s(ninguno)\n", nivel + 2, "");
        return;
    }
    nodo_imprimir(nodo, nivel + 2);
}

void nodo_imprimir(const Nodo *nodo, int nivel)
{
    const Nodo *hijo;

    if (!nodo) {
        return;
    }

    printf("%*s%s", nivel, "", nombre_nodo(nodo->tipo));
    if (nodo->texto && nodo->texto[0] != '\0') {
        printf(" %s", nodo->texto);
    }
    printf("\n");

    /* Funcion, Parametro, Retorno y Llamada rotulan cada hijo como en
     * la Documentacion del compilador, seccion 3.2. Si, mientras,
     * declaracion y asignacion usan el mismo criterio. */
    switch (nodo->tipo) {
    case NODO_FUNCION:
        imprimir_rol("tipo", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("parametros", hijo_en(nodo, 1), nivel + 2);
        imprimir_rol("cuerpo", hijo_en(nodo, 2), nivel + 2);
        return;
    case NODO_PARAMETRO:
        imprimir_rol("tipo", hijo_en(nodo, 0), nivel + 2);
        return;
    case NODO_RETORNO:
        imprimir_rol("expresion", hijo_en(nodo, 0), nivel + 2);
        return;
    case NODO_LLAMADA:
        printf("%*sargumentos:\n", nivel + 2, "");
        if (!nodo->hijo) {
            printf("%*s(ninguno)\n", nivel + 4, "");
            return;
        }
        for (hijo = nodo->hijo; hijo; hijo = hijo->sig) {
            nodo_imprimir(hijo, nivel + 4);
        }
        return;
    case NODO_SI:
        imprimir_rol("condicion", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("entonces", hijo_en(nodo, 1), nivel + 2);
        imprimir_rol("sino", hijo_en(nodo, 2), nivel + 2);
        return;
    case NODO_MIENTRAS:
        imprimir_rol("condicion", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("cuerpo", hijo_en(nodo, 1), nivel + 2);
        return;
    case NODO_DECLARACION:
        imprimir_rol("tipo", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("valor", hijo_en(nodo, 1), nivel + 2);
        return;
    case NODO_ASIGNACION:
        imprimir_rol("destino", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("valor", hijo_en(nodo, 1), nivel + 2);
        return;
    case NODO_PARA:
        imprimir_rol("inicio", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("condicion", hijo_en(nodo, 1), nivel + 2);
        imprimir_rol("actualizacion", hijo_en(nodo, 2), nivel + 2);
        imprimir_rol("cuerpo", hijo_en(nodo, 3), nivel + 2);
        return;
    case NODO_DECL_LISTA:
        imprimir_rol("tipo", hijo_en(nodo, 0), nivel + 2);
        imprimir_rol("dimensiones", hijo_en(nodo, 1), nivel + 2);
        imprimir_rol("valor", hijo_en(nodo, 2), nivel + 2);
        return;
    default:
        break;
    }

    for (hijo = nodo->hijo; hijo; hijo = hijo->sig) {
        nodo_imprimir(hijo, nivel + 2);
    }
}

static void json_indent(FILE *f, int nivel)
{
    fprintf(f, "%*s", nivel * 2, "");
}

static void json_str(FILE *f, const char *s)
{
    fputc('"', f);
    if (s) {
        for (; *s; s++) {
            unsigned char c = (unsigned char)*s;
            if (c == '"' || c == '\\') {
                fprintf(f, "\\%c", c);
            } else if (c == '\n') {
                fputs("\\n", f);
            } else if (c == '\r') {
                fputs("\\r", f);
            } else if (c == '\t') {
                fputs("\\t", f);
            } else if (c < 0x20) {
                fprintf(f, "\\u%04x", c);
            } else {
                fputc(c, f);
            }
        }
    }
    fputc('"', f);
}

/* Emite un objeto. `rol` es el papel del nodo respecto de su padre
 * (condicion, cuerpo, ...); NULL si el padre no le pone nombre. */
static void json_nodo(FILE *f, const Nodo *nodo, const char *rol, int nivel)
{
    const Nodo *hijo;
    int primero = 1;
    const char *roles[4];
    int nroles = 0;
    int i;

    if (!nodo) {
        fputs("null", f);
        return;
    }
    fputs("{\n", f);
    if (rol) {
        json_indent(f, nivel + 1);
        fputs("\"rol\": ", f);
        json_str(f, rol);
        fputs(",\n", f);
    }
    json_indent(f, nivel + 1);
    fputs("\"tipo\": ", f);
    json_str(f, nombre_nodo(nodo->tipo));
    if (nodo->texto && nodo->texto[0] != '\0') {
        fputs(",\n", f);
        json_indent(f, nivel + 1);
        fputs("\"texto\": ", f);
        json_str(f, nodo->texto);
    }
    if (nodo->tipo == NODO_LITERAL) {
        fprintf(f, ",\n");
        json_indent(f, nivel + 1);
        fprintf(f, "\"valor\": %ld", nodo->val);
    }
    fprintf(f, ",\n");
    json_indent(f, nivel + 1);
    fprintf(f, "\"linea\": %d,\n", nodo->linea);
    json_indent(f, nivel + 1);
    fprintf(f, "\"col\": %d,\n", nodo->col);
    json_indent(f, nivel + 1);
    fputs("\"hijos\": [", f);

    switch (nodo->tipo) {
    case NODO_FUNCION:
        roles[0] = "tipo"; roles[1] = "parametros"; roles[2] = "cuerpo";
        nroles = 3;
        break;
    case NODO_PARAMETRO:
        roles[0] = "tipo";
        nroles = 1;
        break;
    case NODO_RETORNO:
        roles[0] = "expresion";
        nroles = 1;
        break;
    case NODO_SI:
        roles[0] = "condicion"; roles[1] = "entonces"; roles[2] = "sino";
        nroles = 3;
        break;
    case NODO_MIENTRAS:
        roles[0] = "condicion"; roles[1] = "cuerpo";
        nroles = 2;
        break;
    case NODO_DECLARACION:
        roles[0] = "tipo"; roles[1] = "valor";
        nroles = 2;
        break;
    case NODO_ASIGNACION:
        roles[0] = "destino"; roles[1] = "valor";
        nroles = 2;
        break;
    case NODO_PARA:
        roles[0] = "inicio"; roles[1] = "condicion";
        roles[2] = "actualizacion"; roles[3] = "cuerpo";
        nroles = 4;
        break;
    case NODO_DECL_LISTA:
        roles[0] = "tipo"; roles[1] = "dimensiones"; roles[2] = "valor";
        nroles = 3;
        break;
    default:
        break;
    }

    if (nroles) {
        for (i = 0; i < nroles; i++) {
            hijo = hijo_en(nodo, i);
            if (!hijo) {
                continue;
            }
            if (!primero) {
                fputc(',', f);
            }
            primero = 0;
            fputc('\n', f);
            json_indent(f, nivel + 2);
            json_nodo(f, hijo, roles[i], nivel + 2);
        }
    } else {
        for (hijo = nodo->hijo; hijo; hijo = hijo->sig) {
            if (!primero) {
                fputc(',', f);
            }
            primero = 0;
            fputc('\n', f);
            json_indent(f, nivel + 2);
            json_nodo(f, hijo, nodo->tipo == NODO_LLAMADA ? "argumento" : NULL, nivel + 2);
        }
    }
    if (!primero) {
        fputc('\n', f);
        json_indent(f, nivel + 1);
    }
    fputs("]\n", f);
    json_indent(f, nivel);
    fputc('}', f);
}

void nodo_json(FILE *f, const Nodo *nodo)
{
    json_nodo(f, nodo, NULL, 0);
    fputc('\n', f);
}

void nodo_liberar(Nodo *nodo)
{
    if (!nodo) {
        return;
    }
    nodo_liberar(nodo->hijo);
    nodo_liberar(nodo->sig);
    free(nodo->texto);
    nodo->texto = NULL;
    free(nodo);
}