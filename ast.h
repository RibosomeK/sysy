#ifndef AST_H_
#define AST_H_
#include "da.h"
#include "lex.h"

typedef enum {
    AST_RET,
    AST_BLOCK,
    AST_FUNC_DEF,
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

typedef struct {
    Node*  items;
    size_t length;
    size_t capacity;
} AstProg;

static Arena AST_ARENA = {0};

static void NODE_to_str(Node* node, Str* buf) {
    switch (node->kind) {
        case AST_RET:
            STR_append(buf, "return ");
            STR_append_int(buf, node->as.ret.val);
            STR_append(buf, ";");
            break;
        case AST_BLOCK:
            STR_append(buf, "{ ");
            DA_foreach(&node->as.block, sub_node) {
                NODE_to_str(sub_node, buf);
            }
            STR_append(buf, " }");
            break;
        case AST_FUNC_DEF:
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

static void AST_prog_to_str(AstProg* prog, Str* buf) {
    DA_foreach(prog, node) {
        NODE_to_str(node, buf);
    }
}

typedef struct {
    Tokens* tokens;
    size_t  pos;
} Parser;

static Token PARSER_curr(Parser* parser) {
    return parser->tokens->items[parser->pos];
}

static Token PARSER_consume(Parser* parser) {
    Token curr = PARSER_curr(parser);
    parser->pos += 1;
    return curr;
}

static Token PARSER_next(Parser* parser) {
    parser->pos += 1;
    return PARSER_curr(parser);
}

static Node AST_parse(Parser* parser);

static Node parse_ret(Parser* parser) {
    Token curr = PARSER_next(parser);
    if (curr.type != TOK_INT)
        panic(
            "Error: Missing expression at line %zu, column %zu",
            curr.loc.row, curr.loc.col
        );
    int val = curr.as.integer;
    curr = PARSER_next(parser);
    if (
        curr.type == TOK_PUNC && 
        SV_eq(&curr.as.str_view, ";")
    )
        PARSER_consume(parser);
    else
        panic(
            "Error: Missing semicolon at line %zu, column %zu", 
            curr.loc.row, curr.loc.col
        );
    return (Node) {
        .kind = AST_RET,
        .as.ret = {
            .val = val
        }
    };
}

static Node parse_block(Parser* parser) {
    Loc loc = PARSER_curr(parser).loc;
    Node block = { .kind = AST_BLOCK, .as.block = {0} };
    Token next = PARSER_next(parser);
    while (!(next.type == TOK_PUNC && SV_eq(&next.as.str_view, "}"))) {
        if (next.type == TOK_EOF)
            panic(
                "Error: unbalanced brackets starts at line %zu, column %zu",
                loc.row, loc.col
            );
        Node sub_node = AST_parse(parser);
        DA_append(&block.as.block, sub_node);
        next = PARSER_curr(parser);
    }
    PARSER_consume(parser);
    return block;
}

static Node parse_def(Parser* parser) {
    Token next = PARSER_next(parser);
    if (next.type != TOK_IDENT)
        panic(
            "Error: Missing identifier at line %zu, column %zu",
            next.loc.row, next.loc.col
        );
    if (SV_not_eq(&next.as.str_view, "main"))
        panic(
            "Error: function name '%.*s' at line %zu, column %zu should be main",
            (int)next.as.str_view.length, next.as.str_view.items, next.loc.row, next.loc.col
        );
    StrView name = next.as.str_view;
    next = PARSER_next(parser);
    Loc left_loc = next.loc;
    if (next.type != TOK_PUNC || SV_not_eq(&next.as.str_view, "("))
        panic(
            "Error: '(' is expected at line %zu, column %zu",
            next.loc.row, next.loc.col
        );
    next = PARSER_next(parser);
    if (next.type != TOK_PUNC || SV_not_eq(&next.as.str_view, ")"))
        panic(
            "Error: unbalanced parenthesis starts at line %zu, column %zu",
            left_loc.row, left_loc.col
        );
    next = PARSER_next(parser);
    if (next.type != TOK_PUNC)
        panic(
            "Error: '(' is expected at line %zu, column %zu",
            next.loc.row, next.loc.col
        );
    if (SV_eq(&next.as.str_view, ";"))
        panic(
            "function declaration at line %zu, column %zu is not yet implemented",
            next.loc.row, next.loc.col
        );
    if (SV_eq(&next.as.str_view, "{")) {
        Node* block = ARENA_alloc(&AST_ARENA, parse_block(parser));
        return (Node) {.kind = AST_FUNC_DEF,
            .as.func_def = {
                .type = DATA_INT,
                .name = name,
                .body = block,
            }
        };
    }
    panic(
        "Error: '{' is expected at line %zu, column %zu",
        next.loc.row, next.loc.col
    );
}

static Node AST_parse(Parser* parser) {
    Token curr = PARSER_curr(parser);
    if (curr.type == TOK_IDENT) {
        if (SV_eq(&curr.as.str_view, "return")) {
            return parse_ret(parser);
        }
        if (SV_eq(&curr.as.str_view, "int")) {
            Node node = parse_def(parser);
            return node;
        }
        panic("NOT IMPLEMENTED YET");
    }
    panic("NOT IMPLEMENTED YET");
}

static AstProg AST_parse_prog(Parser* parser) {
    AstProg prog = {0};
    while (PARSER_curr(parser).type != TOK_EOF ) {
        DA_append(&prog, AST_parse(parser));
    }
    return prog;
}

#endif // AST_H_