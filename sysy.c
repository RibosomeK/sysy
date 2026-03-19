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
        switch (token->type) {
        case IDENT:
        case PUNC:
            printf("%zu:%s\n", idx, token->as.string);
            break;
        case INT:
            printf("%zu:%d\n", idx, token->as.integer);
            break;
        }
    }
    return 0;
}