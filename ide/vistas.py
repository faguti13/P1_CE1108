"""Vistas del inspector del IDE JAF: AST gráfico, LL(1), símbolos, IR, paquetes y métricas.

Todas se llenan con el JSON de `jaf --ide-todo` (ver src/ide.h) salvo la de LL(1),
que recibe la traza que calcula jaf_ide.py con su propia tabla predictiva.
"""
from __future__ import annotations

import csv
import html
import re

try:
    from PyQt6.QtCore import Qt, QPointF, QRectF, pyqtSignal as Signal
    from PyQt6.QtGui import (
        QBrush, QColor, QFont, QFontMetricsF, QImage, QPainter, QPainterPath, QPen,
        QSyntaxHighlighter, QTextCharFormat,
    )
    from PyQt6.QtWidgets import (
        QAbstractItemView, QCheckBox, QComboBox, QFileDialog, QGraphicsPathItem,
        QGraphicsScene, QGraphicsSimpleTextItem, QGraphicsView, QHBoxLayout, QHeaderView,
        QLabel, QLineEdit, QPlainTextEdit, QPushButton, QScrollArea, QSplitter, QTabWidget,
        QTableWidget, QTableWidgetItem, QVBoxLayout, QWidget,
    )
except ImportError:
    from PySide6.QtCore import Qt, QPointF, QRectF, Signal
    from PySide6.QtGui import (
        QBrush, QColor, QFont, QFontMetricsF, QImage, QPainter, QPainterPath, QPen,
        QSyntaxHighlighter, QTextCharFormat,
    )
    from PySide6.QtWidgets import (
        QAbstractItemView, QCheckBox, QComboBox, QFileDialog, QGraphicsPathItem,
        QGraphicsScene, QGraphicsSimpleTextItem, QGraphicsView, QHBoxLayout, QHeaderView,
        QLabel, QLineEdit, QPlainTextEdit, QPushButton, QScrollArea, QSplitter, QTabWidget,
        QTableWidget, QTableWidgetItem, QVBoxLayout, QWidget,
    )


def mono(pt=11):
    f = QFont("DejaVu Sans Mono")
    f.setStyleHint(QFont.StyleHint.Monospace)
    f.setPointSize(pt)
    return f


def tabla(headers, stretch_last=True):
    t = QTableWidget(0, len(headers))
    t.setHorizontalHeaderLabels(headers)
    t.verticalHeader().setVisible(False)
    t.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
    t.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
    t.setAlternatingRowColors(True)
    t.setShowGrid(False)
    h = t.horizontalHeader()
    h.setSectionResizeMode(QHeaderView.ResizeMode.Interactive)
    h.setStretchLastSection(stretch_last)
    return t


def boton(texto, slot):
    b = QPushButton(texto)
    b.setStyleSheet("QPushButton{background:#2d2d2d;color:#ccc;border:none;border-radius:4px;"
                    "padding:5px 10px;} QPushButton:hover{background:#3c3c3c;}")
    b.clicked.connect(slot)
    return b


def fila(*widgets):
    h = QHBoxLayout()
    h.setContentsMargins(6, 4, 6, 4)
    for w in widgets:
        if w is None:
            h.addStretch(1)
        else:
            h.addWidget(w)
    return h


def titulo(texto):
    l = QLabel(texto)
    l.setStyleSheet("color:#bbb;font-weight:600;padding:2px 6px;")
    return l


# ---------------------------------------------------------------------------
# AST gráfico
# ---------------------------------------------------------------------------
COLOR_NODO = (
    (("Programa", "Modulo", "Import"), "#3a3d41"),
    (("Funcion", "Parametro"), "#0e5a6b"),
    (("Decl", "Lista", "Tipo"), "#264f78"),
    (("Literal", "Cadena", "Caracter", "Bool"), "#6b4a1e"),
    (("Ident", "Variable", "Id"), "#7a5a1e"),
    (("Binari", "Unari", "Llamada", "Indice", "Expr", "Oper"), "#2e6b4a"),
)


def color_nodo(tipo):
    for claves, c in COLOR_NODO:
        if any(k.lower() in tipo.lower() for k in claves):
            return c
    return "#5a3a6b"                                 # sentencias


class _Nodo:
    __slots__ = ("d", "hijos", "colapsado", "x", "y", "w", "h", "sw", "prof", "lineas")

    def __init__(self, d, prof):
        self.d = d
        self.prof = prof
        self.hijos = [_Nodo(h, prof + 1) for h in d.get("hijos") or [] if isinstance(h, dict)]
        self.colapsado = False
        self.lineas = [d.get("tipo", "?")]
        txt = d.get("texto")
        if txt is not None and txt != "":
            self.lineas.append(str(txt)[:28])
        elif "valor" in d:
            self.lineas.append(str(d["valor"]))

    def total(self):
        return 1 + sum(h.total() for h in self.hijos)


