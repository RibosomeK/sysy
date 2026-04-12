#include <stdio.h>
#include <string.h>
#include "da.h"
#include "lex.h"
#include "ast.h"
#include "kir.h"
#include "include/koopa.h"

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
    printf("================KOOPA===================\n");
    koopa_program_t kp = {0};
    koopa_error_code_t ret = koopa_parse_from_string(buf.items,&kp);
    panic_if(ret != KOOPA_EC_SUCCESS, "ERROR: failed to read koopa ir string");
    STR_clear(&buf);
    size_t write_len = buf.capacity;
    ret = koopa_dump_to_string(kp, buf.items, &write_len);
    panic_if(ret != KOOPA_EC_SUCCESS, "Error: failed to dump koopa ir string");
    printf("%s\n", buf.items);
    koopa_delete_program(kp);
    return 0;
}