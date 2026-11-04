#!/usr/bin/env python3
"""IDE JAF — proceso propio con Qt (estilo VS Code Dark+).

No es extension de VS Code. Logica en C: `jaf --ide` (src/ide.c).
Requiere:  sudo apt install python3-pyqt6   o   pip install PyQt6
"""
from __future__ import annotations

import fcntl
import json
import os
import pty
import re
import shutil
import signal
import struct
import subprocess
import sys
import termios
from dataclasses import dataclass

try:
    from PyQt6.QtCore import Qt, QSize, QSocketNotifier, QTimer
    from PyQt6.QtGui import (
        QAction, QColor, QFont, QKeySequence, QPainter, QTextCharFormat,
        QTextCursor, QTextFormat, QSyntaxHighlighter, QStandardItem, QStandardItemModel,
    )
    from PyQt6.QtWidgets import (
        QApplication, QFileDialog, QHBoxLayout, QInputDialog, QLabel,
        QMainWindow, QMenu, QMessageBox, QPlainTextEdit, QSplitter, QStatusBar,
        QTabWidget, QToolBar, QTreeWidget, QTreeWidgetItem, QVBoxLayout,
        QWidget, QTableWidget, QTableWidgetItem, QHeaderView, QAbstractItemView,
        QListWidget, QDialog, QPushButton, QTextEdit, QCompleter,
    )
except ImportError:
    try:
        from PySide6.QtCore import Qt, QSize, QSocketNotifier, QTimer
        from PySide6.QtGui import (
            QAction, QColor, QFont, QKeySequence, QPainter, QTextCharFormat,
            QTextCursor, QTextFormat, QSyntaxHighlighter, QStandardItem, QStandardItemModel,
        )
        from PySide6.QtWidgets import (
            QApplication, QFileDialog, QHBoxLayout, QInputDialog, QLabel,
            QMainWindow, QMenu, QMessageBox, QPlainTextEdit, QSplitter, QStatusBar,
            QTabWidget, QToolBar, QTreeWidget, QTreeWidgetItem, QVBoxLayout,
            QWidget, QTableWidget, QTableWidgetItem, QHeaderView, QAbstractItemView,
            QListWidget, QDialog, QPushButton, QTextEdit, QCompleter,
        )
    except ImportError:
        print(
            "Falta Qt para el IDE. Instala:\n"
            "  sudo apt install python3-pyqt6\n"
            "  # o: pip install PyQt6\n",
            file=sys.stderr,
        )
        sys.exit(1)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
JAF = os.environ.get("JAF", os.path.join(ROOT, "build", "jaf"))
SKIP_DIRS = {".git", "build", "__pycache__", ".cursor", "node_modules", ".venv"}
TEXT_EXTS = {".oly", ".asm", ".c", ".h", ".md", ".txt", ".expect", ".json", ".py", ".y", ".l"}
NUEVO_OLY = """$ archivo nuevo JAF
ent resultado = 0;

vacio def main() {
}
"""

QSS = """
/* Sin font-size global: si no, setFont() del editor no aplica (Qt stylesheet gana). */
QMainWindow, QWidget { background:#1e1e1e; color:#cccccc;
  font-family:'Segoe UI','DejaVu Sans',sans-serif; }
QToolBar { background:#333333; border:none; spacing:4px; padding:4px 8px; }
QToolBar QToolButton { background:transparent; color:#cccccc; border:none;
  border-radius:4px; padding:6px 12px; }
QToolBar QToolButton:hover { background:#3c3c3c; }
QToolBar QToolButton:pressed { background:#094771; }
QMenuBar { background:#3c3c3c; color:#cccccc; border:none; }
QMenuBar::item:selected { background:#094771; }
QMenu { background:#252526; color:#cccccc; border:1px solid #454545; }
QMenu::item:selected { background:#094771; }
QTreeWidget { background:#252526; border:none; outline:none; padding:4px; }
QTreeWidget::item { padding:3px 4px; border-radius:3px; }
QTreeWidget::item:hover { background:#2a2d2e; }
QTreeWidget::item:selected { background:#094771; color:#fff; }
QTabWidget::pane { border:none; background:#1e1e1e; }
QTabBar::tab { background:#2d2d2d; color:#969696; border:none;
  padding:8px 16px; margin-right:1px; }
QTabBar::tab:selected { background:#1e1e1e; color:#fff; border-top:2px solid #007acc; }
QTabBar::tab:hover:!selected { background:#2a2a2a; color:#ccc; }
QPlainTextEdit { background:#1e1e1e; color:#d4d4d4; border:none;
  selection-background-color:#264f78; }
QTableWidget { background:#1e1e1e; alternate-background-color:#252526;
  gridline-color:#2d2d2d; border:none; selection-background-color:#094771; }
QHeaderView::section { background:#252526; color:#bbb; border:none;
  border-bottom:1px solid #2d2d2d; padding:6px 8px; font-weight:600; }
QStatusBar { background:#007acc; color:#fff; }
QStatusBar QLabel { color:#fff; padding:0 8px; }
QSplitter::handle { background:#2d2d2d; }
QScrollBar:vertical { background:#1e1e1e; width:10px; margin:0; }
QScrollBar::handle:vertical { background:#424242; border-radius:4px; min-height:24px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
QScrollBar:horizontal { background:#1e1e1e; height:10px; }
QScrollBar::handle:horizontal { background:#424242; border-radius:4px; min-width:24px; }
QDialog { background:#252526; color:#ccc; }
QListWidget { background:#252526; border:1px solid #454545; }
QListWidget::item { padding:6px 10px; }
QListWidget::item:selected { background:#094771; }
QPushButton { background:#0e639c; color:#fff; border:none; border-radius:4px; padding:8px 14px; }
QPushButton:hover { background:#1177bb; }
QLineEdit { background:#3c3c3c; border:1px solid #3c3c3c; border-radius:3px;
  padding:6px; color:#ccc; }
QLineEdit:focus { border:1px solid #007acc; }
"""


# SUGERENCIAS DE CÓDIGO (LL(1))
# Se analiza con LL(1) el código que está antes del cursor. Lo que queda en la
# pila dice qué puede venir después, y eso es lo que se sugiere: palabras
# clave, tipos, variables visibles en ese punto o funciones (después de llamar).
# Pasos: 1 lexer, 2 gramática, 3 tabla LL(1), 4 analizador, 5 ámbitos,
# 6 sugerencias, 7 popup.

# 1. Lexer: mismas reglas que src/lexer.l. Los comentarios y las cadenas se
# guardan como zonas, porque dentro de ellas no se sugiere nada.
PALABRAS_CLAVE = {
    "ent", "real", "doble", "bool", "caract", "cad", "vacio", "list",
    "verdadero", "falso", "def", "llamar", "retorno", "terminar",
    "si", "sino", "mientras", "para",
}
TIPOS = ("ent", "real", "doble", "bool", "caract", "cad")
TOKEN_RE = re.compile(r"""
    (?P<espacio>\s+)
  | (?P<comentario>\$[^\n]*|[—–―][^—–―]*[—–―]?)
  | (?P<cadena>"(?:[^"\\\n]|\\.)*"?|'(?:[^'\\\n]|\\.)*'?)
  | (?P<num>0[xX][0-9A-Fa-f]+|[0-9]+)
  | (?P<palabra>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<simbolo>\#import|==|!=|<=|>=|&&|\|\||\+=|-=|[-+*/%=<>!;,(){}\[\].:])
  | (?P<error>.)
""", re.VERBOSE)


@dataclass
class Token:
    tipo: str       # terminal de la gramática: 'ent', ';', 'ID', 'NUM', ...
    texto: str
    ini: int        # posición donde empieza en el texto
    fin: int        # posición donde termina (exclusiva)


def ll1_tokenizar(texto):
    """Devuelve (tokens, zonas). Cada zona es (inicio, fin) de un comentario o una cadena."""
    toks, zonas = [], []
    for m in TOKEN_RE.finditer(texto):
        clase, lex, ini, fin = m.lastgroup, m.group(), m.start(), m.end()
        if clase == "espacio":
            continue
        if clase in ("comentario", "cadena"):
            if clase == "cadena":
                cerrada = len(lex) > 1 and lex[-1] == lex[0]
            else:                                    # el de `$` llega hasta el fin de línea
                cerrada = len(lex) > 1 and lex[0] != "$" and lex[-1] in "—–―"
            zonas.append((ini, fin if cerrada else fin + 1))  # abierta: incluye el final
            if clase == "comentario":
                continue
            tipo = "CADENA" if lex[0] == '"' else "CARACTER"
        elif clase == "palabra":
            tipo = lex if lex in PALABRAS_CLAVE else "ID"
        else:
            tipo = {"num": "NUM", "simbolo": lex}.get(clase, "ERROR")
        toks.append(Token(tipo, lex, ini, fin))
    return toks, zonas