class AstCanvas(QGraphicsView):
    """Árbol del JSON de `jaf --json`: rueda = zoom, arrastrar = paneo, clic = ir a la
    línea, doble clic = colapsar/expandir el subárbol."""
    ir_a = Signal(int, int)

    HGAP, VGAP = 14, 46

    def __init__(self):
        super().__init__()
        self.setScene(QGraphicsScene(self))
        self.setRenderHint(QPainter.RenderHint.Antialiasing)
        self.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
        self.setTransformationAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.setBackgroundBrush(QColor("#1e1e1e"))
        self.raiz = None
        self.items = {}
        self.fuente = QFont("DejaVu Sans", 9)
        self.fm = QFontMetricsF(self.fuente)

    def cargar(self, ast):
        self.raiz = _Nodo(ast, 0) if isinstance(ast, dict) else None
        if self.raiz and self.raiz.total() > 220:    # árboles grandes: se abren por niveles
            self._colapsar_desde(self.raiz, 3)
        self.dibujar()
        self.ajustar()

    def _colapsar_desde(self, n, prof):
        n.colapsado = bool(n.hijos) and n.prof >= prof
        for h in n.hijos:
            self._colapsar_desde(h, prof)

    def expandir_todo(self):
        if self.raiz:
            self._colapsar_desde(self.raiz, 10 ** 6)
            self.dibujar()

    def colapsar_funciones(self):
        if self.raiz:
            self._colapsar_desde(self.raiz, 2)
            self.dibujar()

    def _medir(self, n):
        n.w = max(self.fm.horizontalAdvance(t) for t in n.lineas) + 16
        n.h = len(n.lineas) * self.fm.height() + 8
        visibles = [] if n.colapsado else n.hijos
        for h in visibles:
            self._medir(h)
        ancho = sum(h.sw for h in visibles) + self.HGAP * max(0, len(visibles) - 1)
        n.sw = max(n.w, ancho)

    def _colocar(self, n, x0, y):
        n.x, n.y = x0 + n.sw / 2, y
        if n.colapsado:
            return
        ancho = sum(h.sw for h in n.hijos) + self.HGAP * max(0, len(n.hijos) - 1)
        x = x0 + (n.sw - ancho) / 2
        alto = self.fm.height() * 2 + 8
        for h in n.hijos:
            self._colocar(h, x, y + alto + self.VGAP)
            x += h.sw + self.HGAP

    def dibujar(self):
        sc = self.scene()
        sc.clear()
        self.items = {}
        if not self.raiz:
            t = sc.addSimpleText("(sin AST: el archivo no parsea o no se ha inspeccionado)")
            t.setBrush(QColor("#888"))
            return
        self._medir(self.raiz)
        self._colocar(self.raiz, 0, 0)
        self._pintar(self.raiz)
        sc.setSceneRect(sc.itemsBoundingRect().adjusted(-40, -40, 40, 40))

    def _pintar(self, n):
        sc = self.scene()
        if not n.colapsado:
            for h in n.hijos:
                p = QPainterPath(QPointF(n.x, n.y + n.h))
                my = (n.y + n.h + h.y) / 2
                p.cubicTo(QPointF(n.x, my), QPointF(h.x, my), QPointF(h.x, h.y))
                e = sc.addPath(p, QPen(QColor("#555"), 1.2))
                e.setZValue(-1)
                rol = h.d.get("rol")
                if rol:
                    r = sc.addSimpleText(rol, QFont("DejaVu Sans", 7))
                    r.setBrush(QColor("#888"))
                    r.setPos(h.x + 3, h.y - 14)
                self._pintar(h)
        caja = QPainterPath()
        caja.addRoundedRect(QRectF(n.x - n.w / 2, n.y, n.w, n.h), 6, 6)
        it = QGraphicsPathItem(caja)
        it.setBrush(QBrush(QColor(color_nodo(n.d.get("tipo", "")))))
        pen = QPen(QColor("#d7ba7d") if n.colapsado else QColor("#777"), 1.5 if n.colapsado else 1)
        if n.colapsado:
            pen.setStyle(Qt.PenStyle.DashLine)
        it.setPen(pen)
        it.setToolTip(f"{n.d.get('tipo')}  {n.d.get('texto', '')}\n"
                      f"línea {n.d.get('linea')}:{n.d.get('col')}"
                      + (f"\nrol: {n.d.get('rol')}" if n.d.get("rol") else "")
                      + (f"\n{n.total() - 1} nodos ocultos (doble clic)" if n.colapsado else ""))
        it.setCursor(Qt.CursorShape.PointingHandCursor)
        sc.addItem(it)
        self.items[id(it)] = n
        for k, t in enumerate(n.lineas):
            ti = QGraphicsSimpleTextItem(t, it)
            ti.setFont(self.fuente)
            ti.setBrush(QColor("#fff" if k == 0 else "#ddd"))
            ti.setPos(n.x - self.fm.horizontalAdvance(t) / 2, n.y + 4 + k * self.fm.height())
        if n.colapsado and n.hijos:
            b = sc.addSimpleText(f"+{n.total() - 1}", QFont("DejaVu Sans", 7))
            b.setBrush(QColor("#d7ba7d"))
            b.setPos(n.x + n.w / 2 - 6, n.y + n.h - 2)

    def _nodo_en(self, pos):
        it = self.itemAt(pos)
        while it is not None and id(it) not in self.items:
            it = it.parentItem()
        return self.items.get(id(it)) if it is not None else None

    def mousePressEvent(self, e):
        n = self._nodo_en(e.position().toPoint())
        if n and e.button() == Qt.MouseButton.LeftButton:
            self.ir_a.emit(int(n.d.get("linea") or 1), int(n.d.get("col") or 1))
        super().mousePressEvent(e)

    def mouseDoubleClickEvent(self, e):
        n = self._nodo_en(e.position().toPoint())
        if n and n.hijos:
            antes = self.mapFromScene(QPointF(n.x, n.y))
            n.colapsado = not n.colapsado
            self.dibujar()
            despues = self.mapFromScene(QPointF(n.x, n.y))
            d = despues - antes
            self.horizontalScrollBar().setValue(self.horizontalScrollBar().value() + d.x())
            self.verticalScrollBar().setValue(self.verticalScrollBar().value() + d.y())
            return
        super().mouseDoubleClickEvent(e)

    def wheelEvent(self, e):
        f = 1.15 if e.angleDelta().y() > 0 else 1 / 1.15
        self.scale(f, f)

    def ajustar(self):
        r = self.scene().itemsBoundingRect()
        if r.isValid():
            self.fitInView(r, Qt.AspectRatioMode.KeepAspectRatio)
            if self.transform().m11() > 1.0:         # árbol chico: tamaño natural
                self.resetTransform()
                self.centerOn(r.center())
            elif self.transform().m11() < 0.6 and self.raiz:   # árbol ancho: raíz arriba, a tamaño legible
                self.resetTransform()
                self.scale(0.85, 0.85)
                alto = self.viewport().height() / 0.85
                self.centerOn(QPointF(self.raiz.x, self.raiz.y + alto / 2 - 30))

    def exportar_png(self, ruta):
        r = self.scene().itemsBoundingRect().adjusted(-20, -20, 20, 20)
        img = QImage(int(r.width()), int(r.height()), QImage.Format.Format_ARGB32)
        img.fill(QColor("#1e1e1e"))
        p = QPainter(img)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)
        self.scene().render(p, QRectF(img.rect()), r)
        p.end()
        return img.save(ruta)


