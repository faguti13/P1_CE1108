# Prototipo de IDE JAF
  
La lógica de análisis sigue en **C** (`jaf --ide` → `src/ide.c`).

## Dependencia

```bash
sudo apt install python3-pyqt6
# o: pip install PyQt6
```

## Abrir

```bash
cd P1_CE1108
make ide
# o:
python3 ide/jaf_ide.py
python3 ide/jaf_ide.py /ruta/mi_proyecto
```

## Qué incluye la UI

- Explorador de **cualquier carpeta** (no solo este repo)
- Nuevo proyecto / nuevo `.oly`
- Pestañas con **×** para cerrar, Ctrl+W, ask al guardar
- Editor con números de línea y resaltado JAF
- Panel PROBLEMAS / SUGERENCIAS / CLONES
- Barra de estado azul

## Atajos

| Tecla | Acción |
|---|---|
| Ctrl+N | Nuevo archivo |
| Ctrl+Shift+N | Nuevo proyecto |
| Ctrl+O | Abrir archivo |
| Ctrl+Shift+O | Abrir carpeta |
| Ctrl+S | Guardar |
| Ctrl+W | Cerrar pestaña |
| F5 | Analizar |
| Ctrl+Espacio | Sugerencias |

## Features del enunciado (pendientes en C)

Rellenar `src/ide.c`: `emitir_diagnostics`, `emitir_fixes`, `emitir_completions`, `emitir_clones`.  
Hasta entonces las tablas salen vacías; la UI ya está lista para pintar el JSON.
