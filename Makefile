# Compilador JAF (CE1108) -> procesador VLIW Feistel4 (CE4301).
#
# Targets:
#   make              compila build/jaf
#   make test         pruebas del compilador (aceptados, errores, golden, xcheck)
#   make xcheck       alias de make test (la prueba cruzada va dentro de run_tests.sh)
#   make e2e          compila y ejecuta en el RTL (VLIW_DIR=../vliw_grupo2)
#   make ejercicios   compila los ejercicios y versiona la salida en ejercicios/salida/
#   make asan         recompila con AddressSanitizer/UBSan y repite make test
#   make sync-isa     copia la ISA del repo del procesador a ./isa.md
#   make pdf          regenera docs/04_ejemplos.md y arma docs/JAF-compilador.pdf
#   make prueba       compila ejemplos/prueba.oly mostrando el AST (-t) y las fases (-v)
#   make arbol        imprime el arbol de ese mismo programa
#   make clean        borra build/

# Variables de compilación y herramientas.
# EXTRA está vacía por defecto; make asan la usa para agregar los sanitizers
# sin modificar CFLAGS.
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -Isrc -Ibuild $(EXTRA)
FLEX    = flex
BISON   = bison
# Ruta del repositorio del procesador VLIW Feistel4.
# ?= usa este valor solo si no se pasa otra ruta.
VLIW_DIR ?= ../vliw_grupo2

# Lista de objetos, para el target build/jaf.
# Se produce un objeto por cada módulo del compilador.
# Orden de las fases: CLI, léxico, sintáctico, AST, tokens, ISA, semántico, codegen,
# planificador, emisor del .asm y ensamblador propio (-x: de .asm a .bin).
OBJS = build/main.o build/lexer.o build/parser.o build/ast.o build/tokens.o \
       build/isa.o build/semantico.o build/codegen.o build/planificador.o \
       build/emisor.o build/ensamblador.o

.PHONY: all prueba arbol clean test e2e ejercicios asan sync-isa xcheck pdf

# Define el target por defecto como build/jaf.
all: build/jaf

# Crea la carpeta de salida build/ si no existe.
build:
	mkdir -p build

# Enlaza todos los objetos en el ejecutable del compilador.
build/jaf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Lista de todos los .h. Si cambia cualquiera, se recompilan todos los .c
# (incluyan o no ese .h): es simple y nunca deja un objeto desactualizado.
HDRS = src/ast.h src/comun.h src/isa.h src/semantico.h src/codegen.h src/planificador.h \
       src/emisor.h src/ensamblador.h src/tokens.h

# Regla genérica: cada src/X.c produce build/X.o y depende de todos los .h de HDRS.
build/%.o: src/%.c $(HDRS) | build
	$(CC) $(CFLAGS) -c $< -o $@

# FLEX recibe lexer.l el cual contiene las expresiones regulares de los tokens.
# Genera el analizador léxico build/lexer.c y lo compila a build/lexer.o.
build/lexer.c: src/lexer.l src/tokens.h src/comun.h | build
	$(FLEX) -o build/lexer.c src/lexer.l

build/lexer.o: build/lexer.c
	$(CC) $(CFLAGS) -c build/lexer.c -o $@

# BISON lee parser.y el cual contiene la gramática del lenguaje y las acciones semánticas.
# Genera el parser LALR en build/parser.c.
# -d genera build/parser.h con los códigos de los tokens (lo usa el lexer).
# -v deja build/parser.output para revisar que no haya conflictos.
build/parser.c build/parser.h: src/parser.y src/comun.h src/ast.h | build
	$(BISON) -d -v -Wall -o build/parser.c src/parser.y

build/parser.o: build/parser.c src/ast.h src/comun.h
	$(CC) $(CFLAGS) -c build/parser.c -o $@

# Corre el compilador sobre el ejemplo con -t (imprime el AST) y -v (muestra las fases).
prueba: build/jaf
	./build/jaf -t -v ejemplos/prueba.oly

# Parsea el ejemplo e imprime el árbol AST.
arbol: build/jaf
	./build/jaf --arbol ejemplos/prueba.oly

