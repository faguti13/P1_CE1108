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

/* ---------------- intrinsecas y llamadas ---------------- */
/*Tabla de funciones intrinsecas del lenguaje.
*/
static const struct {
    const char *nombre; /*Nombre utilizado en el programa*/
    Intr id;            /*Identificador interno de la intrinseca*/
    int ret;            /*Tipo de retorno*/
    int n;              /*Cantidad de argumentos*/
} intrinsecas[] = {
    {"autenticar", IN_AUTENTICAR, TY_VACIO, 1},
    {"cerrar_sesion", IN_CERRAR_SESION, TY_VACIO, 0},
    {"leer_estado", IN_LEER_ESTADO, TY_ENT, 0},
    {"cargar_llave", IN_CARGAR_LLAVE, TY_VACIO, 3},
    {"ronda_feistel", IN_RONDA_FEISTEL, TY_VACIO, 4},
    {"feistel_cifrar", IN_FEISTEL_CIFRAR, TY_VACIO, 3},
    {"feistel_descifrar", IN_FEISTEL_DESCIFRAR, TY_VACIO, 3},
    {"leer_mem", IN_LEER_MEM, TY_ENT, 1},
    {"escribir_mem", IN_ESCRIBIR_MEM, TY_VACIO, 2},
};

/*Busca una funcion dentro de la tabla de intrinsecas
  El simbolo se crea solo la primera vez que se solicita.
 Para no crealo varias veces, se usa una mini cache.
  */
static Simbolo *buscar_intrinseca(const char *nombre)
{
    static Simbolo *cache[sizeof intrinsecas / sizeof intrinsecas[0]];
    size_t i;
    for (i = 0; i < sizeof intrinsecas / sizeof intrinsecas[0]; i++) {
        if (!strcmp(intrinsecas[i].nombre, nombre)) {
            /*Si todavia no existe se crea su simbolo*/
            if (!cache[i]) {
                cache[i] = calloc(1, sizeof(Simbolo));
                cache[i]->nombre = strdup(nombre);
                cache[i]->clase = CL_INTRINSECA;
                cache[i]->ty = intrinsecas[i].ret;
                cache[i]->nparams = intrinsecas[i].n;
                cache[i]->intr = intrinsecas[i].id;
                cache[i]->funcion = strdup("(intrinseca)");
            }
            return cache[i];
        }
    }
    return NULL;
}
/*
  Verifica si el nodo es un lvalue entero modificable
  (variable entera o elemento entero de lista).
 */

