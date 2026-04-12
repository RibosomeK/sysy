all: build

build: da.h lex.h ast.h sysy.c
	gcc -std=c23 -g -O0 -Wall -Wextra sysy.c -o ./build/sysy -L./lib -lkoopa -lm

run: build
	./build/sysy