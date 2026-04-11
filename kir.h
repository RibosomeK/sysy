#ifndef KIR_H_
#define KIR_H_
#include "ast.h"

typedef enum {
    I32,
} KirType;

KirType KIR_TYPE_from_datatype(DataType type) {
    switch (type) {
        case DATA_INT:
            return I32;
    }
}

static void KIR_TYPE_to_str(KirType type, Str* buf) {
    switch (type) {
        case I32:
            STR_append(buf, "i32");
            break;
    }
}

typedef enum {
    INTEGER,
    RETURN,
} KirValueKind;

typedef struct KirValue KirValue;
struct KirValue {
    KirValueKind kind;
    union {
        int       integer;
        KirValue* ret;
    } as;
};

typedef struct {
    KirValue* items;
    size_t    length;
    size_t    capacity;
} KirBlock;

typedef struct {
    KirBlock* items;
    size_t    length;
    size_t    capacity;
    StrView   name;
    KirType   type;
} KirFunc;

typedef struct {
    struct {
        KirFunc* items;
        size_t   length;
        size_t   capacity;
    } funcs;
} KirProgram;

typedef enum {
    KIR_VALUE,
    KIR_BLOCK,
    KIR_FUNC,
    KIR_PROG,
} KirUnitKind;

typedef struct {
    KirUnitKind kind;
    union {
        KirProgram prog;
        KirFunc    func;
        KirBlock   block;
        KirValue   val;
    } as;
} KirUnit;

static KirUnit KIR_from_node(Node* node) {
    KirUnit unit = {0};
    switch (node->kind) {
        case AST_FUNC_DEF:
            unit.kind = KIR_FUNC;
            unit.as.func.name = node->as.func_def.name;
            unit.as.func.type = KIR_TYPE_from_datatype(node->as.func_def.type);
            KirUnit block = KIR_from_node(node->as.func_def.body);
            DA_append(&unit.as.func, block.as.block);
            break;
        case AST_RET:
            unit.kind = KIR_VALUE;
            KirValue* ret = malloc(sizeof(KirValue));
            KirValue tmp = {
                .kind = INTEGER,
                .as.integer = node->as.ret.val,
            };
            *ret = tmp;
            unit.as.val = (KirValue) {
                .kind = RETURN,
                .as.ret = ret,
            };
            break;
        case AST_BLOCK:
            unit.kind = KIR_BLOCK;
            DA_foreach(&node->as.block, sub_node) {
                KirUnit sub_unit = KIR_from_node(sub_node);
                panic_if(sub_unit.kind != KIR_VALUE, "ERROR: Value is expected");
                DA_append(&unit.as.block, sub_unit.as.val);
            }
            break;
    }
    return unit;
}

static void KIR_VALUE_to_str(KirValue* val, Str* buf) {
    switch (val->kind) {
        case RETURN: {
            STR_append(buf, "ret ");
            STR_append_int(buf, val->as.ret->as.integer);
        } break;
        case INTEGER: {
            STR_append_int(buf, val->as.integer);
        } break;
    }
}

static void KIR_BLOCK_to_str(KirBlock* block, Str* buf, int count) {
    STR_append(buf, "%");
    STR_append_int(buf, count);
    STR_append(buf, ":\n");
    DA_foreach(block, val) {
        KIR_VALUE_to_str(val, buf);
        STR_append(buf, "\n");
    }
}

static void KIR_FUNC_to_str(KirFunc* func, Str* buf, int count) {
    STR_append(buf, "fun @");
    STR_append_sv(buf, &func->name);
    STR_append(buf, "(): ");
    KIR_TYPE_to_str(func->type, buf);
    STR_append(buf, " {\n");
    DA_foreach (func, block) {
        KIR_BLOCK_to_str(block, buf, count);
    }
    STR_append(buf, "}\n");
}

static void KIR_to_str(KirUnit* unit, Str* buf, int count) {
    switch (unit->kind) {
        case KIR_PROG: {
            DA_foreach(&unit->as.prog.funcs, func) {
                KIR_FUNC_to_str(func, buf, count);
            }
        } break;
        case KIR_FUNC: {
            KIR_FUNC_to_str(&unit->as.func, buf, count);
        } break;
        case KIR_VALUE: {
            KIR_VALUE_to_str(unit->as.val.as.ret, buf);
        } break;
        case KIR_BLOCK: {
            KIR_BLOCK_to_str(&unit->as.block, buf, count);
        } break;
    }
}

#endif // KIR_H_