static int es_lvalue_ent(Nodo *a)
{
    return (a->tipo == NODO_IDENT && a->sim && a->sim->clase != CL_FUNC && a->sim->ty == TY_ENT) ||
           (a->tipo == NODO_INDICE && a->ty == TY_ENT);
}
/* Analiza una llamada a una funcion o intrinseca*/
static int llamada(Nodo *n, int es_valor)
/*Es valor indica si la llamada se esta usando como una expr que necesita un valor*/
{
    Simbolo *f = buscar_intrinseca(n->texto);
    Nodo *arg;
    int i, na = n_hijos(n);
    /*Primero se busca intrinsecas
      Luego en la func definidas por el programa
    */
    if (!f) {
        f = buscar_func(n->texto);
    }
    /*La funcion no existe*/
    if (!f) {
        error(n, 10, "la funcion '%s' no esta definida", n->texto);
        /*Se analizan todos los argumentos para detectar otros errores semanticos*/
        for (arg = n->hijo; arg; arg = arg->sig) {
            expr(arg);
        }
        return TY_ERR;
    }
    n->sim = f; /*Se relaciona llamada con el simbolo de la funcion*/
    /*Se verifica que la cant de argumentos sea igual a la cant de parametros esperados*/
    if (na != f->nparams) {
        error(n, 13, "'%s' espera %d argumento(s) y recibe %d", n->texto, f->nparams, na);
        /*Se analizan los argumentos aunque haya error*/
        for (arg = n->hijo; arg; arg = arg->sig) {
            expr(arg);
        }
        return f->ty;
    }
    /*Se analiza cada arg y se comprueba que sea compatible con su respectivo parametro*/
    for (i = 0, arg = n->hijo; arg; arg = arg->sig, i++) {
        int t = expr(arg);
        if (f->clase == CL_INTRINSECA) {
            /* Intrisecas con reglas especiales para sus argumentos*/
            switch (f->intr) {
            /*Requieren que las dos primeras variables sean enteros y los demas arg constantes entre 0 y 3*/
            case IN_RONDA_FEISTEL:
            case IN_FEISTEL_CIFRAR:
            case IN_FEISTEL_DESCIFRAR:
                if (i < 2) {
                    if (!es_lvalue_ent(arg)) {
                        error(arg, 13, "el argumento %d de '%s' debe ser una variable ent", i + 1, f->nombre);
                    }
                } else {
                    long v;
                    if (!sem_const(arg, &v) || v < 0 || v > 3) {
                        error(arg, 25, "el argumento %d de '%s' debe ser una constante entre 0 y 3", i + 1, f->nombre);
                    }
                }
                break;
            /*Recibe un entero como primer arg, los demas son const entre 0 y 3*/
            case IN_CARGAR_LLAVE:
                if (i >= 1) {
                    long v;
                    if (!sem_const(arg, &v) || v < 0 || v > 3) {
                        error(arg, 25, "el argumento %d de '%s' debe ser una constante entre 0 y 3", i + 1, f->nombre);
                    }
                } else if (!compatibles(t, TY_ENT)) {
                    error(arg, 13, "el argumento 1 de '%s' debe ser ent", f->nombre);
                }
                break;
            /*Para las demas intrinsecas se exige un arg entero*/
            default:
                if (!compatibles(t, TY_ENT)) {
                    error(arg, 13, "el argumento %d de '%s' debe ser ent, no %s", i + 1, f->nombre, ty_nombre(t));
                }
                break;
            }
        } else {
            /*Funcion normal:Tipo de arg debe corresponder con tipo de parametro*/
            if (!compatibles(t, f->ptipo[i])) {
                error(arg, 13, "el argumento %d de '%s' es %s y se esperaba %s", i + 1, f->nombre,
                      ty_nombre(t), ty_nombre(f->ptipo[i]));
            /*Los parametros que son listas reciben el nombre de la lista, no una expresion cualquiera.*/
            } else if (es_lista(f->ptipo[i]) && arg->tipo != NODO_IDENT) {

                error(arg, 13, "el argumento %d de '%s' debe ser el nombre de una lista", i + 1, f->nombre);
            }
        }
    }
    /*Uns expr de tipo vacio no se puede usar como expr que espera obtener un valor.*/
    if (es_valor && f->ty == TY_VACIO) {
        error(n, 20, "'%s' no devuelve valor (vacio) y se usa en una expresion", n->texto);
        return TY_ERR;
    }
    return f->ty;
}

/* ---------------- Sentencias ---------------- */

static void sentencia(Nodo *n); 

/*Analiza la condicion de una estructura de control
 Se requiere que las condiciones de si,para y mientras deben ser booleanas
*/
static void condicion(Nodo *c)
{
    int t = expr(c);
    if (!compatibles(t, TY_BOOL)) {
        error(c, 21, "la condicion debe ser bool, no %s", ty_nombre(t));
    }
}

