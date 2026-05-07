all: build

build: da.h lex.h ast.h kir.h sysy.c
	gcc -std=c23 -g -O0 -Wall -Wextra sysy.c -o ./build/sysy -L./lib -lkoopa -lm

run: build
	./build/sysy -koopa ./example/main.sysy -o ./out/main.koopa

GREEN = \033[32m
RESET = \033[0m
check:
	@echo "====================Checking with GCC===================="; \
	output=$$(                                                         \
		gcc -fsyntax-only -fdiagnostics-color=always                   \
		-std=c23 -g -O0 -Wall -Wextra sysy.c 2>&1                      \
	);                                                                 \
	if [ -z "$$output" ]; then                                         \
		printf "$(GREEN)All pass$(RESET)\n";                           \
	else                                                               \
		printf "%s\n" "$$output";                                      \
	fi
	@echo "===================Checking with Clang==================="; \
	output=$$(                                                         \
		clang -fsyntax-only -fdiagnostics-color=always                 \
		-std=c23 -g -O0 -Wall -Wextra sysy.c 2>&1                      \
	);                                                                 \
	if [ -z "$$output" ]; then                                         \
		printf "$(GREEN)All pass$(RESET)\n";                           \
	else                                                               \
		printf "%s\n" "$$output";                                      \
	fi