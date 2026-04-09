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

static void DATA_TYPE_to_str(DataType type, Str* buf) {
    switch (type) {
        case DATA_INT:
            STR_append(buf, "int");
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

static void NODE_to_str(Node* node, Str* buf) {
    switch (node->kind) {
        case RET:
            STR_append(buf, "return ");
            STR_append_int(buf, node->as.ret.val);
            STR_append(buf, ";");
            break;
        case BLOCK:
            STR_append(buf, "{ ");
            DA_foreach(&node->as.block, sub_node) {
                NODE_to_str(sub_node, buf);
            }
            STR_append(buf, " }");
            break;
        case FUNC_DEF:
            DATA_TYPE_to_str(node->as.func_def.type, buf);
            STR_append(buf, " ");
            STR_append_by_size(
                buf,
                node->as.func_def.name.items,
                node->as.func_def.name.length
            );
            STR_append(buf, " ");
            NODE_to_str(node->as.func_def.body, buf);
            break;
    }
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