/*Analiza declaracion de variable escalar*/
static void declarar_escalar(Nodo *n)
{
    Nodo *tn = hijo_n(n, 0);
    Nodo *ini = hijo_n(n, 1);
    int ty = tipo_de_nodo(tn);
    Simbolo *s;

    /*Las listas no siguen esta forma.Cuentan con un propio*/
    if (es_lista(ty)) {
        error(n, 12, "'%s' es una lista: se declara con tamano o con [..]", n->texto);
        ty = TY_ERR;
    }
    /*Si hay inicializados se comprueba el tipo*/
    if (ini) {
        int t = expr(ini);
        if (!compatibles(t, ty)) {
            error(ini, 12, "no se puede inicializar %s '%s' con %s", ty_nombre(ty), n->texto, ty_nombre(t));
        }
    }
    /*No se puede redeclarar un simbolo en el mismo ambito.(Tampoco se pueden usar nombres reservados de funciones)*/
    if (declarada_en_nivel(n->texto) || buscar_func(n->texto) || buscar_intrinseca(n->texto)) {
        error(n, 11, "'%s' ya esta declarado", n->texto);
        return;
    }
    /*Se crea el simbolo de la variable*/
    s = nuevo_simbolo(n->texto, CL_VAR, n);
    s->ty = ty;
    asignar(s, 4);
    n->sim = s;
    apilar(s);
}

/*Determina forma de un literal de lista
-Calcula dimensiones y verifica elementos con tipo correcto,
 que no se mezclen sublistas y escalares,filas del mismo tamano
 que no haya mas de dos dimensiones*/
static int forma_lista(Nodo *lit, int elem_ty, long *filas, long *cols)
{
    Nodo *h;
    int sublistas = 0, escalares = 0;
    long n = 0, w = -1;

    for (h = lit->hijo; h; h = h->sig) {
        n++;
        /*Si el elemento es otra lista, se trata como una posible segunda dimension*/
        if (h->tipo == NODO_LISTA_LIT) {
            long f2, c2;
            sublistas++;
            if (!forma_lista(h, elem_ty, &f2, &c2)) {
                return 0;
            }
            /*No se permiten mas de dos dimensiones*/
            if (c2 != 0) {
                error(h, 23, "las listas admiten a lo sumo 2 dimensiones");
                return 0;
            }
            /*Todas las filas deben tener la misma cant de elementos*/
            if (w >= 0 && w != f2) {
                error(h, 23, "las filas de la lista tienen distinta longitud (%ld y %ld)", w, f2);
                return 0;
            }
            w = f2;
        } else {
            /*El elemento debe tener el tipo esperado*/
            int t = expr(h);
            escalares++;
            if (!compatibles(t, elem_ty)) {
                error(h, 12, "elemento %s en una lista de %s", ty_nombre(t), ty_nombre(elem_ty));
            }
        }
    }
    /*Una lista no puede mezclar elementos individuales con sublistas*/
    if (sublistas && escalares) {
        error(lit, 23, "no se pueden mezclar elementos y sublistas");
        return 0;
    }
    /*Si contiene sublistas, es una lista de 2 dimensiones
     Si solo tiene elementos, es de 1 dimension
    */
    if (sublistas) {
        *filas = n;
        *cols = w;
    } else {
        *filas = n;
        *cols = 0;
    }
    return 1;
}

