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
    koopa_raw_program_t raw_prog = { 
        .values = KIR_new_slice(KOOPA_RSIK_VALUE), 
        .funcs = KIR_new_slice(KOOPA_RSIK_FUNCTION) 
    };
    koopa_raw_slice_t used_by = KIR_new_slice(KOOPA_RSIK_VALUE);
    koopa_raw_slice_t params = KIR_new_slice(KOOPA_RSIK_VALUE);
    koopa_raw_slice_t func_params_type = KIR_new_slice(KOOPA_RSIK_TYPE);
    struct koopa_raw_type_kind int_type = { .tag = KOOPA_RTT_INT32 };
    struct koopa_raw_type_kind func_type = {
        .tag = KOOPA_RTT_FUNCTION,
        .data.function = {
            .params = func_params_type,
            .ret = &int_type,
        }
    };
    struct koopa_raw_type_kind unit_type = { .tag = KOOPA_RTT_UNIT };
    struct koopa_raw_value_data val = {
        .ty = &int_type,
        .name = nullptr,
        .used_by = used_by,
        .kind = (koopa_raw_value_kind_t) { 
            .tag = KOOPA_RVT_INTEGER, 
            .data = { 
                .integer = (koopa_raw_integer_t) {.value = 0}
            }
        }
    };
    koopa_raw_value_kind_t kind = {
        .tag = KOOPA_RVT_RETURN,
        .data = { .ret =  { .value = &val } }
    };
    struct koopa_raw_value_data ret = {
        .ty = &unit_type,
        .name = nullptr,
        .used_by = used_by,
        .kind = kind,
    };
    koopa_raw_basic_block_data_t block = {
        .name = nullptr,
        .params = params,
        .used_by = used_by,
        .insts = KIR_new_slice(KOOPA_RSIK_VALUE),
    };
    KIR_slice_append_ptr(&block.insts, &ret);
    koopa_raw_function_data_t main_func = { 
        // type is function signature
        // for main is () -> i32
        .ty =  &func_type, 
        // name should start with @ or %
        .name = "@main", 
        .params = params, 
        .bbs = KIR_new_slice(KOOPA_RSIK_BASIC_BLOCK),
    };
    KIR_slice_append_ptr(&main_func.bbs, &block);
    KIR_slice_append_ptr(&raw_prog.funcs, &main_func);
    koopa_program_t kp = {0};
    koopa_error_code_t err = koopa_generate_raw_to_koopa(&raw_prog,&kp);
    KIR_handle_err_code(err);
    err = koopa_dump_to_stdout(kp);
    KIR_handle_err_code(err);
    koopa_delete_program(kp);
    return 0;
}