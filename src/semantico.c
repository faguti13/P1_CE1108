#include "semantico.h"
#include "comun.h"
#include "isa.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*
 * Analisis semantico: tabla de simbolos con ambitos, validaciones de tipo,
 * asignacion de direcciones y reconocimiento de las intrinsecas.
 *
 * Formato de error:
 *   {ERROR codigo}:archivo:linea:columna:"mensaje"
 *
 * Codigos:
 *   10 simbolo no declarado        11 redeclaracion
 *   12 tipos incompatibles         13 argumentos de llamada incorrectos
 *   14 tipo o literal no soportado 15 retorno invalido
 *   16 dimension de lista invalida 17 indexacion invalida
 *   18 sentencia fuera de lugar    20 vacio usado como valor
 *   21 condicion no booleana       22 main invalida
 *   23 literal de lista invalido   24 funcion usada como variable
 *   25 constante invalida (argumento de intrinseca)
 */

/*Estado global del analisis semantico
  Contiene tabla de simbolos, cont. de errores y cant. de mem global utilizada
*/
Semantico sem;

static Simbolo *cur_func = NULL;       /* Funcion que se esta analizando actualmente.NULL cuando se analiza codigo de nivel superior*/
static long cursor = 0;                /* Cant de bytes ya usados en el marco de la funcion actual */
static int nivel = 0;                  /*Nivel de profundidad del ambito actual.0 corresponde a global*/

static Simbolo **tabla = NULL;         /*Tabla de simbolos visibles (pila de simbolos)*/
static int n_tabla = 0, cap_tabla = 0; /*Cantidad de simbolos almacenados y capacidad de la tabla*/
static int marcas[256];                /*Guarda posicion donde comienza cada ambito dentro de la tabla */
static Simbolo **funcs = NULL;         /*Lista de funciones declaradas en el programa*/
static int n_funcs = 0;

/* Emite un error con el formato entandar del compilador y aumenta el contador  de errores semanticos.
   sem.archivo ya esta puesto y fmt es un formato valido
   El mensaje recibido mediante fmt se contruye usando argumentos variables*/
static void error(const Nodo *n, int codigo, const char *fmt, ...)
{
    va_list ap;
    char msg[512];
    /*Se construye el mensaje de error apartir del formato recibido*/
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    /*Imprime error junto el archivo y la posicion del nodo*/
    fprintf(stderr, "{ERROR %d}:%s:%d:%d:\"%s\"\n", codigo, sem.archivo,
            n ? n->linea : 0, n ? n->col : 0, msg);
    sem.errores++;
}

/* Convierte un tipo interno del analizador semantico 
  en su representacion textual. 
*/
const char *ty_nombre(int ty)
{
    switch (ty) {
    case TY_ENT: return "ent";
    case TY_BOOL: return "bool";
    case TY_VACIO: return "vacio";
    case TY_LISTA_ENT: return "list ent";
    case TY_LISTA_BOOL: return "list bool";
    default: return "?";
    }
}

/*Determina si un tipo corresponde a una lista*/
static int es_lista(int ty) 
{ 
    return ty == TY_LISTA_ENT || ty == TY_LISTA_BOOL; 
}
/*Obtiene el tipo de los elementos de una lista.*/
static int elem_de(int ty) 
{
    return ty == TY_LISTA_ENT ? TY_ENT : TY_BOOL; 
}

/* Obtiene el hijo numero i de un nodo del AST.
  NOTA:hijo apunta al primero y sig al siguiente.
*/
static Nodo *hijo_n(const Nodo *n, int i)
{
    Nodo *h = n ? n->hijo : NULL;
    while (h && i-- > 0) {
        h = h->sig;
    }
    return h;
}

/*Cuenta la cantidad de hijos que tiene un nodo del AST*/
static int n_hijos(const Nodo *n)
{
    int k = 0;
    const Nodo *h = n ? n->hijo : NULL;
    for (; h; h = h->sig) {
        k++;
    }
    return k;
}
/*Stubs temporales para compilacion*/
static int tipo_de_nodo(const Nodo *t);
static int compatibles(int a, int b);
static Simbolo *buscar_intrinseca(const char *n);
static void registrar_funcion(Nodo *f);
static void analizar_funcion(Nodo *f);
static void sentencia(Nodo *n);
static int llamada(Nodo *n, int es_valor);
/* ---------------- Tabla de Simbolos ---------------- */

