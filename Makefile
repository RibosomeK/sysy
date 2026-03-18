all: build

build: da.h lex.h sysy.c
	gcc -std=c23 -g -O0 -Wall -Wextra sysy.c -o ./build/sysy

run: build
	./build/sysy