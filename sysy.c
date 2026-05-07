#include <stdio.h>
#include <string.h>
#include "da.h"
#include "lex.h"
#include "ast.h"
#include "kir.h"
#include "include/koopa.h"

static char* USAGE = "Usage: sysy -koopa input -o output";

int main(int argc, char** argv) {
    char* name = shift_args(argc, argv);
    bool is_koopa = false;
    char* input = nullptr;
    char* output = nullptr;
    panic_if(argc < 1, USAGE);
    do {
        char* option = shift_args(argc, argv);
        if (strncmp(option, "-", 1) == 0) {
            if (strcmp(option, "-koopa") == 0) {
                is_koopa = true;
                continue;
            }
            if (strcmp(option, "-o") == 0) {
                panic_if(argc < 1, "Error: Missing ouput file\n%s", USAGE);
                output = shift_args(argc, argv);
            }
        } else input = option;
    } while (argc > 0);
    panic_if(is_koopa == false, "Error: Missing option '-koopa'\n%s", USAGE);
    panic_if(input == nullptr, "Error: Missing input file\n%s", USAGE);
    panic_if(output == nullptr, "Error: Missing output file\n%s", USAGE);

    FILE* file = fopen(input, "rb");
    panic_if(file == nullptr, "Error: failed to open file");
    panic_if(fseek(file, 0, SEEK_END) != 0, "Error: failed to get file size");
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    Str src = {0};
    DA_resize(&src, size + 1);
    fread(src.items, size, 1, file);
    src.length = size;
    DA_append(&src, '\0');
    printf("%s\n", src.items);

    char* program = src.items;
    Tokens tokens = {0};
    Lexer lexer = LEX_new(program);
    LEX_parse(&lexer, &tokens);
    Str buf = {0};
    printf("=================TOKEN==================\n");
    DA_foreach(&tokens, token) {
        size_t idx = token - tokens.items;
        TOKEN_to_str(token, &buf);
        printf("%zu:%zu:%zu:%s\n", idx, token->loc.row, token->loc.col, buf.items);
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
    err = koopa_dump_to_file(kp, output);
    KIR_handle_err_code(err);
    koopa_delete_program(kp);
    return 0;
}