# Limpia la carpeta de salida build/ y los binarios generados.
clean:
	rm -rf build

# Corre tests/run_tests.sh sin uso del simulador. Debe terminar en "55 pasaron, 0 fallaron".
# Aceptados: compila cada .oly de tests/aceptados/, ejercicios/ y tests/e2e/ y verifica que no haya errores.
# Errores: cada .oly de tests/errores/ debe fallar y el mensaje debe contener el código de error indicado en su primera línea ("$ ESPERA: ERROR 11").
# Golden: el .asm generado debe ser idéntico a tests/golden/<nombre>.asm.
# Xcheck: prueba cruzada de ensambladores donde el binario del compilador, el de jaf -x y el de vliwasm.py deben ser iguales byte a byte. 
# Se hace con los .oly de ejercicios, e2e y golden, y con los .asm escritos a mano en vliw_grupo2/programs/.
# prueba.oly: debe dar 6 errores ERROR 14 (tipos no soportados) y aun así parsearse con --arbol.
test: build/jaf
	JAF=./build/jaf VLIW_DIR=$(VLIW_DIR) tests/run_tests.sh

# Define xcheck como alias de test.
xcheck: test

# Prueba de punta a punta sobre el hardware simulado (Icarus).
# Pasos:
# 1. Construye en vliw_grupo2 el testbench (tb_top.vvp) y la referencia en C (feistel4_ref).
# 2. Para cada .oly que tenga .expect, corre tests/e2e.sh: compila con jaf, convierte
#    a .mem, simula y compara registros y memoria contra el .expect.
# 3. Cifra un archivo de texto real con un programa JAF corriendo en el procesador y
#    lo compara byte por byte contra la referencia en C.
# En la receta, $$ es un $ para el shell (make interpreta el $ solo).
e2e: build/jaf
	$(MAKE) -s -C $(VLIW_DIR) build/tb_top.vvp build/feistel4_ref
	@set -e; for f in ejercicios/*.oly tests/e2e/*.oly; do \
	  [ -f $${f%.oly}.expect ] || continue; \
	  VLIW_DIR=$(VLIW_DIR) tests/e2e.sh $$f; \
	done
	VLIW_DIR=$(VLIW_DIR) tests/e2e_archivo.sh

# Compila los 6 ejercicios y guarda en ejercicios/salida/ el .asm, el .bin y los paquetes (-p). 
# Los que tienen .expect (5 de 6) también se corren en el RTL y se guardan su traza, lo esperado y el resultado (PASS/FAIL).
ejercicios: build/jaf
	$(MAKE) -s -C $(VLIW_DIR) build/tb_top.vvp
	VLIW_DIR=$(VLIW_DIR) tests/ejercicios.sh

# Detecta errores de memoria y comportamiento indefinido de C corriendo la batería de test.
# Pasos:
# 1. Hace un clean.
# 2. Recompila con AddressSanitizer y UBSan.
# 3. Corre make test con esa versión del compilador (detect_leaks=0 apaga solo el detector de fugas).
# 4. Hace otro clean.
# 5. Recompila la versión normal del compilador.
asan:
	$(MAKE) clean
	$(MAKE) EXTRA="-fsanitize=address,undefined -g -O1 -fno-omit-frame-pointer" build/jaf
	ASAN_OPTIONS=detect_leaks=0 $(MAKE) test
	$(MAKE) clean
	$(MAKE) build/jaf

# Copia isa.md desde vliw_grupo2 para que el compilador y el hardware usen la misma versión del contrato de la ISA.
sync-isa:
	cp $(VLIW_DIR)/docs/isa.md isa.md

# Capítulos del documento de entrega, en el orden del PDF.
DOCS = docs/01_arquitectura.md docs/02_lenguaje.md docs/03_algoritmos.md docs/04_ejemplos.md \
       docs/05_casos_especiales.md docs/uso.md

# Regenera docs/04_ejemplos.md y arma docs/JAF-compilador.pdf.
.PHONY: pdf
pdf:
	python3 tests/gen_04_ejemplos.py
	python3 docs/md2pdf.py docs/JAF-compilador.pdf "Compilador JAF para Feistel4-VLIW" $(DOCS)
