#ifndef KIR_H_
#define KIR_H_
#include "ast.h"
#include "da.h"
#include "include/koopa.h"
#include <stdint.h>
#include <stdlib.h>

static Arena KIR_ARENA = {0};

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

#define KIR_slice_append_val(slice, type, item)                                      \
    do {                                                                             \
        if (KIR_slice_cap((slice)) <= (slice)->len) {                                \
            SliceHeader* header = (SliceHeader*)((slice)->buffer) - 1;               \
            header->capacity *= 2;                                                   \
            header = realloc(                                                        \
                header,                                                              \
                sizeof(void*)*header->capacity + sizeof(SliceHeader)                 \
            );                                                                       \
            panic_if(header == nullptr, "Error: failed to append to slice");         \
            (slice)->buffer = (void*)(header + 1);                                   \
        }                                                                            \
        void* heap = malloc(sizeof((item)));                                         \
        panic_if(heap == nullptr, "Error: failed to append item to slice");          \
        *(type*)heap = (item);                                                       \
        (slice)->buffer[(slice)->len] = heap;                                        \
        (slice)->len += 1;                                                           \
    } while (false)

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

static struct koopa_raw_value_data 
KIR_new_raw_ret_data_from_node(
    Node*             node, 
    koopa_raw_slice_t used_by
) {
    return KIR_new_raw_ret_data(
        ARENA_alloc(
            &KIR_ARENA, 
            KIR_new_raw_int_data(node->as.ret.val, used_by)
        ), used_by
    );
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

static koopa_raw_basic_block_data_t KIR_new_raw_block_data_from_node(
    Node*             node, 
    koopa_raw_slice_t params, 
    koopa_raw_slice_t used_by
) {
    koopa_raw_basic_block_data_t block = KIR_new_raw_block_data(params, used_by);
    DA_foreach(&node->as.block, sub_node) {
        switch (sub_node->kind) {
            case AST_RET: {
                koopa_raw_value_data_t* ret = ARENA_alloc(
                    &KIR_ARENA, 
                    KIR_new_raw_ret_data_from_node(sub_node, used_by)
                );
                KIR_slice_append_ptr(&block.insts, ret);
            } break;
            default: unreachable();
        }
    }
    return block;
}

static koopa_raw_type_kind_t KIR_new_raw_func_kind(
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

static koopa_raw_type_kind_t KIR_new_raw_func_kind_from_node(Node* node) {
    switch (node->as.func_def.type) {
        case DATA_INT: {
            return KIR_new_raw_func_kind(
                KIR_new_slice(KOOPA_RSIK_TYPE), &INT32_TYPE
            );
        }
        default: unreachable();
    }
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

static char* KIR_new_symbol_name(char prefix, char* name, size_t length) {
    char* symbol_name = Arena_alloc(&KIR_ARENA, strlen(name) + 2, alignof(char));
    symbol_name[0] = prefix;
    memcpy(symbol_name + 1, name, length);
    symbol_name[length + 1] = '\0';
    return symbol_name;
}

static koopa_raw_program_t KIR_to_raw_prog(AstProg* prog) {
    koopa_raw_program_t raw_prog = KIR_new_raw_prog();
    DA_foreach(prog, node) {
        switch (node->kind) {
            case AST_FUNC_DEF: {
                koopa_raw_slice_t params = KIR_new_slice(KOOPA_RSIK_VALUE);
                koopa_raw_slice_t used_by = KIR_new_slice(KOOPA_RSIK_VALUE);
                koopa_raw_type_kind_t* type = ARENA_alloc(&KIR_ARENA, KIR_new_raw_func_kind_from_node(node));
                koopa_raw_function_data_t* func_data =ARENA_alloc(&KIR_ARENA, KIR_new_raw_func_data(
                    type, 
                    KIR_new_symbol_name('@', node->as.func_def.name.items, node->as.func_def.name.length), 
                    params
                ));
                koopa_raw_basic_block_data_t* block = ARENA_alloc(
                    &KIR_ARENA, 
                    KIR_new_raw_block_data_from_node(node->as.func_def.body, params, used_by)
                );
                KIR_slice_append_ptr(&func_data->bbs, block);
                KIR_slice_append_ptr(&raw_prog.funcs, func_data);
            } break;
            case AST_RET:
            case AST_BLOCK: {
                panic("Unimplemented yet");
            } break;
        }
    }
    return raw_prog;
}

#endif // KIR_H_