/*Se analiza la declaracion de una lista
 Se obtiene tipo, dimensiones y literal para inicializarla(si existe)
 Crea luego el simbolo y reserva el espacio correspondiente en memoria.
*/
static void declarar_lista(Nodo *n)
{
    Nodo *tn = hijo_n(n, 0);
    Nodo *dims = hijo_n(n, 1);
    Nodo *val = hijo_n(n, 2);
    int ty = tipo_de_nodo(tn);
    Simbolo *s;
    long d[2] = {0, 0};
    int nd = 0;
    long bytes;
    /*Verifica que el tipo declarado corresponda a una lista*/
    if (!es_lista(ty) && ty != TY_ERR) {
        error(n, 12, "'%s' tiene tipo %s: solo una lista admite tamano o [..]", n->texto, ty_nombre(ty));
        ty = TY_ERR;
    }
    /*  Si hay una literal se calcula forma y se cormpueba tipo de sus elementos*/
    if (val && val->tipo == NODO_LISTA_LIT) {
        long f, c;
        if (ty != TY_ERR && forma_lista(val, elem_de(ty), &f, &c)) {
            /*No se permiten listas literales vacias*/
            if (f == 0) {
                error(val, 16, "una lista literal no puede estar vacia");
            }
            /*Limite del tamano de las listas literales*/
            if (f * (c > 0 ? c : 1) > 2000) {
                error(val, 16, "una lista literal admite a lo sumo 2000 elementos");
            }
            /*Calcular dimensiones detectadas*/
            if (c > 0) {
                nd = 2;
                d[0] = f;
                d[1] = c;
            } else {
                nd = 1;
                d[0] = f;
                d[1] = 1;
            }
        } else {
            /*En caso de literal invalido usar dimensiones de respaldo*/
            nd = 1;
            d[0] = d[1] = 1;
        }
    } else if (dims) {
        /*Sino hay literal, se leen las dimensiones indicadas en la declaracion
        Cada dimension debe ser una constante entera positiva
        */
        Nodo *x;
        for (x = dims->hijo; x && nd < 2; x = x->sig, nd++) {
            long v;
            if (!sem_const(x, &v)) {
                expr(x);
                error(x, 16, "el tamano de la lista debe ser una constante entera");
                v = 1;
            } else if (v <= 0 || v > 65536) {
                error(x, 16, "tamano de lista invalido (%ld): debe estar entre 1 y 65536", v);
                v = 1;
            }
            d[nd] = v;
        }
        /*Una lista de una dimension tiene una segunda dimension de 1. */
        if (nd == 1) {
            d[1] = 1;
        }
        /*Se verifica limite total de elementos*/
        if (nd == 2 && d[0] * d[1] > 65536) {
            error(n, 16, "la lista tiene mas de 65536 elementos");
            d[0] = d[1] = 1;
        }
    }
    /*Se evita declarar un nombre ya utilizado*/
    if (declarada_en_nivel(n->texto) || buscar_func(n->texto) || buscar_intrinseca(n->texto)) {
        error(n, 11, "'%s' ya esta declarado", n->texto);
        return;
    }
    /*Crea simbolo y almacena tipo y dimensiones*/
    s = nuevo_simbolo(n->texto, CL_VAR, n);
    s->ty = ty;
    s->ndim = nd ? nd : 1;
    s->dim[0] = d[0] ? d[0] : 1;
    s->dim[1] = d[1] ? d[1] : 1;
    /*Se reservan 8 bytes para la info de la lista y 4 bytes por cada elemento*/
    bytes = 8 + 4 * s->dim[0] * s->dim[1];
    asignar(s, bytes);
    /*Relaciona el nodo con el simbolo y lo agrega al ambito*/
    n->sim = s;
    apilar(s);
}
/*Analiza una asignacion y verifica que el tipo del destino
  sea compatible con el tipo del valor asignado
*/
static void asignacion(Nodo *n)
{
    Nodo *d = hijo_n(n, 0);
    Nodo *v = hijo_n(n, 1);
    /*Obtiene el tipo de destino y el de la expr asignada*/
    int td = expr(d);
    int tv = expr(v);
    /*No se puede asignar una lista completa de un solo
      Sus elementos se asignan de manera individual*/
    if (d->tipo == NODO_IDENT && d->sim && es_lista(d->sim->ty)) {
        error(n, 12, "no se asigna una lista completa; asigne elemento a elemento");
        return;
    }
    /*Comprueba que ambos tipos sean compatibles*/
    if (!compatibles(td, tv)) {
        error(n, 12, "no se puede asignar %s a %s", ty_nombre(tv), ty_nombre(td));
    }
    /*Operadores de asignacion compuesta solo se permiten con variables enteras*/
    if (strcmp(n->texto, "=") != 0 && !compatibles(td, TY_ENT)) {
        error(n, 12, "'%s' solo aplica a ent", n->texto);
    }
}
/*Analiza una sentencia segun el tipo de nodo del AST
 Procesa bloques,declaraciones, asignaciones, estruc de control, retornos y llamadas de funcion*/
