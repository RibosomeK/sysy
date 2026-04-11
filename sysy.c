#include <stdio.h>
#include <string.h>
#include "da.h"
#include "lex.h"
#include "ast.h"
#include "kir.h"

int main() {
    char* program = "int main() { return 0; }";
    Tokens tokens = {0};
    Lexer lexer = {.src = program, .pos = 0, .len = strlen(program)};
    LEX_parse(&lexer, &tokens);
    Str buf = {0};
    printf("=================TOKEN==================\n");
    DA_foreach(&tokens, token) {
        size_t idx = token - tokens.items;
        TOKEN_to_str(token, &buf);
        printf("%zu:%s\n", idx, buf.items);
        STR_clear(&buf);
    }
    printf("=================AST====================\n");
    Parser parser = {.tokens = &tokens, .pos = 0};
    Nodes nodes = {0};
    STR_clear(&buf);
    while (PARSER_curr(&parser).type != TOK_EOF) {
        Node node = AST_parse(&parser);
        NODE_to_str(&node, &buf);
        printf("%s\n", buf.items);
        DA_append(&nodes, node);
        STR_clear(&buf);
    }
    printf("=================KIR====================\n");
    KirUnit unit = KIR_from_node(&nodes.items[0]);
    KIR_to_str(&unit, &buf, 0);
    printf("%s\n", buf.items);
    return 0;
}