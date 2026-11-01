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

void ide_emitir_json(char **archivos, int n);

#endif
