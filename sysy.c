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
    auto raw_prog = KIR_new_raw_prog();
    auto used_by = KIR_new_slice(KOOPA_RSIK_VALUE);
    auto params = KIR_new_slice(KOOPA_RSIK_VALUE);
    auto func_params_type = KIR_new_slice(KOOPA_RSIK_TYPE);
    auto val = KIR_new_raw_int_data(0,  used_by);
    auto ret = KIR_new_raw_ret_data(&val, used_by); 
    auto block = KIR_new_raw_block_data(params, used_by);
    KIR_slice_append_ptr(&block.insts, &ret);
    auto func_type = KIR_new_raw_func_kind(func_params_type, &INT32_TYPE);
    auto main_func = KIR_new_raw_func_data(&func_type, "@main",  params);
    KIR_slice_append_ptr(&main_func.bbs, &block);
    KIR_slice_append_ptr(&raw_prog.funcs, &main_func);
    koopa_program_t kp = {0};
    auto err = koopa_generate_raw_to_koopa(&raw_prog,&kp);
    KIR_handle_err_code(err);
    err = koopa_dump_to_stdout(kp);
    KIR_handle_err_code(err);
    koopa_delete_program(kp);
    return 0;
}