/*Crea un nuevo simbolo y lo registra en la lista global de simbolos
  que fueron encontrados durante el analisis.
  El simbolo  conserva info como su nombre, clase, ambito y posicion dentro del codigo fuente.
*/
static Simbolo *nuevo_simbolo(const char *nombre, Clase cl, const Nodo *pos)
{
    Simbolo *s = calloc(1, sizeof *s);
    s->nombre = strdup(nombre);
    s->clase = cl;
    s->nivel = nivel;
    s->linea = pos ? pos->linea : 0;
    s->col = pos ? pos->col : 0;
    /*Guarda la funcion que contiene el simbolo*/
    s->funcion = strdup(cur_func ? cur_func->nombre : "(global)");
    /*Registra el simbolo en la lista completa de simbolos*/
    sem.todos = realloc(sem.todos, (size_t)(sem.n_todos + 1) * sizeof(Simbolo *));
    sem.todos[sem.n_todos++] = s;
    return s;
}
/* Se entra en un nuevo ambito
   En marcas se guarda la posicion actual de la tabla para poder 
   saber luego que simbolos pertencen al ambito.
*/
static void push_scope(void)
{
    marcas[++nivel] = n_tabla;
}
/*Sale del ambito actual
  Los simbolos declarados dentro del ambito 
  ya no estan visibles para futuras busquedas
*/
static void pop_scope(void)
{
    n_tabla = marcas[nivel--];
}

/*Busqueda de funcion por nombre segun la lista se func registradas*/
static Simbolo *buscar_func(const char *nombre)
{
    int i;
    for (i = 0; i < n_funcs; i++) {
        if (strcmp(funcs[i]->nombre, nombre) == 0) {
            return funcs[i];
        }
    }
    return NULL;
}

/*Busca variable dentro de ambitos visibles
  Comienza desde ambito interno hacia externo 
*/
static Simbolo *buscar_var(const char *nombre)
{
    int i;
    for (i = n_tabla - 1; i >= 0; i--) {
        if (strcmp(tabla[i]->nombre, nombre) == 0) {
            return tabla[i];
        }
    }
    return NULL;
}

/*Se comprueba si un nombre ya fue declarado dentro del
  ambito actual(detectar redeclaraciones).se construye*/
static int declarada_en_nivel(const char *nombre)
{
    int i;
    for (i = n_tabla - 1; i >= marcas[nivel]; i--) {
        if (strcmp(tabla[i]->nombre, nombre) == 0) {
            return 1;
        }
    }
    return 0;
}
/*Agrega un simbolo a la tabla de simbolos visibles
  NOTA:La tabla aumenta su capacidad cuando se queda sin espacio.
*/
static void apilar(Simbolo *s)
{
    if (n_tabla == cap_tabla) {
        cap_tabla = cap_tabla ? cap_tabla * 2 : 64;
        tabla = realloc(tabla, (size_t)cap_tabla * sizeof(Simbolo *));
    }
    tabla[n_tabla++] = s;
}

/*Signa una posicion de memoria al simbolo
  Variables globales: offsets respecto al registro gp.
  Variables locales: offsets respecto al registro fp
*/
static void asignar(Simbolo *s, long bytes)
{
    if (!cur_func) {
        /*Asignacion de espacio para una variable global*/
        s->alm = AL_GLOBAL;
        s->offset = sem.bytes_globales;
        sem.bytes_globales += bytes;
    } else {
        /*Asignacion de espacio dentro del marco de la funcion*/
        s->alm = AL_LOCAL;
        cursor += bytes;
        s->offset = -cursor;
    }
}

/* ---------------- Constantes ---------------- */

/*Evalua una expresion constante entera durante el analisis semantico
 Salidas:
        -Retorna 1 si la expr se puede evaluar y se almacena el resultado en v.
        -Retorna 0 si expr no es una constante valida.
*/
int sem_const(const Nodo *n, long *v)
{
    long a, b;

    if (!n) {
        return 0;
    }
    /*Literal entero*/
    if (n->tipo == NODO_LITERAL && n->texto && (n->texto[0] >= '0' && n->texto[0] <= '9')) {
        *v = n->val;
        return 1;
    }
    /*Expresion aritmetica que se puede evaluar durante la compilacion*/
    if (n->tipo == NODO_EXP && n->texto) {
        const Nodo *l = n->hijo;
        const Nodo *r = l ? l->sig : NULL;
        /*Menos unario*/
        if (!strcmp(n->texto, "neg") && l && sem_const(l, &a)) {
            *v = -a;
            return 1;
        }
        /*Evalua operaciones binarias si ambos operandos son constantes.*/
        if (l && r && sem_const(l, &a) && sem_const(r, &b)) {
            switch (n->texto[0]) {
            case '+': *v = a + b; return 1;
            case '-': *v = a - b; return 1;
            case '*': *v = a * b; return 1;
            case '/': if (b) { *v = a / b; return 1; } return 0;
            case '%': if (b) { *v = a % b; return 1; } return 0;
            default: return 0;
            }
        }
    }
    return 0;
}

/*Ejecuta el analisis semantico del programa
 1-Registra funciones y despues analiza sus cuerpos y las sentencias del nivel superior.
 2-Devuelve cantidad total de errores semanticos encontrados*/
