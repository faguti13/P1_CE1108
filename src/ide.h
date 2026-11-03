#ifndef IDE_H
#define IDE_H

/*
 * Backend del prototipo de IDE (CE1108, Proyecto 2).
 * La ventana esta en ide/jaf_ide.py; este modulo imprime JSON a stdout.
 *
 * TODO (equipo): rellenar diagnostics, fixes, completions y clones usando
 * lexer, parser y semantico. No implementar la UI aqui.
 *
 * Contrato JSON (un objeto por ejecucion de `jaf --ide a.oly b.oly ...`):
 * {
 *   "diagnostics": [ {"file","line","col","code","msg","severity"} ],
 *   "fixes":        [ {"file","line","col","kind":"insert"|"replace","text"} ],
 *   "completions":  [ {"label","kind","detail"} ],
 *   "clones":       [ {"type":1|2, "a":{"file","line","end"}, "b":{"file","line","end"}} ]
 * }
 */

typedef struct {
    /* Estructura para guardar un error detectado por el compilador 
    *Los datos son dados por el lexer,parser o semnatico segun corresponda
    */
    char *archivo;    //Nombre del archvo donde ocurrió el error
    int linea;        //Número de línea
    int col;          //Número de columna
    int codigo;       //El número de error (10, 11, 12, 02, etc.)
    char *mensaje;    //Descripción del erro
    char *severidad;  //Si es "Error" o "Warning" 
} DiagError;

/*Registra un error en la lista global.Se llama desde semantico o main*/
void ide_registrar_error(const char *archivo, int linea, int col, int codigo, const char *mensaje, const char *severidad);
/*Limpia la lista de errores para cada ejecución del IDE*/
void ide_limpiar_errores(void);

void ide_emitir_json(char **archivos, int n);

#endif