class VistaAst(QWidget):
    def __init__(self):
        super().__init__()
        self.canvas = AstCanvas()
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        v.addLayout(fila(boton("Ajustar", self.canvas.ajustar),
                         boton("Expandir todo", self.canvas.expandir_todo),
                         boton("Colapsar funciones", self.canvas.colapsar_funciones),
                         boton("Exportar PNG…", self._png), None,
                         titulo("clic: ir a la línea · doble clic: colapsar · rueda: zoom")))
        v.addWidget(self.canvas, 1)

    def _png(self):
        ruta, _ = QFileDialog.getSaveFileName(self, "Exportar AST", "ast.png", "PNG (*.png)")
        if ruta:
            self.canvas.exportar_png(ruta)


# ---------------------------------------------------------------------------
# LL(1): traza y matriz predictiva
# ---------------------------------------------------------------------------
class VistaLL1(QWidget):
    """`analizar(hasta_cursor)` devuelve (pasos, celdas). Cada paso es un dict con
    pila, entrada, accion, celda (A, a) o None y pos (offset del token en el texto)."""
    ir_a_pos = Signal(int)

    def __init__(self, tabla_ll1, terminales, analizar):
        super().__init__()
        self.tabla_ll1, self.analizar = tabla_ll1, analizar
        self.terminales = list(terminales) + ["$"]
        self.no_terminales = list(tabla_ll1)
        self.pasos = []
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        self.chk = QCheckBox("Hasta el cursor")
        self.chk.toggled.connect(self.actualizar)
        self.resumen = titulo("")
        v.addLayout(fila(boton("Analizar", self.actualizar), self.chk,
                         boton("Exportar traza…", self._exportar_traza),
                         boton("Exportar matriz…", self._exportar_matriz), None, self.resumen))
        self.tabs = QTabWidget()
        self.tabs.setDocumentMode(True)
        self.t_traza = tabla(["PASO", "PILA", "ENTRADA RESTANTE", "ACCIÓN / PRODUCCIÓN M[A,a]"])
        self.t_traza.setFont(mono(10))
        self.t_traza.setTextElideMode(Qt.TextElideMode.ElideLeft)     # el tope de la pila queda a la vista
        self.t_traza.itemSelectionChanged.connect(self._sel_paso)
        self.t_mat = QTableWidget(len(self.no_terminales), len(self.terminales))
        self.t_mat.setHorizontalHeaderLabels(self.terminales)
        self.t_mat.setVerticalHeaderLabels(self.no_terminales)
        self.t_mat.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        self.t_mat.setFont(mono(9))
        self.t_mat.setStyleSheet("QTableWidget{gridline-color:#333;}")
        self._llenar_matriz({})
        self.tabs.addTab(self.t_traza, "Traza paso a paso")
        self.tabs.addTab(self.t_mat, "Matriz predictiva M[A,a]")
        v.addWidget(self.tabs, 1)

    @staticmethod
    def produccion(simbolos):
        return " ".join(s.split("@")[0] for s in simbolos)

    def _llenar_matriz(self, celdas):
        mx = max(celdas.values()) if celdas else 1
        self.t_mat.clearContents()
        for i, A in enumerate(self.no_terminales):
            for j, a in enumerate(self.terminales):
                prod = self.tabla_ll1[A].get(a)
                if prod is None:
                    continue
                it = QTableWidgetItem(self.produccion(prod))
                usos = celdas.get((A, a), 0)
                it.setToolTip(f"M[{A}, {a}] = {A} → {self.produccion(prod)}\nusada {usos} vez/veces")
                if usos:
                    k = 0.35 + 0.65 * usos / mx
                    it.setBackground(QColor(int(14 + 20 * k), int(70 + 60 * k), int(110 + 80 * k)))
                    it.setForeground(QColor("#fff"))
                else:
                    it.setForeground(QColor("#777"))
                self.t_mat.setItem(i, j, it)
        self.t_mat.resizeColumnsToContents()

    def actualizar(self):
        self.pasos, celdas = self.analizar(self.chk.isChecked())
        t = self.t_traza
        t.setUpdatesEnabled(False)
        t.setRowCount(len(self.pasos))
        errores = 0
        for r, p in enumerate(self.pasos):
            for c, val in enumerate((str(r + 1), p["pila"], p["entrada"], p["accion"])):
                it = QTableWidgetItem(val)
                if c == 3 and val.startswith("error"):
                    it.setForeground(QColor("#f48771"))
                elif c == 3 and val.startswith("match"):
                    it.setForeground(QColor("#89d185"))
                elif c == 3 and val == "aceptar":
                    it.setForeground(QColor("#4fc1ff"))
                t.setItem(r, c, it)
            errores += p["accion"].startswith("error")
        t.setUpdatesEnabled(True)
        t.resizeColumnsToContents()
        t.setColumnWidth(1, min(t.columnWidth(1), 340))
        t.setColumnWidth(2, min(t.columnWidth(2), 230))
        self._llenar_matriz(celdas)
        fin = self.pasos[-1]["accion"] if self.pasos else ""
        self.resumen.setText(f"{len(self.pasos)} pasos · {len(celdas)} celdas usadas · "
                             f"{errores} recuperaciones · {'aceptado' if fin == 'aceptar' else 'parcial'}")

    def _sel_paso(self):
        r = self.t_traza.currentRow()
        if not (0 <= r < len(self.pasos)):
            return
        p = self.pasos[r]
        if p.get("celda"):
            A, a = p["celda"]
            if A in self.no_terminales and a in self.terminales:
                self.t_mat.setCurrentCell(self.no_terminales.index(A), self.terminales.index(a))
        if p.get("pos") is not None:
            self.ir_a_pos.emit(p["pos"])

    def _exportar_traza(self):
        ruta, _ = QFileDialog.getSaveFileName(self, "Exportar traza LL(1)", "traza_ll1.html",
                                              "HTML (*.html);;CSV (*.csv)")
        if not ruta:
            return
        filas = [(str(i + 1), p["pila"], p["entrada"], p["accion"]) for i, p in enumerate(self.pasos)]
        cab = ("Paso", "Pila", "Entrada restante", "Acción")
        if ruta.endswith(".csv"):
            with open(ruta, "w", newline="", encoding="utf-8") as f:
                csv.writer(f).writerows([cab, *filas])
            return
        with open(ruta, "w", encoding="utf-8") as f:
            f.write("<html><meta charset='utf-8'><style>body{font-family:monospace}"
                    "td,th{border:1px solid #999;padding:2px 6px}table{border-collapse:collapse}"
                    "</style><h2>Traza LL(1)</h2><table><tr>")
            f.write("".join(f"<th>{c}</th>" for c in cab) + "</tr>")
            for fl in filas:
                f.write("<tr>" + "".join(f"<td>{html.escape(c)}</td>" for c in fl) + "</tr>\n")
            f.write("</table></html>")

    def _exportar_matriz(self):
        ruta, _ = QFileDialog.getSaveFileName(self, "Exportar matriz LL(1)", "tabla_ll1.csv", "CSV (*.csv)")
        if not ruta:
            return
        with open(ruta, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(["M[A,a]", *self.terminales])
            for A in self.no_terminales:
                w.writerow([A, *(self.produccion(self.tabla_ll1[A][a]) if a in self.tabla_ll1[A] else ""
                                 for a in self.terminales)])


# ---------------------------------------------------------------------------
# Tabla de símbolos
# ---------------------------------------------------------------------------
class VistaSimbolos(QWidget):
    ir_a = Signal(int, int)
    COLS = ["NOMBRE", "CLASE", "TIPO", "ÁMBITO", "FUNCIÓN", "NIVEL", "DIRECCIÓN", "LÍNEA:COL", "ARCHIVO"]

    def __init__(self):
        super().__init__()
        self.datos = []
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        self.filtro = QLineEdit()
        self.filtro.setPlaceholderText("Filtrar por nombre, tipo o función…")
        self.filtro.textChanged.connect(self._llenar)
        self.clase = QComboBox()
        self.clase.addItems(["todas", "variable", "parametro", "funcion"])
        self.clase.currentTextChanged.connect(self._llenar)
        self.cuenta = titulo("")
        v.addLayout(fila(self.filtro, self.clase, self.cuenta))
        self.t = tabla(self.COLS)
        self.t.cellDoubleClicked.connect(self._ir)
        v.addWidget(self.t, 1)

    def cargar(self, simbolos):
        self.datos = simbolos or []
        self._llenar()

    @staticmethod
    def direccion(s):
        if s.get("clase") == "funcion":
            return f"marco {s.get('marco', 0)} B"
        off = int(s.get("offset", 0))
        return f"{s.get('base')}{'+' if off >= 0 else '-'}{abs(off)}"

    def _visibles(self):
        q, cl = self.filtro.text().lower(), self.clase.currentText()
        for s in self.datos:
            if cl != "todas" and s.get("clase") != cl:
                continue
            if q and not any(q in str(s.get(k, "")).lower() for k in ("nombre", "tipo", "funcion")):
                continue
            yield s

    def _llenar(self):
        vis = list(self._visibles())
        self.t.setRowCount(len(vis))
        colores = {"funcion": "#dcdcaa", "parametro": "#9cdcfe", "variable": "#4fc1ff"}
        for r, s in enumerate(vis):
            vals = (s.get("nombre"), s.get("clase"), s.get("tipo"), s.get("ambito"), s.get("funcion"),
                    s.get("nivel"), self.direccion(s), f"{s.get('linea')}:{s.get('col')}",
                    str(s.get("archivo", "")).split("/")[-1])
            for c, val in enumerate(vals):
                it = QTableWidgetItem("" if val is None else str(val))
                if c == 0:
                    it.setForeground(QColor(colores.get(s.get("clase"), "#ccc")))
                it.setData(Qt.ItemDataRole.UserRole, s)
                self.t.setItem(r, c, it)
        self.t.resizeColumnsToContents()
        self.cuenta.setText(f"{len(vis)}/{len(self.datos)} símbolos")

    def _ir(self, r, _c):
        it = self.t.item(r, 0)
        s = it.data(Qt.ItemDataRole.UserRole) if it else None
        if s:
            self.ir_a.emit(int(s.get("linea") or 1), int(s.get("col") or 1))


# ---------------------------------------------------------------------------
# IR de 3 direcciones antes / después
# ---------------------------------------------------------------------------
class TacHighlighter(QSyntaxHighlighter):
    def __init__(self, doc):
        super().__init__(doc)

        def fmt(c, bold=False):
            f = QTextCharFormat()
            f.setForeground(QColor(c))
            if bold:
                f.setFontWeight(QFont.Weight.Bold)
            return f
        self.reglas = [
            (re.compile(r"\b[A-Za-z_][\w.]*\b"), fmt("#9cdcfe")),
            (re.compile(r"(?<![\w.])-?(0x[0-9A-Fa-f]+|\d+)\b"), fmt("#b5cea8")),
            (re.compile(r"\b(if|ifnot|goto|call|param|return|terminar|auth|logout|rdsr|loadkey|"
                        r"fround|clear)\b"), fmt("#c586c0")),
            (re.compile(r"\bM\[|\]"), fmt("#4ec9b0")),
            (re.compile(r"^L_\w+:"), fmt("#d7ba7d", True)),
            (re.compile(r"^funcion .*"), fmt("#dcdcaa", True)),
            (re.compile(r";.*$"), fmt("#6a9955")),
        ]

    def highlightBlock(self, text):
        for rx, f in self.reglas:
            for m in rx.finditer(text):
                self.setFormat(m.start(), m.end() - m.start(), f)


class VistaIR(QWidget):
    def __init__(self):
        super().__init__()
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        sp = QSplitter()
        self.cajas, self.titulos = [], []
        for nombre in ("TAC recién generado (antes)", "TAC tras las pasadas (después)"):
            w = QWidget()
            l = QVBoxLayout(w)
            l.setContentsMargins(0, 0, 0, 0)
            t = titulo(nombre)
            e = QPlainTextEdit()
            e.setReadOnly(True)
            e.setLineWrapMode(QPlainTextEdit.LineWrapMode.NoWrap)
            e.setFont(mono(10))
            e._hl = TacHighlighter(e.document())
            l.addWidget(t)
            l.addWidget(e, 1)
            sp.addWidget(w)
            self.cajas.append(e)
            self.titulos.append((t, nombre))
        self.sinc = QCheckBox("Desplazamiento sincronizado")
        self.sinc.setChecked(True)
        a, b = (c.verticalScrollBar() for c in self.cajas)
        a.valueChanged.connect(lambda val: self.sinc.isChecked() and b.setValue(val))
        b.valueChanged.connect(lambda val: self.sinc.isChecked() and a.setValue(val))
        v.addLayout(fila(self.sinc, None))
        v.addWidget(sp, 1)

    def cargar(self, ir):
        ir = ir or {}
        if not ir.get("con_opt", True) and ir.get("p1") is not None:
            # Sin la tuberia optimizadora: a la izquierda el IR lineal de P1.
            p1 = ir.get("p1") or ""
            n = sum(1 for ln in p1.splitlines() if ln.startswith("    "))
            (t0, _), (t1, _) = self.titulos
            self.cajas[0].setPlainText(p1 or "(sin IR: el programa tiene errores)")
            t0.setText(f"IR lineal del generador de P1 · {n} instrucciones")
            self.cajas[1].setPlainText("(el TAC y sus pasadas llegan con la tubería optimizadora\n"
                                       " -O0/-O1/-O2; este build/jaf todavía no la trae)")
            t1.setText("TAC optimizado · no disponible en esta versión")
            return
        for k, (caja, (t, nombre)) in zip(("antes", "despues"), zip(self.cajas, self.titulos)):
            texto = ir.get(k) or ""
            caja.setPlainText(texto or "(sin TAC: el programa tiene errores)")
            n = sum(1 for ln in texto.splitlines() if ln.startswith("    "))
            t.setText(f"{nombre} · {n} instrucciones")

    def texto(self):
        return self.cajas[0].toPlainText(), self.cajas[1].toPlainText()


# ---------------------------------------------------------------------------
# Paquetes VLIW coloreados por unidad funcional
# ---------------------------------------------------------------------------
UNIDADES = (("S0 · ALU", "#264f78"), ("S1 · LSU", "#2e6b4a"),
            ("S2 · BRU", "#7a5a1e"), ("S3 · CRIPTO", "#6b2e6b"))
COLOR_NOP = "#262626"


class TablaPaquetes(QWidget):
    ir_a = Signal(int, int)

    def __init__(self, nombre):
        super().__init__()
        self.nombre = nombre
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        self.tit = titulo(nombre)
        v.addWidget(self.tit)
        self.t = tabla(["DIR", "ETIQUETA", *[u for u, _ in UNIDADES]])
        self.t.setFont(mono(9))
        self.t.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectItems)
        self.t.cellDoubleClicked.connect(self._ir)
        v.addWidget(self.t, 1)
        self.lineas = []

    def cargar(self, bundles):
        bundles = bundles or []
        t = self.t
        t.setUpdatesEnabled(False)
        t.setRowCount(len(bundles))
        self.lineas = []
        usados = 0
        por_unidad = [0, 0, 0, 0]
        for r, b in enumerate(bundles):
            t.setItem(r, 0, QTableWidgetItem(f"{b.get('dir', 0):05x}"))
            etq = QTableWidgetItem(", ".join(b.get("etq") or []))
            etq.setForeground(QColor("#d7ba7d"))
            t.setItem(r, 1, etq)
            lin = b.get("lineas") or [0, 0, 0, 0]
            self.lineas.append(lin)
            for s, txt in enumerate(b.get("slots") or ["", "", "", ""]):
                it = QTableWidgetItem(txt or "NOP")
                if txt:
                    usados += 1
                    por_unidad[s] += 1
                    it.setBackground(QColor(UNIDADES[s][1]))
                    it.setForeground(QColor("#fff"))
                    if lin[s]:
                        it.setToolTip(f"línea {lin[s]} del fuente")
                else:
                    it.setBackground(QColor(COLOR_NOP))
                    it.setForeground(QColor("#666"))
                t.setItem(r, 2 + s, it)
        t.setUpdatesEnabled(True)
        t.resizeColumnsToContents()
        t.setColumnWidth(1, min(t.columnWidth(1), 110))
        n = len(bundles)
        occ = 100.0 * usados / (4 * n) if n else 0.0
        det = " · ".join(f"{u.split('·')[1].strip()} {c}" for (u, _), c in zip(UNIDADES, por_unidad))
        self.tit.setText(f"{self.nombre} · {n} bundles · {usados} instr. · {occ:.1f}% slots · {det}")

    def _ir(self, r, c):
        if 2 <= c <= 5 and r < len(self.lineas) and self.lineas[r][c - 2]:
            self.ir_a.emit(self.lineas[r][c - 2], 1)


