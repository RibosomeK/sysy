#include <stdio.h>
#include <string.h>
#include "da.h"
#include "lex.h"
// #include "ast.h"

int main() {
    char* program = "int main() {return 0;}";
    Tokens tokens = {0};
    Lexer lexer = {.src = program, .pos = 0, .len = strlen(program)};
    LEX_parse(&lexer, &tokens);
    TOKEN_display(&tokens);
    // Parser parser = {.tokens = &tokens, .pos = 0};
    // Nodes nodes = {0};
    // AST_parse(&parser, &nodes);
    // DA_foreach(&nodes, node) {
    //     NODE_display(node);
    // }
    return 0;
}