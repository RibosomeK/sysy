#include <stdio.h>
#include <string.h>
#include "da.h"
#include "lex.h"
#include "ast.h"

void TOKEN_display(Tokens* tokens) {
    DA_foreach(tokens, token) {
        size_t idx = token - tokens->items;
        switch (token->type) {
        case TOK_IDENT:
        case TOK_PUNC:
        case TOK_EOF:
            printf("%zu:%s\n", idx, token->as.string);
            break;
        case TOK_INT:
            printf("%zu:%d\n", idx, token->as.integer);
            break;
        }
    }
}

int main() {
    char* program = "return 0;";
    Str buf = {0};
    Tokens tokens = {0};
    Lexer lexer = {.src = program, .pos = 0, .len = strlen(program)};
    LEX_parse(&lexer, &tokens, &buf);
    TOKEN_display(&tokens);
    Parser parser = {.tokens = &tokens, .pos = 0};
    Nodes nodes = {0};
    AST_parse(&parser, &nodes);
    DA_foreach(&nodes, node) {
        NODE_display(node);
    }
    return 0;
}