# 2. Gramática LL(1): la de src/parser.y sin recursión por la izquierda,
# factorizada y con la precedencia de operadores escrita en niveles.
# Cada ID lleva una marca que no cambia el terminal, solo indica qué sugerir:
#   ID@var    variable (en una expresión o a la izquierda de una asignación)
#   ID@func   función, después de `llamar`
#   ID@nuevo  nombre nuevo que se está declarando (no se sugiere nada)
EPS = "ε"
FIN = "$"
INICIAL = "programa"
GRAMATICA = {
    "programa":          [["importaciones", "elementos"]],
    "importaciones":     [["#import", "CADENA", ";", "importaciones"], []],
    "elementos":         [["elemento", "elementos"], []],
    "elemento":          [["tipo", "elem_tipo"],
                          ["vacio", "def", "ID@nuevo", "(", "parametros", ")", "bloque"],
                          ["sent_no_decl"]],
    "elem_tipo":         [["def", "ID@nuevo", "(", "parametros", ")", "bloque"],
                          ["ID@nuevo", "decl_resto"]],
    "tipo":              [["tipo_escalar"], ["list", "tipo_escalar"]],
    "tipo_escalar":      [[t] for t in TIPOS],
    "parametros":        [["tipo", "ID@nuevo", "parametros_r"], []],
    "parametros_r":      [[",", "tipo", "ID@nuevo", "parametros_r"], []],
    "bloque":            [["{", "sentencias", "}"]],
    "sentencias":        [["sentencia", "sentencias"], []],
    "sentencia":         [["tipo", "ID@nuevo", "decl_resto"], ["sent_no_decl"]],
    "sent_no_decl":      [["ID@var", "indice_opt", "op_asig", "expresion", ";"],
                          ["retorno", "retorno_resto"],
                          ["llamar", "ID@func", "(", "argumentos", ")", ";"],
                          ["terminar", ";"],
                          ["si", "(", "expresion", ")", "bloque", "si_resto"],
                          ["mientras", "(", "expresion", ")", "bloque"],
                          ["para", "(", "para_inicio", ";", "expresion", ";",
                           "para_paso", ")", "bloque"]],
    "retorno_resto":     [["(", "expresion", ")", ";"], [";"]],
    "decl_resto":        [[";"], ["=", "valor_inicial", ";"],
                          ["[", "expresion", "]", "dim2", ";"]],
    "dim2":              [["[", "expresion", "]"], []],
    "valor_inicial":     [["expresion"], ["literal_lista"]],
    "literal_lista":     [["[", "elemento_lista", "elementos_lista_r", "]"]],
    "elementos_lista_r": [[",", "elemento_lista", "elementos_lista_r"], []],
    "elemento_lista":    [["expresion"], ["literal_lista"]],
    "indice_opt":        [["[", "expresion", "]", "dim2"], []],
    "op_asig":           [["="], ["+="], ["-="]],
    "si_resto":          [["sino", "sino_resto"], []],
    "sino_resto":        [["si", "(", "expresion", ")", "bloque", "si_resto"], ["bloque"]],
    "para_inicio":       [["tipo", "ID@nuevo", "=", "expresion"],
                          ["ID@var", "indice_opt", "op_asig", "expresion"], []],
    "para_paso":         [["ID@var", "indice_opt", "op_asig", "expresion"], []],
    "argumentos":        [["expresion", "argumentos_r"], []],
    "argumentos_r":      [[",", "expresion", "argumentos_r"], []],
    # Precedencia de menor a mayor, igual que los %left de parser.y
    "expresion":         [["exp_and", "exp_or_r"]],
    "exp_or_r":          [["||", "exp_and", "exp_or_r"], []],
    "exp_and":           [["exp_ig", "exp_and_r"]],
    "exp_and_r":         [["&&", "exp_ig", "exp_and_r"], []],
    "exp_ig":            [["exp_rel", "exp_ig_r"]],
    "exp_ig_r":          [[op, "exp_rel", "exp_ig_r"] for op in ("==", "!=")] + [[]],
    "exp_rel":           [["exp_suma", "exp_rel_r"]],
    "exp_rel_r":         [[op, "exp_suma", "exp_rel_r"] for op in ("<", ">", "<=", ">=")] + [[]],
    "exp_suma":          [["exp_mul", "exp_suma_r"]],
    "exp_suma_r":        [[op, "exp_mul", "exp_suma_r"] for op in ("+", "-")] + [[]],
    "exp_mul":           [["exp_unaria", "exp_mul_r"]],
    "exp_mul_r":         [[op, "exp_unaria", "exp_mul_r"] for op in ("*", "/", "%")] + [[]],
    "exp_unaria":        [["!", "exp_unaria"], ["-", "exp_unaria"], ["primario"]],
    "primario":          [["verdadero"], ["falso"], ["ID@var", "indice_opt"], ["NUM"],
                          ["CADENA"], ["CARACTER"],
                          ["llamar", "ID@func", "(", "argumentos", ")"],
                          ["(", "expresion", ")"]],
}


def es_no_terminal(s):
    return s in GRAMATICA


def base(s):
    """Terminal sin la marca: 'ID@var' -> 'ID'."""
    return s.split("@")[0]


# 3. FIRST, FOLLOW y tabla predictiva M[A][a]
def _first_de(simbolos, FIRST):
    res = set()
    for s in simbolos:
        if not es_no_terminal(s):
            return res | {base(s)}
        res |= FIRST[s] - {EPS}
        if EPS not in FIRST[s]:
            return res
    return res | {EPS}


def _calcular_first():
    FIRST = {A: set() for A in GRAMATICA}
    cambio = True
    while cambio:
        cambio = False
        for A, prods in GRAMATICA.items():
            for p in prods:
                nuevo = _first_de(p, FIRST)
                if not nuevo <= FIRST[A]:
                    FIRST[A] |= nuevo
                    cambio = True
    return FIRST


def _calcular_follow():
    FOLLOW = {A: set() for A in GRAMATICA}
    FOLLOW[INICIAL].add(FIN)
    cambio = True
    while cambio:
        cambio = False
        for A, prods in GRAMATICA.items():
            for p in prods:
                for i, B in enumerate(p):
                    if es_no_terminal(B):
                        resto = _first_de(p[i + 1:], FIRST)
                        nuevo = (resto - {EPS}) | (FOLLOW[A] if EPS in resto else set())
                        if not nuevo <= FOLLOW[B]:
                            FOLLOW[B] |= nuevo
                            cambio = True
    return FOLLOW


def _construir_tabla():
    M = {A: {} for A in GRAMATICA}
    for A, prods in GRAMATICA.items():
        for p in prods:
            f = _first_de(p, FIRST)
            for a in (f - {EPS}) | (FOLLOW[A] if EPS in f else set()):
                assert a not in M[A], f"conflicto LL(1) en M[{A}, {a}]"
                M[A][a] = p
    return M


FIRST = _calcular_first()
FOLLOW = _calcular_follow()
TABLA = _construir_tabla()


# 4. Analizador predictivo con pila. Si encuentra un error se recupera en
# modo pánico, para que un error más arriba no apague las sugerencias.
def _uso_id(X):
    """Marca ('var', 'func' o 'nuevo') del ID que se deriva primero de X."""
    if not es_no_terminal(X):
        return X.split("@")[1] if X.startswith("ID@") else None
    for s in TABLA[X].get("ID", []):
        if not es_no_terminal(s) or "ID" in FIRST[s]:
            return _uso_id(s)
        if EPS not in FIRST[s]:
            break
    return None


def ll1_esperados(tokens):
    """Analiza los tokens y devuelve {terminal: marca} con lo que puede venir después."""
    pila = [FIN, INICIAL]
    i = 0
    while i < len(tokens):
        a, X = tokens[i].tipo, pila[-1]
        if X == FIN:
            i += 1                                   # sobran tokens
        elif not es_no_terminal(X):
            pila.pop()
            if base(X) == a:
                i += 1                               # si no coincide, se asume que faltaba X
        elif a in TABLA[X]:
            pila.pop()
            pila.extend(reversed(TABLA[X][a]))
        elif a in FOLLOW[X]:
            pila.pop()                               # modo pánico: se abandona X
        else:
            i += 1                                   # modo pánico: se descarta el token
    esperados = {}
    for X in reversed(pila):                         # FIRST de lo que queda en la pila
        if X == FIN:
            break
        if not es_no_terminal(X):
            esperados.setdefault(base(X), _uso_id(X))
            break
        for a in FIRST[X] - {EPS}:
            esperados.setdefault(a, _uso_id(X) if a == "ID" else None)
        if EPS not in FIRST[X]:
            break
    return esperados


# 5. Ámbitos: qué variables se ven en el cursor y qué funciones hay
def _tipo_en(toks, k):
    """Si en toks[k] empieza un tipo, devuelve (tipo, índice siguiente)."""
    if toks[k].tipo == "list" and k + 1 < len(toks) and toks[k + 1].tipo in TIPOS:
        return "list " + toks[k + 1].tipo, k + 2
    if toks[k].tipo in TIPOS:
        return toks[k].tipo, k + 1
    return None, k


def ll1_variables(toks):
    """Variables visibles después del último token, de la más interna a la global."""
    ambitos = [[]]          # pila de bloques { }, el primero son las globales
    pendientes = []         # declaradas entre paréntesis (parámetros o la del `para`)
    parentesis = 0
    for k, t in enumerate(toks):
        if t.tipo == "(":
            parentesis += 1
        elif t.tipo == ")":
            parentesis -= 1
        elif t.tipo == "{":
            ambitos.append(pendientes)               # entran al bloque que se abre
            pendientes = []
        elif t.tipo == "}" and len(ambitos) > 1:
            ambitos.pop()
        else:
            tipo, j = _tipo_en(toks, k)
            if tipo and j < len(toks) and toks[j].tipo == "ID":
                (pendientes if parentesis > 0 else ambitos[-1]).append((toks[j].texto, tipo))
    if parentesis > 0:
        ambitos.append(pendientes)                   # cursor dentro de `para (...)`
    visibles = {}
    for amb in reversed(ambitos):
        for nombre, tipo in reversed(amb):
            visibles.setdefault(nombre, tipo)        # la más interna tiene prioridad
    return visibles


