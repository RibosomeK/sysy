#ifndef LEX_H_
#define LEX_H_
#include <string.h>
#include "da.h"

typedef enum {
    IDENT,
    INT,
    PUNC,
} TokType;

typedef struct {
    TokType type;
    union {
        int   integer;
        char* string;
    } as;
} Token;

typedef struct {
    Token* items;
    size_t length;
    size_t capacity;
} Tokens;

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

void parse_ident(Lexer* lexer, Str* buf) {
    DA_append(buf, lexer->src[lexer->pos]);
    lexer->pos += 1;
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_identifier_follow(curr)) {
            DA_append(buf, curr);
            lexer->pos += 1;
        } else {
            break;
        }
    }
    DA_append(buf, '\0');
}

void parse_num(Lexer* lexer, Str* buf) {
    DA_append(buf, lexer->src[lexer->pos]);
    lexer->pos += 1;
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_digit(curr)) {
            DA_append(buf, curr);
            lexer->pos += 1;
        } else {
            break;
        }
    }
    DA_append(buf, '\0');
}

void Tok_append(Tokens* tokens, TokType type, char* val) {
    switch (type) {
    case IDENT: 
    case PUNC:
        DA_append(tokens, ((Token){ .type = type, .as.string = val }));
        break;
    case INT: 
        DA_append(tokens, ((Token){ .type = type, .as.integer = atoi(val) }));
        break;
    }
}
 
void LEX_parse(Lexer* lexer, Tokens* tokens, Str* buf) {
    while (lexer->pos < lexer->len) {
        char curr = lexer->src[lexer->pos];
        if (is_identifier_inital(curr)) {
            parse_ident(lexer, buf);
            Tok_append(tokens, IDENT, strdup(buf->items));
            DA_clear(buf);
            continue;
        }
        if (is_whitespace(curr)) {
            lexer->pos += 1;
            continue;
        }
        if (is_digit(curr)) {
            parse_num(lexer, buf);
            Tok_append(tokens, INT, strdup(buf->items));
            DA_clear(buf);
            continue;
        }
        // as punctuation
        DA_append(buf, curr);
        DA_append(buf, '\0');
        Tok_append(tokens, PUNC, strdup(buf->items));
        DA_clear(buf);
        lexer->pos += 1;
    }
}

#endif // LEXER_H_