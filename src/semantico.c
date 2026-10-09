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

/* ---------------- Tipos ---------------- */

/*Obtiene el tipo semantico correspondiente a un nodo
 que representa una declaracion de tipo
 Los tipos definidos fueron:ent,bool,list ent y list bool.
*/

static int tipo_de_nodo(const Nodo *t)
{
    const char *x = t->texto ? t->texto : "";

    if (!strcmp(x, "ent")) return TY_ENT;
    if (!strcmp(x, "bool")) return TY_BOOL;
    if (!strcmp(x, "vacio")) return TY_VACIO;
    if (!strcmp(x, "list")) {
        const Nodo *e = t->hijo;
        if (e && e->texto && !strcmp(e->texto, "ent")) return TY_LISTA_ENT;
        if (e && e->texto && !strcmp(e->texto, "bool")) return TY_LISTA_BOOL;
        error(t, 14, "lista de '%s' no soportada (solo list ent y list bool)", e && e->texto ? e->texto : "?");
        return TY_ERR;
    }
    error(t, 14, "tipo '%s' no soportado en esta version (solo ent, bool y listas)", x);
    return TY_ERR;
}

/*Se comprueba si dos tipos se consideran compatibles
 TY_ERR es compatible con todos para evitar cadenas de errores secundarios por un error semantico
*/
static int compatibles(int a, int b)
{
    return a == TY_ERR || b == TY_ERR || a == b;
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


/* ---------------- Expresiones ---------------- */

static int expr(Nodo *n); /*Analiza una expresion y retorna su tipo semantico*/
static int llamada(Nodo *n, int es_valor); /*Analiza una llamada a funcion o funcion intrinseca*/

/*Analiza el acceso a un elemento de una lista.
  Se verifica que:
             - la variable exista
             - sea realmente una lista
             -los indices sean enteros
             - la cant de indices coincida con las dimensiones */
static int expr_indice(Nodo *n)
{
    Simbolo *s = buscar_var(n->texto);
    int k = n_hijos(n);
    Nodo *h;

    n->ty = TY_ERR;
    /*Comprueba que la variabl exista*/
    if (!s) {
        if (buscar_func(n->texto)) {
            error(n, 24, "'%s' es una funcion, no una lista", n->texto);
        } else {
            error(n, 10, "'%s' no esta declarado", n->texto);
        }
        /*Analiza los indices para detectar posibles errores (auqnue la variable no exista)*/
        for (h = n->hijo; h; h = h->sig) {
            expr(h);
        }
        return TY_ERR;
    }
    n->sim = s;
    /*Todos los indices deben ser enteros*/
    for (h = n->hijo; h; h = h->sig) {
        int t = expr(h);
        if (!compatibles(t, TY_ENT)) {
            error(h, 17, "el indice debe ser ent, no %s", ty_nombre(t));
        }
    }
    /*Se revisa que el simbolo corresponda a una lista*/
    if (!es_lista(s->ty)) {
        error(n, 17, "'%s' no es una lista (tipo %s)", n->texto, ty_nombre(s->ty));
        return TY_ERR;
    }
    /*Comprueba que se utiliza la cant. correcta de indices*/
    if (s->ndim > 0 && k != s->ndim) {
        error(n, 17, "'%s' tiene %d dimension(es) y se indexa con %d", n->texto, s->ndim, k);
        return TY_ERR;
    }
    /*Resultado de indexar una lista es el tipo de sus elementos*/
    n->ty = elem_de(s->ty);
    return n->ty;
}
/*Analiza una expr del AST y determina su tipo
  El tipo se guarda en n->ty para usarlo en otras partes del A.Sem
*/
static int expr(Nodo *n)
{
    int a, b;
    const char *op;

    if (!n) {
        return TY_ERR;
    }
    switch (n->tipo) {
        /* ---------------- Literales ---------------- */
    case NODO_LITERAL:
        /*Literales verdadero y falso son del tipo bool*/
        if (!strcmp(n->texto, "verdadero") || !strcmp(n->texto, "falso")) {
            n->val = !strcmp(n->texto, "verdadero");
            return n->ty = TY_BOOL;
        }
        /*Si se comienza con un digito se considera entero*/
        if (n->texto[0] >= '0' && n->texto[0] <= '9') {
            return n->ty = TY_ENT;
        }
        /*Tipos de literales no soportados*/
        error(n, 14, "literal %s no soportado (cadenas y caracteres quedan fuera de esta version)", n->texto);
        return n->ty = TY_ERR;

        /* ---------------- Identificadores ---------------- */
    case NODO_IDENT: {
        Simbolo *s = buscar_var(n->texto);
        /* Busca el identificador en la tabla se simbolos.
           Si no esta como variable, se revisa si es el nombre de una funcion
        */
        if (!s) {
            if (buscar_func(n->texto)) {
                error(n, 24, "'%s' es una funcion; se invoca con llamar", n->texto);
            } else {
                error(n, 10, "'%s' no esta declarado", n->texto);
            }
            return n->ty = TY_ERR;
        }
        /*Se relaciona el nodo del AST con su simbolo*/
        n->sim = s;
        /*Retorno:Tipo de expr igual al tipo del simbolo*/
        return n->ty = s->ty;
    }
    /*Acceso a lista*/
    case NODO_INDICE:
        return expr_indice(n);
    /*Llamada a funciones*/
    case NODO_LLAMADA:
        return n->ty = llamada(n, 1);
    /*Operadores*/
    case NODO_EXP:
        op = n->texto;
        /*OL:NOT*/
        if (!strcmp(op, "!")) {
            a = expr(n->hijo);
            /*Solo se aplica a un expr de tipo bool*/
            if (!compatibles(a, TY_BOOL)) {
                error(n, 12, "'!' espera bool, no %s", ty_nombre(a));
            }
            return n->ty = TY_BOOL;
        }/*Menos unario*/
        if (!strcmp(op, "neg")) {
            a = expr(n->hijo);
            /*Solo se acepta enteros*/
            if (!compatibles(a, TY_ENT)) {
                error(n, 12, "el menos unario espera ent, no %s", ty_nombre(a));
            }
            return n->ty = TY_ENT;
        }
        /*Para operadores bianrios se analizan los dos operandos*/
        a = expr(n->hijo);
        b = expr(hijo_n(n, 1));
        /*Operadores logicos AND Y OR*/
        if (!strcmp(op, "&&") || !strcmp(op, "||")) {
            if (!compatibles(a, TY_BOOL) || !compatibles(b, TY_BOOL)) {
                error(n, 12, "'%s' espera bool y bool, no %s y %s", op, ty_nombre(a), ty_nombre(b));
            }
            return n->ty = TY_BOOL;
        }
        /*Operadores de igualdad
         No se pueden comparar lista ni valores vacios
        */
        if (!strcmp(op, "==") || !strcmp(op, "!=")) {
            if (!compatibles(a, b) || es_lista(a) || a == TY_VACIO) {
                error(n, 12, "'%s' no compara %s con %s", op, ty_nombre(a), ty_nombre(b));
            }
            return n->ty = TY_BOOL;
        }
        /*Operadores relaciona.Solo entre valores enteros*/
        if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, "<=") || !strcmp(op, ">=")) {
            if (!compatibles(a, TY_ENT) || !compatibles(b, TY_ENT)) {
                error(n, 12, "'%s' espera ent y ent, no %s y %s", op, ty_nombre(a), ty_nombre(b));
            }
            return n->ty = TY_BOOL;
        }
        /*Si no es ninguno de los pasados, ent es una op aritmetica
          Estas solo trabajan con enteros.
        */
        if (!compatibles(a, TY_ENT) || !compatibles(b, TY_ENT)) {
            error(n, 12, "'%s' espera ent y ent, no %s y %s", op, ty_nombre(a), ty_nombre(b));
        }
        return n->ty = TY_ENT;
    default:
        /*Caso:El nodo no representa una expr valida*/
        error(n, 18, "expresion no valida en este punto");
        return n->ty = TY_ERR;
    }
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
static Simbolo *buscar_intrinseca(const char *n) { (void)n; return NULL; }
static void registrar_funcion(Nodo *f) { (void)f; }
static void analizar_funcion(Nodo *f) { (void)f; }
static void sentencia(Nodo *n) { (void)n; }
static int llamada(Nodo *n, int es_valor) { (void)n; (void)es_valor; return TY_ERR; }