int sem_analizar(Nodo *raiz, const char *archivo)
{
    Nodo *cuerpo = hijo_n(raiz, 1);
    Nodo *h;
    Nodo *imps = hijo_n(raiz, 0);
    /*Reinicia los resultados del analisis*/
    memset(&sem, 0, sizeof sem);
    sem.archivo = archivo;
    /*Inicializa ambito global y contadores*/
    nivel = 0;
    marcas[0] = 0;
    n_tabla = 0;
    cur_func = NULL;
    /*Reserva espacio para temporales del codigo superior*/
    sem.marco_inicio = 4L * N_DERRAME;

    /*Registra las importaciones encontradas.
    NOTA:Se reconocen pero no se procesan*/
    for (h = imps ? imps->hijo : NULL; h; h = h->sig) {
        fprintf(stderr, "{AVISO 01}:%s:%d:%d:\"importacion '%s' reconocida pero no procesada en esta version\"\n",
                archivo, h->linea, h->col, h->texto ? h->texto : "");
        sem.avisos++;
    }
    /*Primer analisis:Registra todas las funciones*/
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        if (h->tipo == NODO_FUNCION) {
            registrar_funcion(h);
        }
    }
    /*Segundo analisis:Analiza contenido de cada funcion y las sentencias fuera de las funciones*/
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        if (h->tipo == NODO_FUNCION) {
            analizar_funcion(h);
        } else {
            sentencia(h);
        }
    }
    /*Ajusta tamano del area global para que sea multiplo de 4 bytes*/
    if (sem.bytes_globales % 4) {
        sem.bytes_globales += 4 - sem.bytes_globales % 4;
    }
    return sem.errores; /*Numero total de errores encontrados*/
}

/*Imprime la tabla de simbolos con info de cada variable, parametro y funcion
  Tambien muestra los tipos,ambitos, ubicaciones de memoria y el tamano del area global
*/
void sem_imprimir_tabla(FILE *f)
{
    int i;
    /*Encabezados de columnas*/
    fprintf(f, "%-18s %-8s %-12s %-5s %-14s %-9s %s\n", "nombre", "clase", "tipo", "amb.",
            "funcion", "linea:col", "direccion");
    /*Recorre todos los simbolos registrados durante el analisis*/
    for (i = 0; i < sem.n_todos; i++) {
        const Simbolo *s = sem.todos[i];
        char tipo[40], dir[64];
        /*Convierte clase del simbolo en un texto*/
        const char *cl = s->clase == CL_VAR ? "var" : s->clase == CL_PARAM ? "param" : "func";
        /*Construye el texto del tipo
          Para listas se incluyen sus dimensiones cuando se conocen
        */
        if (es_lista(s->ty)) {
            if (s->ndim > 0) {
                snprintf(tipo, sizeof tipo, "%s[%ld]%s", ty_nombre(s->ty), s->dim[0],
                         s->ndim == 2 ? "[..]" : "");
                if (s->ndim == 2) {
                    snprintf(tipo, sizeof tipo, "%s[%ld][%ld]", ty_nombre(s->ty), s->dim[0], s->dim[1]);
                }
            } else {
                /*Una lista puede tener dimensiones desconocidas*/
                snprintf(tipo, sizeof tipo, "%s (ref)", ty_nombre(s->ty));
            }
        /*Construye la info de ubicacion
        Para funciones se muestra cant de parametros y el marco
        para variables, su direccion relativa a gp o fp
        */
        } else {
            snprintf(tipo, sizeof tipo, "%s", ty_nombre(s->ty));
        }
        if (s->clase == CL_FUNC) {
            snprintf(dir, sizeof dir, "%d param, marco %ld B", s->nparams, s->marco);
        } else if (s->alm == AL_GLOBAL) {
            snprintf(dir, sizeof dir, "gp%+ld (0x%lX)", s->offset, (unsigned long)(MEM_GP + s->offset));
        } else {
            snprintf(dir, sizeof dir, "fp%+ld", s->offset);
        }
        /*Imprime los datos del simbolo en una fila*/
        fprintf(f, "%-18s %-8s %-12s %-5d %-14s %4d:%-4d %s\n", s->nombre, cl, tipo, s->nivel,
                s->funcion, s->linea, s->col, dir);
    }
    /*Muestra espacio total reservado para variables globales.*/
    fprintf(f, "area global: %ld bytes\n", sem.bytes_globales);
}

/*Stubs temporales para compilacion*/
static int tipo_de_nodo(const Nodo *t) { (void)t; return TY_ERR; }
static int compatibles(int a, int b) { (void)a; (void)b; return 0; }
static Simbolo *buscar_intrinseca(const char *n) { (void)n; return NULL; }
static void registrar_funcion(Nodo *f) { (void)f; }
static void analizar_funcion(Nodo *f) { (void)f; }
static void sentencia(Nodo *n) { (void)n; }
static int llamada(Nodo *n, int es_valor) { (void)n; (void)es_valor; return TY_ERR; }