static void sentencia(Nodo *n)
{
    Nodo *h;

    if (!n) {
        return;
    }
    switch (n->tipo) {
    case NODO_BLOQUE:
        /*Cada bloque crea su propio ambito
         Cuando se termina su analisis se recupera el ambito anterior
        */
        push_scope();
        for (h = n->hijo; h; h = h->sig) {
            sentencia(h);
        }
        pop_scope();
        break;
    case NODO_DECLARACION:
        declarar_escalar(n); /*Analiza declaracion de variable escalar*/
        break;
    case NODO_DECL_LISTA:
        declarar_lista(n);/*Analiza declaracion de lista*/
        break;
    case NODO_ASIGNACION:
        asignacion(n); /*Comprueba los tipos involucrados en una asignacion*/
        break;
    case NODO_SI:
        /*Comprueba la condicion y analiza las ramas del si.*/
        condicion(hijo_n(n, 0));
        sentencia(hijo_n(n, 1));
        sentencia(hijo_n(n, 2));
        break;
    case NODO_MIENTRAS:
        /*Condicion booleana, luego se analiza el cuerpo*/
        condicion(hijo_n(n, 0));
        sentencia(hijo_n(n, 1));
        break;
    case NODO_PARA:
        /*Crea su propio ambito
        Incluye:inicializacion,condicion,actualizacion y el cuerpo del ciclo*/
        push_scope();
        sentencia(hijo_n(n, 0));
        condicion(hijo_n(n, 1));
        sentencia(hijo_n(n, 2));
        sentencia(hijo_n(n, 3));
        pop_scope();
        break;
    case NODO_LISTA:/* Espacio vacio dentro de un para*/
        break;
    case NODO_RETORNO: {
        Nodo *e = hijo_n(n, 0);
        /*Retorno:Debe estar dentro de una funcion*/
        if (!cur_func) {
            error(n, 15, "retorno fuera de una funcion");
            if (e) expr(e);
        /*Una funcion vacio no debe devolver un valor */
        } else if (cur_func->ty == TY_VACIO) {
            if (e) {
                expr(e);
                error(n, 15, "la funcion '%s' es vacio y no devuelve valor", cur_func->nombre);
            }
        /*Una funcion que devuelve un valor lo debe incluir en la sentencia de retorno*/
        } else if (!e) {
            error(n, 15, "la funcion '%s' debe devolver %s", cur_func->nombre, ty_nombre(cur_func->ty));
        /*Comrpueba compatibilidad entre valor retornado y tipo de retorno declarado en la funcion*/
        } else {
            int t = expr(e);
            if (!compatibles(t, cur_func->ty)) {
                error(n, 15, "'%s' devuelve %s y se retorna %s", cur_func->nombre,
                      ty_nombre(cur_func->ty), ty_nombre(t));
            }
        }
        break;
    }
    case NODO_LLAMADA: /*Se ejecuta como sentencia, sin exigir valor de retorno*/
        llamada(n, 0);
        break;
    case NODO_TERMINAR:/*No requiere comprobaciones semanticas */
        break;
    default: /*Nodo no corresponde a una sentencia valida en este punto*/
        error(n, 18, "sentencia no valida en este punto");
        break;
    }
}

/* ---------------- Funciones ---------------- */

/*Registra la definicion de una funcion en la tabla de simbolos 
  Se registran primero todas las funciones para que una funcion pueda llamar a otra mas adelante*/