def ll1_funciones(toks):
    """Funciones definidas en el archivo: nombre -> firma."""
    funcs = {}
    for k in range(1, len(toks) - 2):
        if toks[k].tipo == "def" and toks[k + 1].tipo == "ID" and toks[k + 2].tipo == "(":
            cierre = next((j for j in range(k + 3, len(toks)) if toks[j].tipo == ")"), len(toks))
            params = " ".join(t.texto for t in toks[k + 3:cierre]).replace(" ,", ",")
            ret = ("list " if k >= 2 and toks[k - 2].tipo == "list" else "") + toks[k - 1].texto
            funcs[toks[k + 1].texto] = f"{ret} {toks[k + 1].texto}({params})"
    return funcs


# 6. Sugerencias
INTRINSECAS = {  # nombre -> tipo de retorno (src/semantico.c)
    "autenticar": "vacio", "cerrar_sesion": "vacio", "leer_estado": "ent",
    "cargar_llave": "vacio", "ronda_feistel": "vacio", "feistel_cifrar": "vacio",
    "feistel_descifrar": "vacio", "leer_mem": "ent", "escribir_mem": "vacio",
}
# La gramática acepta real, doble, caract y cad (igual que parser.y), pero el
# semántico los rechaza con el error 14. Por eso solo se sugieren ent y bool.
TIPOS_SOPORTADOS = ("ent", "bool")
PLANTILLAS = {  # '$0' marca dónde queda el cursor
    "si":       "si ($0) {\n    \n}",
    "sino":     "sino {\n    $0\n}",
    "mientras": "mientras ($0) {\n    \n}",
    "para":     "para (ent i = 0; i < $0; i += 1) {\n    \n}",
    "def":      "def $0() {\n    \n}",
    "vacio":    "vacio def $0() {\n    \n}",     # vacio solo va antes de una función
}
ORDEN = {"variable": 0, "funcion": 1, "tipo": 2, "palabra clave": 3, "simbolo": 4}


@dataclass
class Sugerencia:
    label: str
    kind: str
    detail: str
    insertar: str   # texto que se inserta en el editor


def ll1_sugerir(texto, pos):
    """Devuelve (prefijo, sugerencias) para el cursor en la posición `pos`."""
    toks, zonas = ll1_tokenizar(texto)
    if any(a < pos < b for a, b in zonas):
        return "", []                                # dentro de comentario o cadena
    previos = [t for t in toks if t.ini < pos]
    prefijo = ""
    if previos and previos[-1].fin >= pos and (previos[-1].tipo == "ID"
                                               or previos[-1].tipo in PALABRAS_CLAVE):
        t = previos.pop()                            # palabra a medio escribir
        prefijo = texto[t.ini:pos]
    recien_abierto = bool(previos) and previos[-1].tipo == "{"    # se acaba de escribir {
    sugs = []
    for a, uso in ll1_esperados(previos).items():
        if uso == "var":
            sugs += [Sugerencia(n, "variable", t, n) for n, t in ll1_variables(previos).items()]
        elif uso == "func":
            funcs = {n: f"{r} {n}(...) · intrínseca" for n, r in INTRINSECAS.items()}
            funcs.update(ll1_funciones(toks))
            sugs += [Sugerencia(n, "funcion", d, n + "($0)") for n, d in funcs.items()]
        elif a in TIPOS_SOPORTADOS or a == "list":
            sugs.append(Sugerencia(a, "tipo", "tipo", a + " "))
        elif a in PALABRAS_CLAVE and a not in TIPOS:
            sugs.append(Sugerencia(a, "palabra clave", "", PLANTILLAS.get(a, a + " ")))
        elif a == "}" and recien_abierto:            # cierra el bloque y deja el cursor adentro
            sugs.append(Sugerencia("}", "simbolo", "cerrar el bloque", "\n    $0\n}"))
        elif a == "}":                               # hay un bloque { sin cerrar
            sugs.append(Sugerencia("}", "simbolo", "cerrar el bloque", "}"))
        elif a == "{":                               # después de la cabecera de una función, si, ...
            sugs.append(Sugerencia("{", "simbolo", "abrir el bloque", "{\n    $0\n}"))
    primero = "}" if recien_abierto else None        # justo después de { va primero el cierre
    sugs.sort(key=lambda s: (s.label != primero, ORDEN[s.kind],
                             "" if s.kind == "variable" else s.label))
    return prefijo, [s for s in sugs if s.label.startswith(prefijo)]


# 7. Popup mientras se escribe. Se abre al escribir letras (y después de
# `llamar `). Enter/Tab insertan, Esc cierra y las flechas navegan.
class CompletadorJaf:
    def __init__(self, editor):
        self.editor = editor
        self.al_actualizar = None                    # llena la pestaña SUGERENCIAS
        self.sugs = {}
        self.modelo = QStandardItemModel(editor)
        self.c = QCompleter(self.modelo, editor)
        self.c.setWidget(editor)
        self.c.setCompletionMode(QCompleter.CompletionMode.UnfilteredPopupCompletion)
        self.c.activated.connect(self._insertar)     # clic con el mouse

    def antes_de_tecla(self, e):
        """True si el popup consumió la tecla y el editor no debe procesarla."""
        popup = self.c.popup()
        if not popup.isVisible():
            return False
        if e.key() in (Qt.Key.Key_Return, Qt.Key.Key_Enter, Qt.Key.Key_Tab) \
                and popup.currentIndex().isValid():
            self._insertar(popup.currentIndex().data())
            return True
        if e.key() == Qt.Key.Key_Escape:
            popup.hide()
            return True
        return False

    def despues_de_tecla(self, e):
        cur = self.editor.textCursor()
        linea = cur.block().text()[:cur.positionInBlock()]
        if e.text().isalnum() or e.text() == "_" or e.key() == Qt.Key.Key_Backspace:
            self.mostrar()                           # solo se abre si hay palabra a medio escribir
        elif e.text() == "{" or (e.text() == " " and re.search(r"\bllamar $", linea)):
            self.mostrar(forzar=True)                # después de `{` o de `llamar `
        else:
            self.c.popup().hide()

    def mostrar(self, forzar=False):
        """Abre el popup. Sin `forzar` solo lo abre si se está escribiendo una palabra."""
        prefijo, sugs = ll1_sugerir(self.editor.toPlainText(), self.editor.textCursor().position())
        if self.al_actualizar:
            self.al_actualizar(sugs)
        if not sugs or (not prefijo and not forzar) or [s.insertar for s in sugs] == [prefijo]:
            self.c.popup().hide()
            return
        self.modelo.clear()
        for s in sugs:
            item = QStandardItem(s.label)
            item.setToolTip(s.detail or s.kind)
            self.modelo.appendRow(item)
        self.sugs = {s.label: s for s in sugs}
        rect = self.editor.cursorRect()
        rect.setWidth(300)
        self.c.complete(rect)
        self.c.popup().setCurrentIndex(self.modelo.index(0, 0))

    def _insertar(self, etiqueta):
        """Reemplaza la palabra a medio escribir por la sugerencia elegida."""
        self.c.popup().hide()
        s = self.sugs.get(etiqueta)
        if not s:
            return
        cur = self.editor.textCursor()
        linea = cur.block().text()[:cur.positionInBlock()]
        sangria = re.match(r"\s*", linea).group()
        texto = s.insertar.replace("\n", "\n" + sangria)
        marca = texto.find("$0")
        texto = texto.replace("$0", "")
        largo = len(re.search(r"\w*$", linea).group())
        if texto == "}" and not linea.strip():
            largo = min(4, len(linea))               # la llave va un nivel a la izquierda
        cur.movePosition(QTextCursor.MoveOperation.Left, QTextCursor.MoveMode.KeepAnchor, largo)
        inicio = cur.selectionStart()
        cur.insertText(texto)
        if marca >= 0:
            cur.setPosition(inicio + marca)
        self.editor.setTextCursor(cur)
        if texto.endswith(" "):                      # 'llamar ', 'ent ', 'list ' ...
            QTimer.singleShot(0, lambda: self.mostrar(forzar=True))


class JafHighlighter(QSyntaxHighlighter):
    def __init__(self, doc):
        super().__init__(doc)
        self.rules = []
        kw, ty = QTextCharFormat(), QTextCharFormat()
        kw.setForeground(QColor("#569cd6"))
        ty.setForeground(QColor("#4ec9b0"))
        for w in ("si", "sino", "para", "mientras", "retorno", "llamar", "def",
                  "verdadero", "falso"):
            self.rules.append((rf"\b{w}\b", kw))
        for w in ("ent", "bool", "vacio", "list"):
            self.rules.append((rf"\b{w}\b", ty))
        cmt, st, num = QTextCharFormat(), QTextCharFormat(), QTextCharFormat()
        cmt.setForeground(QColor("#6a9955"))
        st.setForeground(QColor("#ce9178"))
        num.setForeground(QColor("#b5cea8"))
        self.rules += [(r"\$.*", cmt), (r'"[^"]*"', st), (r"\b\d+\b", num)]
        self.err = QTextCharFormat()
        self.err.setUnderlineStyle(QTextCharFormat.UnderlineStyle.SpellCheckUnderline)
        self.err.setUnderlineColor(QColor("#f14c4c"))
        self.err.setBackground(QColor("#5a1d1d"))
        self.error_lines: set[int] = set()

    def set_errors(self, lines):
        self.error_lines = set(lines)
        self.rehighlight()

    def highlightBlock(self, text):
        for pat, fmt in self.rules:
            for m in re.finditer(pat, text):
                self.setFormat(m.start(), m.end() - m.start(), fmt)
        if self.currentBlock().blockNumber() + 1 in self.error_lines:
            self.setFormat(0, len(text), self.err)