class VistaPaquetes(QWidget):
    def __init__(self):
        super().__init__()
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        ley = QHBoxLayout()
        ley.setContentsMargins(6, 4, 6, 4)
        for u, c in (*UNIDADES, ("NOP", COLOR_NOP)):
            l = QLabel(f" {u} ")
            l.setStyleSheet(f"background:{c};color:#fff;border-radius:3px;padding:2px 6px;")
            ley.addWidget(l)
        ley.addStretch(1)
        self.modo = QComboBox()
        self.modo.addItems(["Lado a lado", "Solo optimizado", "Solo P1"])
        self.modo.currentIndexChanged.connect(self._modo)
        self.modo.setToolTip("doble clic en un slot: ir a su línea del fuente")
        ley.addWidget(self.modo)
        v.addLayout(ley)
        sp = QSplitter()
        self.base = TablaPaquetes("Generador de P1 (antes)")
        self.opt = TablaPaquetes("Tubería optimizadora (después)")
        sp.addWidget(self.base)
        sp.addWidget(self.opt)
        v.addWidget(sp, 1)

    def _modo(self, k):
        self.base.setVisible(k != 1)
        self.opt.setVisible(k != 2)

    def cargar(self, paquetes, con_opt=True):
        paquetes = paquetes or {}
        self.base.cargar(paquetes.get("base"))
        self.opt.cargar(paquetes.get("opt"))
        if not con_opt:
            self.opt.tit.setText(f"{self.opt.nombre} · no disponible en esta versión")
            self.modo.setCurrentIndex(2)  # Solo P1


