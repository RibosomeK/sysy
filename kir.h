#ifndef KIR_H_
#define KIR_H_
#include "ast.h"
#include "da.h"
#include "include/koopa.h"
#include <stdlib.h>

typedef enum {
    I32,
} KirType;

static KirType KIR_TYPE_from_datatype(DataType type) {
    switch (type) {
        case DATA_INT:
            return I32;
        default: unreachable();
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

typedef struct {
    size_t capacity;
} SliceHeader;

static koopa_raw_slice_t KIR_new_slice(koopa_raw_slice_item_kind_t kind) {
    koopa_raw_slice_t slice = {0};
    slice.kind = kind;
    SliceHeader* buf = malloc(sizeof(SliceHeader) + sizeof(void*)*INIT_CAPACITY);
    panic_if(buf == nullptr, "Error: failed to init new slice");
    buf->capacity = INIT_CAPACITY;
    slice.buffer = (void*)(buf + 1);
    return slice;
};

static size_t KIR_slice_cap(koopa_raw_slice_t* slice) {
    return ((SliceHeader*)slice->buffer - 1)->capacity;
}

static void KIR_slice_append_ptr(koopa_raw_slice_t* slice, void* item) {
    if (KIR_slice_cap(slice) <= slice->len) {
        SliceHeader* header = (SliceHeader*)(slice->buffer) - 1;
        header->capacity *= 2;
        header = realloc(header, sizeof(void*)*header->capacity + sizeof(SliceHeader));
        panic_if(header == nullptr, "Error: failed to append to slice");
        slice->buffer = (void*)(header + 1);
    }
    slice->buffer[slice->len] = item;
    slice->len += 1;
}

#define KIR_slice_append_val(slice, type, item)                                            \
    do {                                                                             \
        if (KIR_slice_cap((slice)) <= (slice)->len) {                                \
            SliceHeader* header = (SliceHeader*)((slice)->buffer) - 1;               \
            header->capacity *= 2;                                                   \
            header = realloc( \
                header, \
                sizeof(void*)*header->capacity + sizeof(SliceHeader) \
            );   \
            panic_if(header == nullptr, "Error: failed to append to slice");         \
            (slice)->buffer = (void*)(header + 1);                                   \
        }                                                                            \
        void* heap = malloc(sizeof((item)));                                         \
        panic_if(heap == nullptr, "Error: failed to append item to slice");          \
        *(type*)heap = (item);                                               \
        (slice)->buffer[(slice)->len] = heap;                                        \
        (slice)->len += 1;                                                           \
    } while (false)

#define KIR_slice_append(slice, item)                                                \
    _Generic((item),                                                                 \
        void*:   KIR_slice_append_ptr,                                               \
        default: KIR_slice_append_val                                                \
    )(slice, item)

static koopa_raw_program_t KIR_to_raw_prog(KirUnit* unit) {
    koopa_raw_program_t raw_prog = {
        .values = KIR_new_slice(KOOPA_RSIK_VALUE),
        .funcs = KIR_new_slice(KOOPA_RSIK_FUNCTION),
    };
    return raw_prog;
}

#define KIR_slice_foreach(slice, type, item)                                         \
    for (                                                                            \
        type** item = (type**)(slice)->buffer;                                       \
        item < (type**)(slice)->buffer + (slice)->len;                               \
        item++                                                                       \
    )

static void KIR_handle_err_code(koopa_error_code_t code) {
    switch (code) {
        case KOOPA_EC_INVALID_UTF8_STRING:
            panic("Error: UTF-8 string conversion error");
        case KOOPA_EC_INVALID_FILE:
            panic("Error: File operation error");
        case KOOPA_EC_INVALID_KOOPA_PROGRAM:
            panic("Error: Koopa IR program parsing error");
        case KOOPA_EC_IO_ERROR:
            panic("Error: IO operation error");
        case KOOPA_EC_NULL_BYTE_ERROR: 
            panic("Error: Byte array to C string conversion error");
        case KOOPA_EC_INSUFFICIENT_BUFFER_LENGTH:
            panic("Error: Insufficient buffer length");
        case KOOPA_EC_RAW_SLICE_ITEM_KIND_MISMATCH:
            panic("Error: Mismatch of item kind in raw slice");
        case KOOPA_EC_NULL_POINTER_ERROR:
            panic("Error: Passing null pointers to `libkoopa`");
        case KOOPA_EC_TYPE_MISMATCH:
            panic("Error: Mismatch of type");
        case KOOPA_EC_FUNC_PARAM_NUM_MISMATCH:
            panic("Error: Mismatch of function parameter number");
        case KOOPA_EC_SUCCESS: break;
    }
}

static koopa_raw_program_t KIR_new_raw_prog() {
    return (koopa_raw_program_t) { 
        .values = KIR_new_slice(KOOPA_RSIK_VALUE), 
        .funcs = KIR_new_slice(KOOPA_RSIK_FUNCTION) 
    };
}

static struct koopa_raw_type_kind INT32_TYPE = { .tag = KOOPA_RTT_INT32 };
static struct koopa_raw_type_kind UNIT_TYPE  = { .tag = KOOPA_RTT_UNIT };

static koopa_raw_integer_t KIR_new_raw_int(int val) {
    return (koopa_raw_integer_t) { .value = val };
}

static koopa_raw_value_kind_t KIR_new_raw_int_kind(int val) {
    return (koopa_raw_value_kind_t) { 
        .tag  = KOOPA_RVT_INTEGER, 
        .data = { .integer = KIR_new_raw_int(val) }
    };
}

static struct koopa_raw_value_data 
KIR_new_raw_int_data(int val, koopa_raw_slice_t used_by) {
    return  (struct koopa_raw_value_data) {
        .ty      = &INT32_TYPE,
        .name    = nullptr,
        .used_by = used_by,
        .kind    = KIR_new_raw_int_kind(val),
    };
}

static koopa_raw_value_kind_t KIR_new_raw_ret_kind(koopa_raw_value_t val) {
    return (koopa_raw_value_kind_t) {
        .tag  = KOOPA_RVT_RETURN,
        .data = { .ret = { .value = val } }
    };
}

static struct koopa_raw_value_data KIR_new_raw_ret_data(
    koopa_raw_value_t val, 
    koopa_raw_slice_t used_by
) {
    return  (struct koopa_raw_value_data) {
        .ty      = &UNIT_TYPE,
        .name    = nullptr,
        .used_by = used_by,
        .kind    = KIR_new_raw_ret_kind(val),
    };
}

static koopa_raw_basic_block_data_t KIR_new_raw_block_data(
    koopa_raw_slice_t params, koopa_raw_slice_t used_by
) {
    return (koopa_raw_basic_block_data_t) {
        .name    = nullptr,
        .params  = params,
        .used_by = used_by,
        .insts   = KIR_new_slice(KOOPA_RSIK_VALUE),
    };
}

static struct koopa_raw_type_kind KIR_new_raw_func_kind(
    koopa_raw_slice_t      params_kind, 
    koopa_raw_type_kind_t* ret_kind
) {
    return (struct koopa_raw_type_kind) {
        .tag = KOOPA_RTT_FUNCTION,
        .data.function = {
            .params = params_kind,
            .ret    = ret_kind,
        }
    };
}

static koopa_raw_function_data_t KIR_new_raw_func_data(
    koopa_raw_type_t  type,
    char*             name,
    koopa_raw_slice_t params
) {
    return (koopa_raw_function_data_t) { 
        // type is function signature
        // for main is () -> i32
        .ty =  type, 
        // name should start with @ or %
        .name = name, 
        .params = params, 
        .bbs = KIR_new_slice(KOOPA_RSIK_BASIC_BLOCK),
    };
}

#endif // KIR_H_