class LineNumberArea(QWidget):
    def __init__(self, editor):
        super().__init__(editor)
        self.editor = editor

    def sizeHint(self):
        return QSize(self.editor.line_number_area_width(), 0)

    def paintEvent(self, event):
        self.editor.paint_line_numbers(event)


def _mono_font(pt):
    f = QFont("Cascadia Code")
    if not f.exactMatch():
        f = QFont("DejaVu Sans Mono")
    f.setStyleHint(QFont.StyleHint.Monospace)
    f.setFixedPitch(True)
    f.setPointSize(pt)
    return f


class TerminalPane(QPlainTextEdit):
    """Terminal integrada: bash real vía PTY + mini emulador VT (como VS Code).

    No abre gnome-terminal: instancia /bin/bash en un PTY y pinta la salida aquí.
    """

    def __init__(self, cwd=None):
        super().__init__()
        self.setReadOnly(False)
        self.setUndoRedoEnabled(False)
        self.setMaximumBlockCount(12000)
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self._context_menu)
        self._master = None
        self._pid = None
        self._notifier = None
        self._pending = ""  # escape incompleto (str)
        self.set_zoom(14)
        self.start(cwd or os.getcwd())

    def start(self, cwd):
        self.stop()
        try:
            pid, master = pty.fork()
        except OSError as e:
            self.setPlainText(f"[terminal] no se pudo crear PTY: {e}\n")
            return
        if pid == 0:
            try:
                os.chdir(cwd)
            except OSError:
                pass
            os.environ["TERM"] = "xterm-256color"
            os.environ.setdefault("COLORTERM", "truecolor")
            shell = os.environ.get("SHELL", "/bin/bash")
            try:
                os.execvp(shell, [shell, "-i"])
            except OSError:
                os.execvp("/bin/bash", ["/bin/bash", "-i"])
            os._exit(1)
        self._pid = pid
        self._master = master
        os.set_blocking(master, False)
        self._pending = ""
        self._set_winsize()
        self._notifier = QSocketNotifier(master, QSocketNotifier.Type.Read, self)
        self._notifier.activated.connect(self._on_ready)
        self.clear()

    def stop(self):
        if self._notifier is not None:
            self._notifier.setEnabled(False)
            self._notifier.deleteLater()
            self._notifier = None
        if self._master is not None:
            try:
                os.close(self._master)
            except OSError:
                pass
            self._master = None
        if self._pid is not None:
            try:
                os.kill(self._pid, signal.SIGHUP)
                os.waitpid(self._pid, os.WNOHANG)
            except OSError:
                pass
            self._pid = None

    def set_cwd(self, cwd):
        if cwd and os.path.isdir(cwd):
            self.start(cwd)

    def set_zoom(self, pt):
        sz = max(10, min(22, pt))
        self.setStyleSheet(
            f"QPlainTextEdit{{background:#0c0c0c;color:#cccccc;border:none;"
            f"font-family:'Cascadia Code','DejaVu Sans Mono',monospace;"
            f"font-size:{sz}pt;}}"
        )
        self.setFont(_mono_font(sz))
        self._set_winsize()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._set_winsize()

    def _set_winsize(self):
        if self._master is None:
            return
        fm = self.fontMetrics()
        cols = max(40, self.viewport().width() // max(1, fm.horizontalAdvance("M")))
        rows = max(8, self.viewport().height() // max(1, fm.lineSpacing()))
        try:
            fcntl.ioctl(
                self._master,
                termios.TIOCSWINSZ,
                struct.pack("HHHH", rows, cols, 0, 0),
            )
        except OSError:
            pass

    def _on_ready(self, *_):
        if self._master is None:
            return
        try:
            data = os.read(self._master, 16384)
        except OSError:
            return
        if not data:
            return
        try:
            text = data.decode("utf-8", errors="replace")
        except Exception:
            return
        self._feed(self._pending + text)

    def _feed(self, text: str):
        """Interpreta controles VT básicos (CR, BS, clear, borrar línea, CSI)."""
        i = 0
        n = len(text)
        self._pending = ""
        while i < n:
            ch = text[i]
            # Escape sequence
            if ch == "\x1b":
                if i + 1 >= n:
                    self._pending = text[i:]
                    break
                nxt = text[i + 1]
                if nxt == "[":  # CSI
                    j = i + 2
                    # Finales VT: incluir P/@/X (borrar/insertar chars mid-línea)
                    _csi_final = "ABCDEFGHJKSTLMPXZfmhlnsu@`"
                    while j < n and text[j] not in _csi_final:
                        if text[j] not in "0123456789;?":
                            break
                        j += 1
                    if j >= n:
                        self._pending = text[i:]
                        break
                    params = text[i + 2:j]
                    final = text[j]
                    self._csi(params, final)
                    i = j + 1
                    continue
                if nxt == "]":  # OSC … BEL o ST
                    j = text.find("\x07", i + 2)
                    if j < 0:
                        j = text.find("\x1b\\", i + 2)
                        if j < 0:
                            self._pending = text[i:]
                            break
                        i = j + 2
                    else:
                        i = j + 1
                    continue
                # ESC + un char (p.ej. ESC M, ESC 7)
                i += 2
                continue
            if ch == "\r":
                # Cursor al inicio de la línea actual (readline redibuja así)
                cur = self.textCursor()
                cur.movePosition(QTextCursor.MoveOperation.StartOfBlock)
                self.setTextCursor(cur)
                i += 1
                continue
            if ch in ("\b", "\x08"):
                # En VT, BS solo mueve el cursor a la izquierda (NO borra).
                # El borrado real llega como "\b \b" o CSI K tras sobrescribir.
                cur = self.textCursor()
                if not cur.atBlockStart():
                    cur.movePosition(QTextCursor.MoveOperation.Left)
                    self.setTextCursor(cur)
                i += 1
                continue
            if ch == "\x7f":
                # DEL: borrar bajo el cursor (si no, el de la izquierda)
                cur = self.textCursor()
                if not cur.atBlockEnd():
                    cur.deleteChar()
                elif not cur.atBlockStart():
                    cur.deletePreviousChar()
                self.setTextCursor(cur)
                i += 1
                continue
            if ch == "\x07":
                i += 1
                continue
            if ch == "\n":
                cur = self.textCursor()
                cur.movePosition(QTextCursor.MoveOperation.EndOfBlock)
                cur.insertText("\n")
                self.setTextCursor(cur)
                i += 1
                continue
            if ch == "\t":
                self._put_char(" ")
                i += 1
                continue
            if ord(ch) < 32:
                i += 1
                continue
            # Texto imprimible: modo sobrescritura (tras \\r)
            self._put_char(ch)
            i += 1
        self.ensureCursorVisible()

    def _put_char(self, ch: str):
        cur = self.textCursor()
        if not cur.atBlockEnd():
            cur.deleteChar()  # sobrescribe (redibujado de línea)
        cur.insertText(ch)
        self.setTextCursor(cur)

    def _csi(self, params: str, final: str):
        nums = []
        if params and params not in ("?", ""):
            for p in params.replace("?", "").split(";"):
                if p.isdigit():
                    nums.append(int(p))
        a = nums[0] if nums else 0

        if final == "J":  # borrar pantalla
            # 0: cursor→fin, 1: inicio→cursor, 2/3: toda
            if a >= 2:
                self.clear()
            elif a == 0:
                cur = self.textCursor()
                cur.movePosition(
                    QTextCursor.MoveOperation.End,
                    QTextCursor.MoveMode.KeepAnchor,
                )
                cur.removeSelectedText()
                self.setTextCursor(cur)
            return
        if final == "K":  # borrar en la línea
            cur = self.textCursor()
            if a == 1:
                cur.movePosition(
                    QTextCursor.MoveOperation.StartOfBlock,
                    QTextCursor.MoveMode.KeepAnchor,
                )
            else:
                # 0 o 2: desde cursor hasta fin de línea
                cur.movePosition(
                    QTextCursor.MoveOperation.EndOfBlock,
                    QTextCursor.MoveMode.KeepAnchor,
                )
            cur.removeSelectedText()
            self.setTextCursor(cur)
            return
        if final == "H" or final == "f":  # cursor home / posición
            row = (nums[0] if len(nums) > 0 else 1) - 1
            col = (nums[1] if len(nums) > 1 else 1) - 1
            if row <= 0 and col <= 0:
                cur = self.textCursor()
                cur.movePosition(QTextCursor.MoveOperation.Start)
                self.setTextCursor(cur)
            return
        if final == "C":  # cursor adelante (sin borrar)
            nmov = a or 1
            cur = self.textCursor()
            for _ in range(nmov):
                if cur.atBlockEnd():
                    break
                cur.movePosition(QTextCursor.MoveOperation.Right)
            self.setTextCursor(cur)
            return
        if final == "D":  # cursor atrás (sin borrar)
            nmov = a or 1
            cur = self.textCursor()
            for _ in range(nmov):
                if cur.atBlockStart():
                    break
                cur.movePosition(QTextCursor.MoveOperation.Left)
            self.setTextCursor(cur)
            return
        if final == "A":
            nmov = a or 1
            cur = self.textCursor()
            cur.movePosition(QTextCursor.MoveOperation.Up, n=nmov)
            self.setTextCursor(cur)
            return
        if final == "B":
            nmov = a or 1
            cur = self.textCursor()
            cur.movePosition(QTextCursor.MoveOperation.Down, n=nmov)
            self.setTextCursor(cur)
            return
        if final == "P":  # DCH: borrar n chars bajo el cursor (resto se corre)
            # Esto es lo que usa readline al dar Backspace a mitad de línea
            n = a or 1
            cur = self.textCursor()
            for _ in range(n):
                if cur.atBlockEnd():
                    break
                cur.deleteChar()
            self.setTextCursor(cur)
            return
        if final == "X":  # ECH: borrar n chars (poner espacios, cursor no se mueve)
            n = a or 1
            cur = self.textCursor()
            pos = cur.position()
            for _ in range(n):
                if cur.atBlockEnd():
                    break
                cur.deleteChar()
                cur.insertText(" ")
            cur.setPosition(pos)
            self.setTextCursor(cur)
            return
        if final == "@":  # ICH: insertar n espacios (para luego escribir)
            n = a or 1
            cur = self.textCursor()
            cur.insertText(" " * n)
            cur.movePosition(QTextCursor.MoveOperation.Left, n=n)
            self.setTextCursor(cur)
            return
        if final == "G":  # columna absoluta
            col = max(1, a or 1) - 1
            cur = self.textCursor()
            cur.movePosition(QTextCursor.MoveOperation.StartOfBlock)
            for _ in range(col):
                if cur.atBlockEnd():
                    break
                cur.movePosition(QTextCursor.MoveOperation.Right)
            self.setTextCursor(cur)
            return
        if final == "m":
            return  # colores/estilos: ignorar (texto plano)
        # resto CSI: ignorar

    def _pty_write(self, data: bytes):
        if self._master is None or not data:
            return
        try:
            os.write(self._master, data)
        except OSError:
            pass

    def paste_clipboard(self):
        text = QApplication.clipboard().text()
        if not text:
            return
        text = text.replace("\r\n", "\n").replace("\r", "\n")
        if text.endswith("\n") and text.count("\n") == 1:
            text = text[:-1]
        self._pty_write(text.replace("\n", "\r").encode("utf-8", errors="replace"))

    def insertFromMimeData(self, source):
        if source and source.hasText():
            text = source.text().replace("\r\n", "\n").replace("\r", "\n")
            if text.endswith("\n") and text.count("\n") == 1:
                text = text[:-1]
            self._pty_write(text.replace("\n", "\r").encode("utf-8", errors="replace"))

    def _context_menu(self, pos):
        menu = QMenu(self)
        menu.addAction("Pegar", self.paste_clipboard)
        copiar = menu.addAction("Copiar")
        copiar.setEnabled(self.textCursor().hasSelection())
        copiar.triggered.connect(self.copy)
        menu.addSeparator()
        menu.addAction("Limpiar", self.clear)
        menu.exec(self.mapToGlobal(pos))

    def keyPressEvent(self, event):
        if self._master is None:
            return
        key = event.key()
        mods = event.modifiers()
        if key == Qt.Key.Key_V and (mods & Qt.KeyboardModifier.ControlModifier):
            self.paste_clipboard()
            event.accept()
            return
        raw = None
        if key in (Qt.Key.Key_Return, Qt.Key.Key_Enter):
            raw = b"\r"
        elif key == Qt.Key.Key_Backspace:
            raw = b"\x7f"  # DEL — readline/xterm suelen mapear Backspace así
        elif key == Qt.Key.Key_Delete:
            raw = b"\x1b[3~"
        elif key == Qt.Key.Key_Tab:
            raw = b"\t"
        elif key == Qt.Key.Key_Up:
            raw = b"\x1b[A"
        elif key == Qt.Key.Key_Down:
            raw = b"\x1b[B"
        elif key == Qt.Key.Key_Right:
            raw = b"\x1b[C"
        elif key == Qt.Key.Key_Left:
            raw = b"\x1b[D"
        elif key == Qt.Key.Key_Home:
            raw = b"\x1b[H"
        elif key == Qt.Key.Key_End:
            raw = b"\x1b[F"
        elif key == Qt.Key.Key_C and mods & Qt.KeyboardModifier.ControlModifier:
            if self.textCursor().hasSelection():
                self.copy()
                event.accept()
                return
            raw = b"\x03"
        elif key == Qt.Key.Key_D and mods & Qt.KeyboardModifier.ControlModifier:
            raw = b"\x04"
        elif key == Qt.Key.Key_L and mods & Qt.KeyboardModifier.ControlModifier:
            raw = b"\x0c"  # form feed; bash/clear lo maneja; también pintamos
            self.clear()
        else:
            t = event.text()
            if t and not (mods & Qt.KeyboardModifier.ControlModifier):
                raw = t.encode("utf-8", errors="replace")
        if raw:
            self._pty_write(raw)
        event.accept()

    def closeEvent(self, event):
        self.stop()
        super().closeEvent(event)


class CodeEditor(QPlainTextEdit):
    def __init__(self, on_change=None, on_cursor=None, font_pt=15):
        super().__init__()
        self.on_change = on_change
        self.on_cursor = on_cursor
        self._loading = False
        self._zoom_cb = None
        self.setLineWrapMode(QPlainTextEdit.LineWrapMode.NoWrap)
        self.highlighter = JafHighlighter(self.document())
        self.line_area = LineNumberArea(self)
        self.blockCountChanged.connect(self._update_width)
        self.updateRequest.connect(self._update_area)
        self.cursorPositionChanged.connect(self._on_cursor)
        self.textChanged.connect(self._on_text)
        self.set_zoom(font_pt)
        self._update_width(0)
        self.completador = CompletadorJaf(self)

    def keyPressEvent(self, event):
        # Popup de sugerencias: Enter/Tab insertan, Esc cierra y al escribir se actualiza.
        if self.completador.antes_de_tecla(event):
            return
        super().keyPressEvent(event)
        self.completador.despues_de_tecla(event)

    def set_zoom_callback(self, cb):
        self._zoom_cb = cb

    def wheelEvent(self, event):
        if event.modifiers() & Qt.KeyboardModifier.ControlModifier:
            delta = event.angleDelta().y()
            if self._zoom_cb and delta:
                self._zoom_cb(1 if delta > 0 else -1)
            event.accept()
            return
        super().wheelEvent(event)

    def _on_text(self):
        if self._loading or not self.on_change:
            return
        self.on_change()

    def load_text(self, text):
        """Carga texto sin marcar el buffer como modificado."""
        self._loading = True
        self.blockSignals(True)
        self.setPlainText(text)
        self.document().setModified(False)
        self.blockSignals(False)
        self._loading = False
        self._update_width(0)
        if self.on_cursor:
            self.on_cursor()

    def set_zoom(self, pt):
        # Forzar tamaño por stylesheet: el QSS del QMainWindow pisa setFont() solo.
        self.setStyleSheet(
            f"QPlainTextEdit{{background:#1e1e1e;color:#d4d4d4;border:none;"
            f"selection-background-color:#264f78;"
            f"font-family:'Cascadia Code','DejaVu Sans Mono','Consolas',monospace;"
            f"font-size:{pt}pt;}}"
        )
        self.setFont(_mono_font(pt))
        self.setTabStopDistance(4 * self.fontMetrics().horizontalAdvance(" "))
        if hasattr(self, "line_area"):
            self._update_width(0)
            self.line_area.update()

    def _on_cursor(self):
        es = QTextEdit.ExtraSelection()
        es.format.setBackground(QColor("#2a2a2a"))
        es.format.setProperty(QTextFormat.Property.FullWidthSelection, True)
        es.cursor = self.textCursor()
        es.cursor.clearSelection()
        self.setExtraSelections([es])
        if self.on_cursor:
            self.on_cursor()

    def line_number_area_width(self):
        return 12 + self.fontMetrics().horizontalAdvance("9") * len(str(max(1, self.blockCount())))

    def _update_width(self, _):
        self.setViewportMargins(self.line_number_area_width(), 0, 0, 0)

    def _update_area(self, rect, dy):
        if dy:
            self.line_area.scroll(0, dy)
        else:
            self.line_area.update(0, rect.y(), self.line_area.width(), rect.height())
        if rect.contains(self.viewport().rect()):
            self._update_width(0)

    def resizeEvent(self, e):
        super().resizeEvent(e)
        cr = self.contentsRect()
        self.line_area.setGeometry(cr.left(), cr.top(), self.line_number_area_width(), cr.height())

    def paint_line_numbers(self, event):
        painter = QPainter(self.line_area)
        painter.fillRect(event.rect(), QColor("#1e1e1e"))
        block = self.firstVisibleBlock()
        num = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())
        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                painter.setPen(QColor("#6e7681"))
                painter.drawText(0, top, self.line_area.width() - 6,
                                 self.fontMetrics().height(),
                                 Qt.AlignmentFlag.AlignRight, str(num + 1))
            block = block.next()
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())
            num += 1

    def marcar_errores(self, diags, ruta):
        base = os.path.basename(ruta)
        lines = set()
        for d in diags:
            f = os.path.basename(d.get("file", "") or "")
            if f and f != base:
                continue
            lines.add(max(1, int(d.get("line", 1))))
        self.highlighter.set_errors(lines)

    def goto_line(self, line):
        self.setTextCursor(QTextCursor(self.document().findBlockByNumber(max(0, line - 1))))
        self.centerCursor()
        self.setFocus()

    def line_col(self):
        c = self.textCursor()
        return c.blockNumber() + 1, c.positionInBlock() + 1


