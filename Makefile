all: build

build: sysy.c
	gcc -g -O0 -Wall -Wextra sysy.c -o ./build/sysy

run: build
	./build/sysy