static void registrar_funcion(Nodo *f)
{
    Nodo *tipo = hijo_n(f, 0);
    Nodo *params = hijo_n(f, 1);
    Nodo *p;
    Simbolo *s;
    int i = 0;
    /*Evitar duplicar nombres de funciones o utilizar nombres intrinsecos*/
    if (buscar_func(f->texto) || buscar_intrinseca(f->texto)) {
        error(f, 11, "la funcion '%s' ya esta definida", f->texto);
        return;
    }
    /*Crea el tipo de la funcion y determina su tipo de retorno*/
    s = nuevo_simbolo(f->texto, CL_FUNC, f);
    s->ty = tipo_de_nodo(tipo);
    /*NOTA:De momento las funciones no pueden devolver listas*/
    if (es_lista(s->ty)) {
        error(f, 14, "una funcion no puede devolver una lista");
        s->ty = TY_ERR;
    }
    /*Registra tipo y la dimension de cada parametro.Maximo 16 parametros*/
    for (p = params ? params->hijo : NULL; p; p = p->sig, i++) {
        int t;
        if (i >= 16) {
            error(p, 13, "una funcion admite a lo sumo 16 parametros");
            break;
        }
        t = tipo_de_nodo(hijo_n(p, 0));
        s->ptipo[i] = t;
        s->pndim[i] = es_lista(t) ? -1 : 0; /*Para listas -1 indica dimension desconocida*/
    }
    /*Guarda la cantidad de parametros y el nodo de definicion*/
    s->nparams = i;
    s->def = f;
    f->sim = s;
    /*Agrega la funcion a la lista de funciones registradas*/
    funcs = realloc(funcs, (size_t)(n_funcs + 1) * sizeof(Simbolo *));
    funcs[n_funcs++] = s;
    /*Funcion principal:Debe llamarse main, no recibir parametros y tener tipo de retorno vacio*/
    if (!strcmp(f->texto, "main") && (s->nparams != 0 || s->ty != TY_VACIO)) {
        error(f, 22, "main debe ser 'vacio def main()' sin parametros");
    }
}
/*Analiza el contenido de una funcion ya registrada
 Registra parametros, analiza las sentencias de su 
 cuerpo y calcula el tamano del marco usado por la funcion*/
static void analizar_funcion(Nodo *f)
{
    Simbolo *s = f->sim;
    Nodo *params = hijo_n(f, 1);
    Nodo *cuerpo = hijo_n(f, 2);
    Nodo *p, *h;
    int i = 0;
    /*Si la funcion no se registro bien,no se analiza*/
    if (!s) {
        return;
    }
    cur_func = s;    /*Indica la funcion que se esta analizando*/
    /*Los parametros pertenecen al ambito de la funcion*/
    push_scope();
    for (p = params ? params->hijo : NULL; p && i < s->nparams; p = p->sig, i++) {
        Simbolo *ps;
        /*Los parametros no pueden repetirse en el mismo ambito*/
        if (declarada_en_nivel(p->texto)) {
            error(p, 11, "el parametro '%s' esta repetido", p->texto);
            continue;
        }
        /*Crea y configura el simbolo del parametro*/
        ps = nuevo_simbolo(p->texto, CL_PARAM, p);
        ps->ty = s->ptipo[i];
        ps->ndim = s->pndim[i];
        /*Los parametros se almacenan en posiciones relativas a fp*/
        ps->alm = AL_PARAM;
        ps->offset = -4L * (i + 1);
        p->sim = ps;
        apilar(ps);
    }
    /*Inicializa el cursor del marco
    Se consideran los parametros y el espacio reservador para la funcion*/
    cursor = 4L * s->nparams + 8;
    /*Analiza todas las sentencias del cuerpo de la funcion*/
    for (h = cuerpo ? cuerpo->hijo : NULL; h; h = h->sig) {
        sentencia(h);
    }
    /*Guarda el tama;o alcanzado y la zona reservada para los 
    temporales que deben derramarse durante las llamadas*/
    s->zona_derrame = cursor;
    s->marco = cursor + 4L * N_DERRAME;
    /*Al finalizar se restaura el ambito y se sale de la funcion*/
    pop_scope();
    cur_func = NULL;
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