# ---------------------------------------------------------------------------
# Métricas: barras nativas con QPainter
# ---------------------------------------------------------------------------
class GraficaBarras(QWidget):
    """Filas (etiqueta, antes, después, menor_es_mejor). Cada par se normaliza a su
    máximo para poder comparar magnitudes distintas en la misma gráfica."""

    def __init__(self, titulo_txt, series=("Antes", "Después"), colores=("#6e7681", "#0e8ad8")):
        super().__init__()
        self.titulo, self.series, self.colores = titulo_txt, series, colores
        self.filas = []
        self.setMinimumHeight(80)

    def cargar(self, filas):
        self.filas = filas
        self.setMinimumHeight(60 + 34 * len(filas))
        self.update()

    @staticmethod
    def fmt(v):
        if isinstance(v, float) and not v.is_integer():
            return f"{v:.3f}" if abs(v) < 10 else f"{v:.1f}"
        return f"{int(v):,}".replace(",", " ")

    def paintEvent(self, _):
        p = QPainter(self)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)
        p.fillRect(self.rect(), QColor("#1e1e1e"))
        p.setPen(QColor("#ddd"))
        p.setFont(QFont("DejaVu Sans", 10, QFont.Weight.Bold))
        p.drawText(10, 20, self.titulo)
        p.setFont(QFont("DejaVu Sans", 8))
        x = 10 + p.fontMetrics().horizontalAdvance(self.titulo) + 160
        for nombre, c in zip(self.series, self.colores):
            p.fillRect(int(x), 11, 10, 10, QColor(c))
            p.setPen(QColor("#bbb"))
            p.drawText(int(x) + 14, 20, nombre)
            x += p.fontMetrics().horizontalAdvance(nombre) + 30
        etq_w, der_w = 190, 70
        ancho = max(40, self.width() - etq_w - der_w - 120)
        y = 34
        for fila_ in self.filas:
            nombre, valores, menor = fila_[0], fila_[1:-1], fila_[-1]
            mx = max([abs(v) for v in valores if v is not None] + [1e-9])
            p.setPen(QColor("#ccc"))
            p.drawText(10, y + 14, nombre)
            alto = 22 // len(valores)
            for k, v in enumerate(valores):
                if v is None:
                    continue
                w = int(ancho * abs(v) / mx)
                yy = y + 2 + k * (alto + 1)
                p.fillRect(etq_w, yy, max(2, w), alto, QColor(self.colores[k % len(self.colores)]))
                p.setPen(QColor("#ddd"))
                p.drawText(etq_w + max(2, w) + 6, yy + alto - 2, self.fmt(v))
            if len(valores) == 2 and None not in valores and valores[0]:
                d = 100.0 * (valores[1] - valores[0]) / abs(valores[0])
                mejora = (d < 0) == bool(menor) if d else None
                p.setPen(QColor("#89d185" if mejora else "#f48771" if mejora is not None else "#999"))
                p.drawText(self.width() - der_w, y + 14, f"{d:+.1f}%")
            y += 30
        p.end()


