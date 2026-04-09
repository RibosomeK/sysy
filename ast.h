#ifndef AST_H_
#define AST_H_
#include "da.h"
#include "lex.h"

typedef enum {
    RET,
    BLOCK,
    FUNC_DEF
} NodeKind;

typedef enum {
    DATA_INT, 
} DataType;

void DATA_TYPE_display(DataType type) {
    switch (type) {
    case DATA_INT:
        printf("int");
        break;
    }
}

typedef struct Node Node;
struct Node {
    NodeKind kind;
    union {
        struct { int val; } ret;
        struct {
            Node*  items;
            size_t length;
            size_t capacity;
        } block;
        struct {
            DataType type;
            StrView  name;
            Node*    body;
        } func_def;
    } as;
};

void NODE_display(Node* node) {
    switch (node->kind) {
    case RET:
        printf("return %d;", node->as.ret.val);
        break;
    case BLOCK:
        printf("{ ");
        DA_foreach(&(node->as.block), sub_node) {
            NODE_display(sub_node);
        }
        printf("} ");
        break;
    case FUNC_DEF:
        DATA_TYPE_display(node->as.func_def.type);
        printf(" %s()", node->as.func_def.name);
        NODE_display(node->as.func_def.body);
        break;
    }
    printf("\n");
}

typedef struct {
    Node* items;
    size_t length;
    size_t capacity;
} Nodes;

typedef struct {
    Tokens* tokens;
    size_t  pos;
} Parser;

typedef enum {
    MISSING_SEMI = 1,
    MISSING_EXPR,
} ParseError;

typedef struct {
    bool       is_ok;
    ParseError err;
} ParseResult;

Token PARSER_curr(Parser* parser) {
    return parser->tokens->items[parser->pos];
}

Token PARSER_consume(Parser* parser) {
    Token curr = PARSER_curr(parser);
    parser->pos += 1;
    return curr;
}

Token PARSER_next(Parser* parser) {
    parser->pos += 1;
    return PARSER_curr(parser);
}

ParseResult parse_ret(Parser* parser, Nodes* nodes) {
    Token curr = PARSER_next(parser);
    if (curr.type != TOK_INT) 
        return (ParseResult){ .err = MISSING_EXPR };
    int val = curr.as.integer;
    curr = PARSER_next(parser);
    if (
        curr.type == TOK_PUNC && 
        SV_eq(&curr.as.str_view, ";")
    )
        PARSER_consume(parser);
    else
        return (ParseResult){ .err = MISSING_SEMI };
    DA_append(
        nodes, 
        ((Node){ .kind = RET, .as.ret = { .val = val } })
    );
    return (ParseResult){ .is_ok = true };
}

void AST_parse(Parser* parser, Nodes* nodes) {
    ParseResult result = { .is_ok = true };
    while (parser->pos < parser->tokens->length) {
        Token curr = PARSER_curr(parser);
        if (curr.type == TOK_IDENT) {
            if (SV_eq(&curr.as.str_view, "return")) {
                result = parse_ret(parser, nodes);
                if (!result.is_ok) goto error;
            } else {
                assert(false && "NOT IMPLEMENTED YET");
            }
        } else if (curr.type == TOK_EOF) {
            break;
        } else {
            assert(false && "NOT IMPLEMENTED YET");
        }
    }
error:
    switch (result.err) {
        case MISSING_SEMI: 
            fprintf(stderr, "ERROR: Missing semicolon\n"); 
            break;
        case MISSING_EXPR: 
            fprintf(stderr, "ERROR: Missing expresion\n"); 
            break;
    }
}

#endif // AST_H_