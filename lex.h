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
    size_t row;
    size_t col;
} Loc;

typedef struct {
    TokType type;
    union {
        int     integer;
        StrView str_view;
    } as;
    Loc loc;
} Token;

typedef struct {
    Token* items;
    size_t length;
    size_t capacity;
} Tokens;

static void TOKEN_to_str(Token* token, Str* buf) {
    switch (token->type) {
    case TOK_IDENT:
    case TOK_PUNC:
    case TOK_EOF:
        STR_append_by_size(buf, token->as.str_view.items, token->as.str_view.length);
        break;
    case TOK_INT:
        DA_resize(buf, 21);
        sprintf(buf->items, "%d", token->as.integer);
        break;
    }
}

typedef struct {
    char*  src;
    size_t pos;
    size_t len;
    Loc    loc;
} Lexer;

static Lexer LEX_new(char* src) { 
    return (Lexer) {
        .src = src,
        .pos = 0,
        .len = strlen(src),
        .loc = { .row = 1, .col = 1 }
    };
}

bool is_identifier_inital(char c) {
    int ord = (int)c;
    if (65 <= ord && ord <= 90)  return true;
    if (ord == 95)               return true;
    if (97 <= ord && ord <= 122) return true;
    return false;
}

bool is_identifier_follow(char c) {
    int ord = (int)c;
    if (48 <= ord && ord <= 57)  return true;
    if (65 <= ord && ord <= 90)  return true;
    if (ord == 95)               return true;
    if (97 <= ord && ord <= 122) return true;
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
    if (48 <= ord && ord <= 57) return true;
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
    lexer->loc.col += length;
    return sv;
}

typedef enum {
    NUM_INT,
    NUM_FLOAT,
} NUM_TYPE;

typedef struct {
    NUM_TYPE type;
    union {
        int   integer;
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
    lexer->loc.col += buf.length;
    DA_append(&buf, '\0');
    Number num = { .type=NUM_INT, .as.integer = atoi(buf.items) };
    free(buf.items);
    return num;
}

static Loc LEX_curr_loc(Lexer* lexer) {
    return (Loc) { .row = lexer->loc.row, .col = lexer->loc.col };
}
 
void LEX_parse(Lexer* lexer, Tokens* tokens) {
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_identifier_inital(curr)) {
            Loc loc = LEX_curr_loc(lexer);
            StrView ident = parse_ident(lexer);
            DA_append(tokens, ((Token) { 
                .type = TOK_IDENT, 
                .as.str_view = ident,
                .loc = loc
            }));
            continue;
        }
        if (is_digit(curr)) {
            Loc loc = LEX_curr_loc(lexer);
            Number num = parse_num(lexer);
            switch (num.type) {
            case NUM_INT:
                DA_append(tokens, ((Token) { 
                    .type = TOK_INT, 
                    .as.integer = num.as.integer,
                    .loc = loc
                }));
                break;
            case NUM_FLOAT:
                panic("TODO: Implement floating parsing");
                break;
            }
            continue;
        }
        switch (curr) {
            case ' ' :
            case '\t': 
            case '\v': {
                lexer->pos += 1;
                lexer->loc.col += 1;
            } break;
            case '\n': {
                lexer->pos += 1;
                lexer->loc.col = 1;
                lexer->loc.row += 1;
            } break;
            case '\r': {
                if (lexer->src[lexer->pos + 1] == '\n') {
                    lexer->pos += 2;
                } else {
                    lexer->pos += 1;
                }
                lexer->loc.col = 1;
                lexer->loc.row += 1;
            } break;
            // as punctuation
            default: {
                Token token = {
                    .type=TOK_PUNC, 
                    .as.str_view = (StrView) {
                        .items   = &lexer->src[lexer->pos], 
                        .length  = 1
                    },
                    .loc = LEX_curr_loc(lexer)
                };
                DA_append(tokens, token);
                lexer->pos += 1;
                lexer->loc.col += 1;
            } break;
        }
    }
    DA_append(
        tokens, 
        ((Token) { 
            .type=TOK_EOF, 
            .as.str_view = SV_from("\0"), 
            .loc = LEX_curr_loc(lexer) 
        })
    );
}

#endif // LEXER_H_