class JafIde(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("JAF")
        self.resize(1280, 800)
        self.workspace = None
        self.buffers = {}
        self.dirty = set()
        self.untitled = 0
        self.zoom_pt = 15
        self.ZOOM_MIN, self.ZOOM_MAX = 10, 28
        self.ultimo = {"diagnostics": [], "fixes": [], "completions": [], "clones": []}
        self._build()
        self.setStyleSheet(QSS)

        args = [os.path.abspath(p) for p in sys.argv[1:]]
        dirs = [p for p in args if os.path.isdir(p)]
        files = [p for p in args if os.path.isfile(p)]
        self.set_workspace(dirs[0] if dirs else (os.path.dirname(files[0]) if files else ROOT))
        for p in files:
            self.open_path(p)
        self._apply_zoom()
        # Editor ~80%, panel inferior ~20%
        self.vsplit.setSizes([720, 140])

    def _build(self):
        m = self.menuBar()
        arch = m.addMenu("Archivo")
        self._act(arch, "Nuevo archivo", "Ctrl+N", self.nuevo_archivo)
        self._act(arch, "Nuevo proyecto…", "Ctrl+Shift+N", self.nuevo_proyecto)
        arch.addSeparator()
        self._act(arch, "Abrir archivo…", "Ctrl+O", self.abrir_archivo)
        self._act(arch, "Abrir carpeta…", "Ctrl+Shift+O", self.abrir_carpeta)
        arch.addSeparator()
        self._act(arch, "Guardar", "Ctrl+S", self.guardar)
        self._act(arch, "Guardar como…", "Ctrl+Shift+S", self.guardar_como)
        arch.addSeparator()
        self._act(arch, "Cerrar pestaña", "Ctrl+W", self.cerrar_tab)
        self._act(arch, "Cerrar todas", None, self.cerrar_todas)
        arch.addSeparator()
        self._act(arch, "Salir", "Ctrl+Q", self.close)
        an = m.addMenu("Analizar")
        self._act(an, "Analizar", "F5", self.analizar)
        self._act(an, "Aplicar fix", None, self.aplicar_fix)
        self._act(an, "Sugerencias", "Ctrl+Space", self.sugerir)
        ver = m.addMenu("Ver")
        self._act(ver, "Acercar", "Ctrl+=", self.zoom_in)
        self._act(ver, "Acercar", "Ctrl++", self.zoom_in)  # teclado numérico / algunas layouts
        self._act(ver, "Alejar", "Ctrl+-", self.zoom_out)
        self._act(ver, "Zoom normal", "Ctrl+0", self.zoom_reset)
        ver.addSeparator()
        self._act(ver, "Terminal", "Ctrl+`", self.focus_terminal)

        tb = QToolBar()
        tb.setMovable(False)
        self.addToolBar(tb)
        for text, slot in (
            ("Abrir carpeta", self.abrir_carpeta),
            ("Nuevo", self.nuevo_archivo),
            ("Guardar", self.guardar),
            ("Cerrar", self.cerrar_tab),
            ("Analizar", self.analizar),
            ("Fix", self.aplicar_fix),
            ("Sugerir", self.sugerir),
        ):
            a = QAction(text, self)
            a.triggered.connect(slot)
            tb.addAction(a)

        split = QSplitter()
        self.setCentralWidget(split)

        side = QWidget()
        side.setMinimumWidth(220)
        side.setMaximumWidth(380)
        sv = QVBoxLayout(side)
        sv.setContentsMargins(12, 12, 12, 8)
        sv.setSpacing(8)
        t = QLabel("EXPLORADOR")
        t.setStyleSheet("color:#bbb;font-size:11px;font-weight:700;letter-spacing:1px;")
        sv.addWidget(t)
        self.proj_lbl = QLabel("(sin carpeta)")
        self.proj_lbl.setStyleSheet("color:#e7e7e7;font-weight:600;")
        sv.addWidget(self.proj_lbl)
        row = QHBoxLayout()
        for txt, slot in (
            ("Abrir carpeta", self.abrir_carpeta),
            ("Nuevo proy.", self.nuevo_proyecto),
            ("Nuevo .oly", self.nuevo_archivo),
        ):
            b = QPushButton(txt)
            b.setStyleSheet(
                "QPushButton{background:#2d2d2d;color:#ccc;border:none;"
                "border-radius:4px;padding:6px 8px;}"
                "QPushButton:hover{background:#3c3c3c;}"
            )
            b.clicked.connect(slot)
            row.addWidget(b)
        sv.addLayout(row)
        self.tree = QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.tree.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.tree.customContextMenuRequested.connect(self._tree_menu)
        self.tree.itemDoubleClicked.connect(self._tree_open)
        sv.addWidget(self.tree, 1)
        split.addWidget(side)

        self.vsplit = QSplitter(Qt.Orientation.Vertical)
        self.tabs = QTabWidget()
        self.tabs.setTabsClosable(True)
        self.tabs.setMovable(True)
        self.tabs.setDocumentMode(True)
        self.tabs.tabCloseRequested.connect(self.cerrar_idx)
        self.tabs.currentChanged.connect(lambda _: self._status())
        self.vsplit.addWidget(self.tabs)

        self.bottom = QTabWidget()
        self.bottom.setDocumentMode(True)
        self.bottom.setMinimumHeight(80)
        self.tbl_err = self._table(["SEV", "MENSAJE", "ARCHIVO", "LÍNEA", "COL"])
        self.tbl_sug = self._table(["LABEL", "KIND", "DETAIL"])
        self.tbl_clo = self._table(["TIPO", "A", "B"])
        self.tbl_err.cellDoubleClicked.connect(self._goto_err)
        self.tbl_clo.cellDoubleClicked.connect(self._goto_clone)
        self.terminal = TerminalPane(ROOT)
        self.bottom.addTab(self.tbl_err, "PROBLEMAS")
        self.bottom.addTab(self.tbl_sug, "SUGERENCIAS")
        self.bottom.addTab(self.tbl_clo, "CLONES")
        self.bottom.addTab(self.terminal, "TERMINAL")
        self.vsplit.addWidget(self.bottom)
        self.vsplit.setStretchFactor(0, 5)
        self.vsplit.setStretchFactor(1, 1)
        self.vsplit.setSizes([720, 140])
        split.addWidget(self.vsplit)
        split.setSizes([260, 1000])

        sb = QStatusBar()
        self.setStatusBar(sb)
        self.sl = QLabel("JAF")
        self.sr = QLabel("Ln 1, Col 1")
        sb.addWidget(self.sl, 1)
        sb.addPermanentWidget(self.sr)

    def _act(self, menu, text, shortcut, slot):
        a = QAction(text, self)
        if shortcut:
            a.setShortcut(QKeySequence(shortcut))
        a.triggered.connect(slot)
        menu.addAction(a)

    def _table(self, headers):
        t = QTableWidget(0, len(headers))
        t.setHorizontalHeaderLabels(headers)
        t.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        t.verticalHeader().setVisible(False)
        t.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        t.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        t.setAlternatingRowColors(True)
        t.setShowGrid(False)
        return t

    def set_workspace(self, path):
        path = os.path.abspath(path)
        if not os.path.isdir(path):
            QMessageBox.critical(self, "JAF", f"No es una carpeta:\n{path}")
            return
        self.workspace = path
        self.proj_lbl.setText(os.path.basename(path) or path)
        self.setWindowTitle(f"{os.path.basename(path)} — JAF")
        self._fill_tree()
        self.sl.setText(f"carpeta: {path}")
        if hasattr(self, "terminal"):
            self.terminal.set_cwd(path)

    def abrir_carpeta(self):
        d = QFileDialog.getExistingDirectory(self, "Abrir carpeta", self.workspace or ROOT)
        if d:
            self.set_workspace(d)

    def nuevo_proyecto(self):
        parent = QFileDialog.getExistingDirectory(
            self, "¿Dónde crear el proyecto?",
            os.path.dirname(self.workspace or ROOT),
        )
        if not parent:
            return
        nombre, ok = QInputDialog.getText(self, "Nuevo proyecto", "Nombre:",
                                          text="mi_proyecto_jaf")
        if not ok or not nombre.strip():
            return
        dest = os.path.join(parent, nombre.strip())
        try:
            os.makedirs(dest, exist_ok=False)
        except FileExistsError:
            if QMessageBox.question(self, "JAF", f"Ya existe:\n{dest}\n¿Abrirla?") != QMessageBox.StandardButton.Yes:
                return
        except OSError as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        main = os.path.join(dest, "main.oly")
        if not os.path.isfile(main):
            open(main, "w", encoding="utf-8").write(NUEVO_OLY)
        self.set_workspace(dest)
        self.open_path(main)

    def _fill_tree(self):
        self.tree.clear()
        if not self.workspace:
            return
        root = QTreeWidgetItem([os.path.basename(self.workspace)])
        root.setData(0, Qt.ItemDataRole.UserRole, self.workspace)
        root.setExpanded(True)
        self.tree.addTopLevelItem(root)
        self._walk(self.workspace, root)

    def _walk(self, directory, parent):
        try:
            names = sorted(os.listdir(directory),
                           key=lambda s: (not os.path.isdir(os.path.join(directory, s)), s.lower()))
        except OSError:
            return
        for name in names:
            if name in SKIP_DIRS or name.startswith("."):
                continue
            path = os.path.join(directory, name)
            item = QTreeWidgetItem([name])
            item.setData(0, Qt.ItemDataRole.UserRole, path)
            parent.addChild(item)
            if os.path.isdir(path):
                self._walk(path, item)

    def _tree_open(self, item, _):
        path = item.data(0, Qt.ItemDataRole.UserRole)
        if path and os.path.isfile(path):
            self.open_path(path)

    def _tree_path(self, item=None):
        item = item or self.tree.currentItem()
        if not item:
            return None
        path = item.data(0, Qt.ItemDataRole.UserRole)
        return path if path else None

    def _tree_menu(self, pos):
        item = self.tree.itemAt(pos)
        if item is None:
            return
        self.tree.setCurrentItem(item)
        path = self._tree_path(item)
        if not path:
            return
        is_dir = os.path.isdir(path)
        is_root = path == self.workspace
        menu = QMenu(self)
        if not is_dir:
            menu.addAction("Abrir", lambda: self.open_path(path))
        menu.addAction("Abrir en terminal", lambda p=path: self._tree_open_terminal(p))
        menu.addAction("Mostrar en el explorador", lambda p=path: self._tree_reveal(p))
        menu.addSeparator()
        menu.addAction("Copiar ruta", lambda p=path: self._copy_text(p))
        if self.workspace:
            try:
                rel = os.path.relpath(path, self.workspace)
            except ValueError:
                rel = path
            menu.addAction("Copiar ruta relativa", lambda r=rel: self._copy_text(r))
        menu.addSeparator()
        if is_dir:
            menu.addAction("Nuevo archivo…", lambda p=path: self._tree_new_file(p))
            menu.addAction("Nueva carpeta…", lambda p=path: self._tree_new_folder(p))
            menu.addSeparator()
        if not is_root:
            menu.addAction("Renombrar…", lambda p=path: self._tree_rename(p))
            menu.addAction("Eliminar…", lambda p=path: self._tree_delete(p))
        menu.exec(self.tree.viewport().mapToGlobal(pos))

    def _copy_text(self, text):
        QApplication.clipboard().setText(text)
        self.sl.setText(f"copiado: {text}")

    def _tree_open_terminal(self, path):
        cwd = path if os.path.isdir(path) else os.path.dirname(path)
        self.terminal.set_cwd(cwd)
        self.focus_terminal()

    def _tree_reveal(self, path):
        target = path if os.path.isdir(path) else os.path.dirname(path)
        try:
            if sys.platform.startswith("linux"):
                subprocess.Popen(["xdg-open", target])
            elif sys.platform == "darwin":
                subprocess.Popen(["open", target])
            else:
                os.startfile(target)  # type: ignore[attr-defined]
        except OSError as e:
            QMessageBox.warning(self, "JAF", str(e))

    def _tree_new_file(self, folder):
        name, ok = QInputDialog.getText(self, "Nuevo archivo", "Nombre:", text="nuevo.oly")
        if not ok or not name.strip():
            return
        dest = os.path.join(folder, name.strip())
        if os.path.exists(dest):
            QMessageBox.warning(self, "JAF", f"Ya existe:\n{dest}")
            return
        try:
            open(dest, "w", encoding="utf-8").write(
                NUEVO_OLY if dest.endswith(".oly") else ""
            )
        except OSError as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        self._fill_tree()
        self.open_path(dest)

    def _tree_new_folder(self, folder):
        name, ok = QInputDialog.getText(self, "Nueva carpeta", "Nombre:", text="nueva_carpeta")
        if not ok or not name.strip():
            return
        dest = os.path.join(folder, name.strip())
        try:
            os.makedirs(dest, exist_ok=False)
        except FileExistsError:
            QMessageBox.warning(self, "JAF", f"Ya existe:\n{dest}")
            return
        except OSError as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        self._fill_tree()

    def _tree_rename(self, path):
        old = os.path.basename(path)
        name, ok = QInputDialog.getText(self, "Renombrar", "Nuevo nombre:", text=old)
        if not ok or not name.strip() or name.strip() == old:
            return
        dest = os.path.join(os.path.dirname(path), name.strip())
        if os.path.exists(dest):
            QMessageBox.warning(self, "JAF", f"Ya existe:\n{dest}")
            return
        try:
            os.rename(path, dest)
        except OSError as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        # Actualizar pestaña abierta si era este archivo
        if path in self.buffers:
            ed = self.buffers.pop(path)
            self.buffers[dest] = ed
            if path in self.dirty:
                self.dirty.discard(path)
                self.dirty.add(dest)
            ed.on_change = lambda p=dest: self._dirty(p)
        self._fill_tree()
        self._refresh_tabs()
        self._status()

    def _tree_delete(self, path):
        kind = "carpeta" if os.path.isdir(path) else "archivo"
        r = QMessageBox.question(
            self, "Eliminar",
            f"¿Eliminar {kind}?\n{path}\n\nNo se puede deshacer.",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )
        if r != QMessageBox.StandardButton.Yes:
            return
        try:
            if os.path.isdir(path):
                shutil.rmtree(path)
            else:
                os.remove(path)
        except OSError as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        # Cerrar pestañas afectadas
        to_close = []
        for p in list(self.buffers):
            if p == path or p.startswith(path + os.sep):
                to_close.append(p)
        for p in to_close:
            ed = self.buffers.pop(p, None)
            self.dirty.discard(p)
            if ed is not None:
                i = self.tabs.indexOf(ed)
                if i >= 0:
                    self.tabs.removeTab(i)
        self._fill_tree()
        self._status()

    def current_path(self):
        ed = self.tabs.currentWidget()
        for p, e in self.buffers.items():
            if e is ed:
                return p
        return None

    def current_editor(self):
        w = self.tabs.currentWidget()
        return w if isinstance(w, CodeEditor) else None

    def _label(self, path):
        name = path.replace("untitled:", "") if path.startswith("untitled:") else os.path.basename(path)
        return ("● " if path in self.dirty else "") + name

    def _refresh_tabs(self):
        for i in range(self.tabs.count()):
            w = self.tabs.widget(i)
            for p, e in self.buffers.items():
                if e is w:
                    self.tabs.setTabText(i, self._label(p))
                    break

    def _status(self):
        ed = self.current_editor()
        p = self.current_path()
        n = len(self.ultimo.get("diagnostics") or [])
        name = (p.replace("untitled:", "") if p and p.startswith("untitled:")
                else (os.path.basename(p) if p else "sin archivo"))
        ws = os.path.basename(self.workspace) if self.workspace else "—"
        self.sl.setText(f"{ws}  ·  {name}  ·  {n} problema(s)  ·  jaf --ide")
        if ed:
            ln, col = ed.line_col()
            self.sr.setText(f"Ln {ln}, Col {col}  ·  {self.zoom_pt}pt  ·  UTF-8")
        else:
            self.sr.setText(f"{self.zoom_pt}pt")
        if p and not p.startswith("untitled:"):
            self.setWindowTitle(f"{os.path.basename(p)} — JAF")

    def _dirty(self, path):
        self.dirty.add(path)
        self._refresh_tabs()

    def nuevo_archivo(self):
        self.untitled += 1
        name = f"sin_titulo_{self.untitled}.oly"
        key = f"untitled:{name}"
        ed = CodeEditor(
            on_change=lambda k=key: self._dirty(k),
            on_cursor=self._status,
            font_pt=self.zoom_pt,
        )
        ed.load_text(NUEVO_OLY)
        ed.set_zoom_callback(self._zoom_delta)
        ed.completador.al_actualizar = self._panel_sugerencias
        self.buffers[key] = ed
        self.dirty.add(key)  # sin guardar en disco aún
        i = self.tabs.addTab(ed, self._label(key))
        self.tabs.setCurrentIndex(i)

    def abrir_archivo(self):
        ps, _ = QFileDialog.getOpenFileNames(
            self, "Abrir", self.workspace or ROOT, "JAF (*.oly);;Todos (*.*)")
        for p in ps:
            self.open_path(p)

    def open_path(self, path):
        path = os.path.abspath(path)
        for p, ed in self.buffers.items():
            if p == path:
                self.tabs.setCurrentWidget(ed)
                return
        ed = CodeEditor(
            on_change=lambda p=path: self._dirty(p),
            on_cursor=self._status,
            font_pt=self.zoom_pt,
        )
        try:
            ed.load_text(open(path, encoding="utf-8").read())
        except OSError as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        self.dirty.discard(path)
        ed.set_zoom_callback(self._zoom_delta)
        ed.completador.al_actualizar = self._panel_sugerencias
        self.buffers[path] = ed
        self.tabs.addTab(ed, self._label(path))
        self.tabs.setCurrentWidget(ed)

    def zoom_in(self):
        self._zoom_delta(1)

    def zoom_out(self):
        self._zoom_delta(-1)

    def zoom_reset(self):
        self.zoom_pt = 15
        self._apply_zoom()

    def _zoom_delta(self, d):
        nxt = self.zoom_pt + d
        if nxt < self.ZOOM_MIN or nxt > self.ZOOM_MAX:
            return
        self.zoom_pt = nxt
        self._apply_zoom()

    def _apply_zoom(self):
        for ed in self.buffers.values():
            ed.set_zoom(self.zoom_pt)
            ed.set_zoom_callback(self._zoom_delta)
        if hasattr(self, "terminal"):
            self.terminal.set_zoom(self.zoom_pt)
        # Tablas del panel inferior (problemas / sugerencias / clones)
        ui = max(11, min(18, self.zoom_pt - 1))
        for tbl in (self.tbl_err, self.tbl_sug, self.tbl_clo):
            tbl.setStyleSheet(
                f"QTableWidget{{font-size:{ui}pt;}}"
                f"QHeaderView::section{{font-size:{ui}pt;}}"
            )
            tbl.verticalHeader().setDefaultSectionSize(max(22, ui + 10))
        self.tree.setStyleSheet(f"QTreeWidget{{font-size:{ui}pt;}}")
        self._status()

    def focus_terminal(self):
        self.bottom.setCurrentWidget(self.terminal)
        self.terminal.setFocus()

    def closeEvent(self, event):
        if hasattr(self, "terminal"):
            self.terminal.stop()
        super().closeEvent(event)

    def guardar(self):
        p = self.current_path()
        if not p:
            return
        if p.startswith("untitled:"):
            self.guardar_como()
        else:
            ed = self.buffers[p]
            open(p, "w", encoding="utf-8").write(ed.toPlainText())
            self.dirty.discard(p)
            self._refresh_tabs()
            self._status()

    def guardar_como(self):
        ed = self.current_editor()
        p = self.current_path()
        if not ed:
            return
        suggested = "main.oly"
        if p and p.startswith("untitled:"):
            suggested = p.replace("untitled:", "")
        elif p:
            suggested = os.path.basename(p)
        path, _ = QFileDialog.getSaveFileName(
            self, "Guardar como", os.path.join(self.workspace or ROOT, suggested),
            "JAF (*.oly);;Todos (*.*)")
        if not path:
            return
        path = os.path.abspath(path)
        open(path, "w", encoding="utf-8").write(ed.toPlainText())
        if p in self.buffers:
            del self.buffers[p]
        self.dirty.discard(p)
        self.buffers[path] = ed
        self._refresh_tabs()
        self._fill_tree()
        self._status()

    def cerrar_tab(self):
        self.cerrar_idx(self.tabs.currentIndex())

    def cerrar_idx(self, index):
        if index < 0:
            return
        ed = self.tabs.widget(index)
        path = None
        for p, e in self.buffers.items():
            if e is ed:
                path = p
                break
        if path and path in self.dirty:
            name = path.replace("untitled:", "") if path.startswith("untitled:") else os.path.basename(path)
            r = QMessageBox.question(
                self, "JAF", f"¿Guardar cambios en {name}?",
                QMessageBox.StandardButton.Save
                | QMessageBox.StandardButton.Discard
                | QMessageBox.StandardButton.Cancel)
            if r == QMessageBox.StandardButton.Cancel:
                return
            if r == QMessageBox.StandardButton.Save:
                self.tabs.setCurrentIndex(index)
                self.guardar()
                if path in self.dirty:
                    return
        self.tabs.removeTab(index)
        if path:
            self.buffers.pop(path, None)
            self.dirty.discard(path)
        if ed:
            ed.deleteLater()
        self._status()

    def cerrar_todas(self):
        while self.tabs.count():
            n = self.tabs.count()
            self.cerrar_idx(0)
            if self.tabs.count() == n:
                break

    def _rutas(self):
        return [p for p in self.buffers if not p.startswith("untitled:") and os.path.isfile(p)]

    def analizar(self):
        p = self.current_path()
        if p and p.startswith("untitled:"):
            QMessageBox.information(self, "JAF", "Guarda el archivo antes de analizar.")
            return
        if p in self.dirty:
            self.guardar()
        rutas = self._rutas()
        if not rutas:
            QMessageBox.information(self, "JAF", "Abre o guarda al menos un .oly")
            return
        if not os.path.isfile(JAF):
            QMessageBox.critical(self, "JAF", f"No está {JAF}. Corre: make")
            return
        try:
            r = subprocess.run([JAF, "--ide", *rutas], cwd=ROOT,
                               capture_output=True, text=True, timeout=20)
        except (OSError, subprocess.TimeoutExpired) as e:
            QMessageBox.critical(self, "JAF", str(e))
            return
        raw = (r.stdout or "").strip()
        if not raw:
            self.sl.setText("jaf --ide no devolvió JSON — implementar src/ide.c")
            return
        try:
            data = json.loads(raw)
        except json.JSONDecodeError:
            QMessageBox.critical(self, "jaf --ide", (r.stderr or raw)[:800])
            return
        self.ultimo = data
        self._fill(self.tbl_err, data.get("diagnostics", []),
                   lambda d: (d.get("severity", "Error"), d.get("msg"),
                              os.path.basename(d.get("file", "")), d.get("line"), d.get("col")))
        self._fill(self.tbl_sug, data.get("completions", []),
                   lambda d: (d.get("label"), d.get("kind"), d.get("detail")))
        self._fill(self.tbl_clo, data.get("clones", []),
                   lambda d: (f"Tipo {d.get('type')}", self._span(d.get("a")), self._span(d.get("b"))))
        ed, cur = self.current_editor(), self.current_path()
        if ed and cur and not cur.startswith("untitled:"):
            ed.marcar_errores(data.get("diagnostics", []), cur)
        self._status()

    def _span(self, s):
        return "" if not s else f"{os.path.basename(s.get('file', ''))}:{s.get('line')}-{s.get('end')}"

    def _fill(self, table, rows, proj):
        table.setRowCount(0)
        for d in rows:
            r = table.rowCount()
            table.insertRow(r)
            for c, val in enumerate(proj(d)):
                texto = "" if val is None else str(val)
                item = QTableWidgetItem(texto)
                item.setToolTip(texto) 
                table.setItem(r, c, item)
                #table.setItem(r, c, QTableWidgetItem("" if val is None else str(val)))

    def _goto_err(self, row, _):
        item = self.tbl_err.item(row, 3)
        if not item:
            return
        try:
            line = int(item.text())
        except ValueError:
            return
        ed = self.current_editor()
        if ed:
            ed.goto_line(line)

    def _goto_clone(self, row, col):
        """Doble-clic en CLONES: abre el span A (cols 0–1) o B (col 2)."""
        clones = self.ultimo.get("clones") or []
        if row < 0 or row >= len(clones):
            return
        key = "b" if col >= 2 else "a"
        span = clones[row].get(key) or {}
        path = span.get("file")
        try:
            line = int(span.get("line") or 1)
        except (TypeError, ValueError):
            line = 1
        if not path or not os.path.isfile(path):
            self.sl.setText(f"clon: no se encuentra {path}")
            return
        self.open_path(path)
        ed = self.current_editor()
        if ed:
            ed.goto_line(line)

    def aplicar_fix(self):
        fixes = self.ultimo.get("fixes") or []
        if not fixes:
            self.sl.setText("sin fixes — TODO emitir_fixes() en src/ide.c")
            return
        fx, ed = fixes[0], self.current_editor()
        if not ed:
            return
        line = int(fx.get("line", 1))
        col = max(0, int(fx.get("col", 1)) - 1)
        cur = QTextCursor(ed.document().findBlockByNumber(line - 1))
        cur.movePosition(QTextCursor.MoveOperation.Right, QTextCursor.MoveMode.MoveAnchor, col)
        cur.insertText(fx.get("text", ""))
        self.sl.setText(f"fix en {line}:{col + 1}")

    def sugerir(self):
        """Ctrl+Espacio: abre el popup de sugerencias en el cursor."""
        ed = self.current_editor()
        if ed:
            ed.completador.mostrar(forzar=True)

    def _panel_sugerencias(self, sugs):
        """Muestra en la pestaña SUGERENCIAS las sugerencias del cursor."""
        self._fill(self.tbl_sug, sugs, lambda s: (s.label, s.kind, s.detail))


def _limpiar_entorno_snap():
    """VS Code instalado como snap le pasa a su terminal variables que apuntan a
    las librerías del snap (XDG_DATA_DIRS, GSETTINGS_SCHEMA_DIR, ...). GLib las lee
    al crear la ventana y el proceso se cae con:
      GLib-GIO-ERROR: Settings schema 'org.gnome.settings-daemon.plugins.xsettings'
      does not contain a key named 'antialiasing'
    Si se detectan, se quitan antes de crear la QApplication."""
    env = os.environ
    variables = ("GSETTINGS_SCHEMA_DIR", "GTK_PATH", "GIO_MODULE_DIR", "GTK_EXE_PREFIX",
                 "GTK_IM_MODULE_FILE", "GDK_PIXBUF_MODULE_FILE", "GDK_PIXBUF_MODULEDIR",
                 "LOCPATH")
    if not any("/snap/" in env.get(v, "") for v in variables + ("XDG_DATA_DIRS",)):
        return
    for v in variables:
        if "/snap/" in env.get(v, ""):
            env.pop(v)
    dirs = [d for d in env.get("XDG_DATA_DIRS", "").split(":") if d and "/snap/" not in d]
    env["XDG_DATA_DIRS"] = ":".join(dirs or ["/usr/local/share", "/usr/share"])
    if env.get("QT_QPA_PLATFORMTHEME", "gtk3") in ("", "gtk3", "gtk2"):
        env["QT_QPA_PLATFORMTHEME"] = "xdgdesktopportal"


def main():
    _limpiar_entorno_snap()              # terminal de VS Code (snap): evita GLib-GIO-ERROR
    os.environ.setdefault("QT_AUTO_SCREEN_SCALE_FACTOR", "1")
    app = QApplication(sys.argv)
    app.setApplicationName("JAF IDE")
    app.setStyle("Fusion")
    app.setFont(QFont("Segoe UI", 13))
    win = JafIde()
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
