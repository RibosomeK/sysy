#ifndef LEX_H_
#define LEX_H_
#include "da.h"

typedef enum {
    TOK_IDENT,
    TOK_INT,
    TOK_PUNC,
    TOK_EOF,
} TokType;

typedef struct {
    TokType type;
    union {
        int     integer;
        StrView str_view;
    } as;
} Token;

typedef struct {
    Token* items;
    size_t length;
    size_t capacity;
} Tokens;

void TOKEN_display(Tokens* tokens) {
    DA_foreach(tokens, token) {
        size_t idx = token - tokens->items;
        switch (token->type) {
        case TOK_IDENT:
        case TOK_PUNC:
        case TOK_EOF:
            printf("%zu:", idx);
            fwrite(token->as.str_view.items, 1, token->as.str_view.length, stdout);
            printf("\n");
            break;
        case TOK_INT:
            printf("%zu:%d\n", idx, token->as.integer);
            break;
        }
    }
}

typedef struct {
    char*  src;
    size_t pos;
    size_t len;
} Lexer;

bool is_identifier_inital(char c) {
    int ord = (int)c;
    if (65 <= ord && ord <= 90)     return true;
    if (ord == 95)                  return true;
    if (97 <= ord && ord <= 122)    return true;
    return false;
}

bool is_identifier_follow(char c) {
    int ord = (int)c;
    if (48 <= ord && ord <= 57)     return true;
    if (65 <= ord && ord <= 90)     return true;
    if (ord == 95)                  return true;
    if (97 <= ord && ord <= 122)    return true;
    return false; 
}

bool is_whitespace(char c) {
    switch (c) {
        case ' ' : return true;
        case '\r': return true;
        case '\n': return true;
        case '\t': return true;
        case '\v': return true;
        default  : return false;
    }
}

bool is_digit(char c) {
    int ord = (int)c;
    if (48 <= ord && ord <= 57)     return true;
    return false;
}

StrView parse_ident(Lexer* lexer) {
    size_t length = 1;
    StrView sv = {.items=&lexer->src[lexer->pos]};
    lexer->pos += 1;
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_identifier_follow(curr)) {
            length += 1;
            lexer->pos += 1;
        } else {
            break;
        }
    }
    sv.length = length;
    return sv;
}

typedef enum {
    NUM_INT,
    NUM_FLOAT,
} NUM_TYPE;

typedef struct {
    NUM_TYPE type;
    union {
        int integer;
        float floating;
    } as;
} Number;

Number parse_num(Lexer* lexer) {
    Str buf = {0};
    DA_append(&buf, lexer->src[lexer->pos]);
    lexer->pos += 1;
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_digit(curr)) {
            DA_append(&buf, curr);
            lexer->pos += 1;
        } else {
            break;
        }
    }
    DA_append(&buf, '\0');
    Number num = {.type=NUM_INT, .as.integer=atoi(buf.items)};
    free(buf.items);
    return num;
}
 
void LEX_parse(Lexer* lexer, Tokens* tokens) {
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_identifier_inital(curr)) {
            StrView ident = parse_ident(lexer);
            DA_append(tokens, ((Token){.type=TOK_IDENT, .as.str_view=ident}));
            continue;
        }
        if (is_whitespace(curr)) {
            lexer->pos += 1;
            continue;
        }
        if (is_digit(curr)) {
            Number num = parse_num(lexer);
            switch (num.type) {
            case NUM_INT:
                DA_append(tokens, ((Token){.type=TOK_INT, .as.integer=num.as.integer}));
                break;
            case NUM_FLOAT:
                assert(false && "TODO: Implement floating parsing");
                break;
            }
            continue;
        }
        // as punctuation
        DA_append(
            tokens, 
            ((Token){
                .type=TOK_PUNC, 
                .as.str_view=(StrView){
                    .items=&lexer->src[lexer->pos], 
                    .length=1
                }
            })
        );
        lexer->pos += 1;
    }
    DA_append(tokens, ((Token){.type=TOK_EOF, .as.str_view=SV_from("\0")}));
}

#endif // LEXER_H_