class VistaMetricas(QWidget):
    def __init__(self):
        super().__init__()
        cont = QWidget()
        v = QVBoxLayout(cont)
        self.resumen = QLabel("Inspecciona un archivo (F6) para ver las métricas.")
        self.resumen.setWordWrap(True)
        self.resumen.setStyleSheet("color:#ccc;padding:6px;")
        self.g_comp = GraficaBarras("Generador de P1 vs tubería optimizadora",
                                    ("P1 (antes)", "Optimizado (después)"))
        self.g_tac = GraficaBarras("TAC antes vs después de las pasadas",
                                   ("TAC generado", "TAC optimizado"))
        self.g_pasadas = GraficaBarras("Trabajo de cada pasada", ("cantidad",), ("#c586c0",))
        self.g_rtl = GraficaBarras("Simulación RTL: pulsa Simular RTL (F8)",
                                   ("P1 (antes)", "Optimizado (después)"), ("#6e7681", "#d7ba7d"))
        for w in (self.resumen, self.g_rtl, self.g_comp, self.g_tac, self.g_pasadas):
            v.addWidget(w)
        v.addStretch(1)
        sc = QScrollArea()
        sc.setWidgetResizable(True)
        sc.setWidget(cont)
        lay = QVBoxLayout(self)
        lay.setContentsMargins(0, 0, 0, 0)
        lay.addWidget(sc)
        self.rtl = {}

    def cargar(self, metricas, flags=""):
        m = metricas or {}
        b, o = m.get("base") or {}, m.get("opt") or {}
        if b.get("ok") and not o.get("ok") and "sim" not in b:
            self._cargar_solo_p1(b, o.get("error", ""))
            return
        self.g_comp.titulo = "Generador de P1 vs tubería optimizadora"
        self.g_comp.series, self.g_comp.colores = ("P1 (antes)", "Optimizado (después)"), ("#6e7681", "#0e8ad8")
        if not (b.get("ok") and o.get("ok")):
            self.resumen.setText("Sin métricas: " + (b.get("error") or o.get("error") or "el programa tiene errores"))
            for g in (self.g_comp, self.g_tac, self.g_pasadas):
                g.cargar([])
            return
        bs, os_ = b["sim"], o["sim"]
        e = o.get("estad") or {}
        acel = bs["ciclos"] / os_["ciclos"] if os_["ciclos"] else 0
        self.resumen.setText(
            f"<b>{flags or '-O0'}</b> · ciclos {bs['ciclos']} → {os_['ciclos']} "
            f"(<b>{acel:.2f}×</b>) · IPC {bs['ipc']:.3f} → {os_['ipc']:.3f} · "
            f"{b['bytes']} → {o['bytes']} bytes · simulador rápido "
            f"(memoria ideal; la cifra con caché es la de tb_top)")
        self.g_comp.cargar([
            ("Ciclos simulados", bs["ciclos"], os_["ciclos"], True),
            ("Instrucciones (código)", b["instrucciones"], o["instrucciones"], True),
            ("Instrucciones ejecutadas", bs["instrucciones"], os_["instrucciones"], True),
            ("Bundles", b["bundles"], o["bundles"], True),
            ("Tamaño (bytes)", b["bytes"], o["bytes"], True),
            ("IPC", bs["ipc"], os_["ipc"], False),
            ("Lecturas de memoria", bs["lecturas"], os_["lecturas"], True),
            ("Escrituras de memoria", bs["escrituras"], os_["escrituras"], True),
            ("Tiempo de compilación (ms)", b["tiempo_ms"], o["tiempo_ms"], True),
        ])
        self.g_tac.cargar([
            ("Instrucciones TAC", e.get("instrucciones_antes", 0), e.get("instrucciones_despues", 0), True),
        ])
        self.g_pasadas.cargar([
            ("Eliminadas (DCE)", e.get("eliminadas_dce", 0), None),
            ("Lazos desenrollados", e.get("lazos_desenrollados", 0), None),
            ("Copias de cuerpo (unroll)", e.get("iteraciones_unroll", 0), None),
            ("Renombradas", e.get("renombradas", 0), None),
            ("Reordenadas", e.get("reordenadas", 0), None),
            ("Virtuales derramados", e.get("virtuales_derramados", 0), None),
            ("Spills insertados", e.get("spills_insertados", 0), None),
        ])
        self.cargar_rtl(self.rtl)

    def _cargar_solo_p1(self, b, motivo):
        """Sin tuberia optimizadora ni simulador rapido: metricas estaticas del generador de P1."""
        n = b.get("bundles", 0)
        occ = 100.0 * b.get("slots_usados", 0) / (4 * n) if n else 0.0
        self.resumen.setText(
            f"<b>P1</b> · {b.get('instrucciones', 0)} instrucciones en {n} bundles "
            f"({occ:.1f}% de slots ocupados) · {b.get('bytes', 0)} bytes · "
            f"compilado en {b.get('tiempo_ms', 0):.3f} ms<br>"
            f"<span style='color:#999'>Optimizado: {motivo or 'no disponible'}.</span>")
        self.g_comp.titulo = "Generador de P1"
        self.g_comp.series, self.g_comp.colores = ("P1",), ("#6e7681",)
        self.g_comp.cargar([
            ("Instrucciones (código)", b.get("instrucciones", 0), True),
            ("Bundles", n, True),
            ("Slots usados", b.get("slots_usados", 0), True),
            ("Slots con NOP", 4 * n - b.get("slots_usados", 0), True),
            ("Tamaño (bytes)", b.get("bytes", 0), True),
            ("Ocupación de slots (%)", occ, False),
            ("Tiempo de compilación (ms)", b.get("tiempo_ms", 0.0), True),
        ])
        for g in (self.g_tac, self.g_pasadas):
            g.cargar([])
        self.cargar_rtl(self.rtl)

    def cargar_rtl(self, rtl):
        """rtl = {"base": {...}, "opt": {...}} con lo que se lee de HALT/PERF de tb_top."""
        self.rtl = rtl or {}
        b, o = self.rtl.get("base"), self.rtl.get("opt")
        if not (b and o):
            self.g_rtl.cargar([])
            return
        filas = [("Ciclos RTL", b.get("cycles"), o.get("cycles"), True),
                 ("IPC RTL", b.get("ipc"), o.get("ipc"), False),
                 ("Hit rate L1 (%)", b.get("l1"), o.get("l1"), False),
                 ("Hit rate L2 (%)", b.get("l2"), o.get("l2"), False),
                 ("Ciclos de stall L1-D", b.get("stall_l1d"), o.get("stall_l1d"), True),
                 ("Accesos a DRAM", b.get("dram"), o.get("dram"), True)]
        ok = "check_dump PASS" if b.get("pasa") and o.get("pasa") else "check_dump FALLÓ"
        self.g_rtl.titulo = f"Simulación RTL · {self.rtl.get('config', 'tb_top')} · {ok}"
        self.g_rtl.cargar([f for f in filas if f[1] is not None and f[2] is not None])
