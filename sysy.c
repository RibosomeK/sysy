#include <stdio.h>
#include <string.h>
#include "da.h"
#include "lex.h"

int main() {
    char* program = "int main() { return 0; }";
    Str buf = {0};
    Tokens tokens = {0};
    Lexer lexer = {.src = program, .pos = 0, .len = strlen(program)};
    LEX_parse(&lexer, &tokens, &buf);
    DA_foreach(&tokens, token) {
        size_t idx = token - tokens.items;
        printf("%zu:%s\n", idx, *token);
    }
    return 0;
}