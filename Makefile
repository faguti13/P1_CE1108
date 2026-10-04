# Compilador JAF (CE1108).
#
#   make            compila build/jaf
#   make prueba     vuelca los tokens de ejemplos/prueba.oly
#   make arbol      imprime el arbol de ese mismo programa
#   make clean

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -Isrc -Ibuild
FLEX    = flex
BISON   = bison

.PHONY: all prueba arbol clean

all: build/jaf

build:
	mkdir -p build

build/jaf: build/main.o build/lexer.o build/parser.o build/ast.o build/tokens.o
	$(CC) $(CFLAGS) -o $@ $^

build/main.o: src/main.c src/tokens.h src/comun.h src/ast.h | build
	$(CC) $(CFLAGS) -c src/main.c -o $@

build/tokens.o: src/tokens.c src/tokens.h src/comun.h | build
	$(CC) $(CFLAGS) -c src/tokens.c -o $@

build/ast.o: src/ast.c src/ast.h | build
	$(CC) $(CFLAGS) -c src/ast.c -o $@

build/lexer.c: src/lexer.l src/tokens.h src/comun.h | build
	$(FLEX) -o build/lexer.c src/lexer.l

build/lexer.o: build/lexer.c
	$(CC) $(CFLAGS) -c build/lexer.c -o $@

build/parser.c build/parser.h: src/parser.y src/comun.h src/ast.h | build
	$(BISON) -d -v -Wall -o build/parser.c src/parser.y

build/parser.o: build/parser.c src/ast.h src/comun.h
	$(CC) $(CFLAGS) -c build/parser.c -o $@

prueba: build/jaf
	./build/jaf -v ejemplos/prueba.oly

arbol: build/jaf
	./build/jaf --arbol ejemplos/prueba.oly

clean:
	rm -rf build
