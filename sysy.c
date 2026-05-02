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
    STR_clear(&buf);
    AstProg prog = AST_parse_prog(&parser);
    AST_prog_to_str(&prog, &buf);
    printf("%s\n", buf.items);
    printf("================KOOPA===================\n");
    koopa_raw_program_t raw_prog = KIR_to_raw_prog(&prog);
    koopa_program_t kp = {0};
    koopa_error_code_t err = koopa_generate_raw_to_koopa(&raw_prog,&kp);
    KIR_handle_err_code(err);
    err = koopa_dump_to_stdout(kp);
    KIR_handle_err_code(err);
    koopa_delete_program(kp);
    return 0;
}
