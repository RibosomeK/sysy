#ifndef DA_H_
#define DA_H_
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAPACITY 16
#define DA [[maybe_unused]] static

[[noreturn]] DA void panic(char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(-1);
}

[[noreturn]] DA void unreachable() {
    panic("Error: unreachable");
}

DA void panic_if(bool cond, char* msg) {
    if (cond) panic(msg);
}

#define DA_def(type, name)                                                           \
    typedef struct {                                                                 \
        type   items;                                                                \
        size_t length;                                                               \
        size_t capacity;                                                             \
    } name

#define DA_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->capacity <= (da)->length) {                                        \
            if ((da)->capacity == 0) (da)->capacity = INIT_CAPACITY;                 \
            else (da)->capacity *= 2;                                                \
            (da)->items = realloc((da)->items, sizeof(*(da)->items)*(da)->capacity); \
            panic_if((da)->items == nullptr, "ERROR: append failed");                \
        }                                                                            \
        (da)->items[(da)->length] = (item);                                          \
        (da)->length += 1;                                                           \
    } while (false)

#define DA_pop(da, ptr)                                                              \
    do {                                                                             \
        panic_if((da)->length == 0, "ERROR: pop from empty array");                  \
        *(ptr) = (da)->items[(da)->length-1];                                        \
        (da)->length -= 1;                                                           \
    } while (false)

#define DA_append_str(da, str) \
    do {                                                                             \
        if ((da)->capacity <= (da)->length) {                                        \
            if ((da)->capacity == 0) (da)->capacity = INIT_CAPACITY;                 \
            else (da)->capacity *= 2;                                                \
            (da)->items = realloc((da)->items, sizeof(*(da)->items)*(da)->capacity); \
            panic_if((da)->items == nullptr, "ERROR: append failed");                \
        }                                                                            \
        char* idx = malloc(sizeof(char)*(strlen(str)+1));                            \
        panic_if(idx == nullptr, "ERROR: append failed");                            \
        strcpy(idx, (str));                                                          \
        (da)->items[(da)->length] = idx;                                             \
        (da)->length += 1;                                                           \
    } while (false)

#define DA_pop_str(da, ptr)                                                          \
    do {                                                                             \
        panic_if((da)->length == 0, "ERROR: pop from empty array");                  \
        (ptr) = (da)->items[(da)->length-1];                                         \
        (da)->length -= 1;                                                           \
    } while (false)

#define DA_foreach(da, item)                                                         \
    for (auto (item) = (da)->items; (item) < (da)->items + (da)->length; (item)++)

#define DA_clear(da) (da)->length = 0

#define DA_resize(da, size)                                                          \
    do {                                                                             \
        (da)->capacity = (size);                                                     \
        (da)->items = realloc((da)->items, sizeof(char)*(da)->capacity);             \
        panic_if((da)->items == nullptr, "ERROR: resize failed");                    \
    } while (false)

typedef struct {
    char*  items;
    size_t length;
    size_t capacity;
} Str;

DA void STR_clear(Str* str) {
    str->items[0] = '\0';
    str->length = 0;
}

DA void STR_append(Str* builder, char* str) {
    size_t str_len = strlen(str);
    if (builder->capacity <= builder->length + str_len) {
        if (builder->capacity == 0) builder->capacity = INIT_CAPACITY + str_len + 1;
        else builder->capacity = 2 * builder->capacity + str_len + 1;
        builder->items = realloc(builder->items, sizeof(char)*builder->capacity);
        panic_if(builder->items == nullptr, "ERROR: append failed");
    }
    strcpy(&builder->items[builder->length], str);
    builder->length += str_len;
}

DA void STR_extend(Str* builder, char* str, ...) {
    va_list va;
    va_start(va, str);
    do {
        STR_append(builder, str);
        str = va_arg(va, char*);
    } while (str != nullptr);
    va_end(va);
}

 DA void STR_append_by_size(Str* builder, char* str, size_t size) {
    if (builder->capacity <= builder->length + size + 1)
        DA_resize(builder, builder->length + size + 1);
    memcpy(&builder->items[builder->length], str, size);
    builder->length += size;
    builder->items[builder->length] = '\0';
}

// len(str(2**64)) + 1
#define INT_LEN 21

DA void STR_append_int(Str* builder, int num) {
    if (builder->capacity <= builder->length + INT_LEN)
        DA_resize(builder, builder->length + INT_LEN);
    int length = sprintf(&builder->items[builder->length], "%d", num);
    builder->length += length;
}

typedef struct {
    char*  items;
    size_t length;
} StrView;

DA StrView SV_from(char* cstr) {
    return (StrView) { .items=cstr, .length=strlen(cstr) };
}

DA void STR_append_sv(Str* builder, StrView* sv) {
    STR_append_by_size(builder, sv->items, sv->length);
}

#define SV_eq(sv, str_like)                                                          \
    _Generic((str_like),                                                             \
        char*:    SV_cstr_eq,                                                        \
        Str*:     SV_str_eq,                                                         \
        StrView*: SV_sv_eq                                                           \
    )(sv, str_like) 

#define SV_not_eq(sv, str_like)                                                      \
    !_Generic((str_like),                                                            \
        char*:    SV_cstr_eq,                                                        \
        Str*:     SV_str_eq,                                                         \
        StrView*: SV_sv_eq                                                           \
    )(sv, str_like)

DA bool SV_cstr_eq(StrView* sv, char* cstr) {
    if (sv->length != strlen(cstr)) return false;
    return memcmp(sv->items, cstr, sv->length) == 0;
}

DA bool SV_str_eq(StrView* sv, Str* str) {
    if (sv->length != str->length) return false;
    return memcmp(sv->items, str->items, sv->length) == 0;
}

DA bool SV_sv_eq(StrView* sv1, StrView* sv2) {
    if (sv1->length != sv2->length) 
        return false;
    if (sv1->items == sv2->items) 
        return true;
    return memcmp(sv1->items, sv2->items, sv1->length);
}

#define OPTION_DEF(type)                                                             \
    typedef struct {                                                                 \
        bool is_some;                                                                \
        union {                                                                      \
            type  some;                                                              \
            void* none;                                                              \
        } as;                                                                        \
        void* tag;                                                                   \
    } Option_##type

#define Option(type) Option_##type
#define SOME(type, val) (Option(type)) { .is_some = true, .as.some = (val) }
#define NONE(type) (Option(type)) { .as.none = nullptr }
#define OUnwrap(opt) opt->is_some ? opt->as.some : panic("ERROR: none unwrap")

#define RESULT_DEF(type)                                                             \
    typedef struct {                                                                 \
        bool is_ok;                                                                  \
        union {                                                                      \
            type val;                                                                \
            int  err;                                                                \
        } as;                                                                        \
        bool tag;                                                                    \
    } Result_##type

#define Result(type) Result_##type
#define OK(type, val) (Result(type)) { .is_ok = true, .as.val = (val) }
#define ERR(type, err) (Result(type)) { .as.err = err }
#define RUnwrap(ret) ret->is_ok ? opt->as.val : panic("ERROR: error unwrap")

#define Unwrap(x)                                                                    \
    _Generic((x->tag),                                                               \
        void*: OUnwrap,                                                              \
        bool:  RUnwrap,                                                              \
    )(x)

#define ARENA_INIT_CAPACITY 4096

typedef struct {
    uint8_t* start;
    size_t   offset;
    size_t   capacity;
} Arena;

DA size_t Arena_get_padding(Arena* arena, size_t align) {
    uintptr_t curr = (uintptr_t)(arena->start + arena->offset);
    return curr % align == 0 ? 0 : align - curr % align;
}

DA void* Arena_alloc(Arena* arena, size_t size, size_t align) {
    size_t padding = Arena_get_padding(arena, align);
    if (arena->capacity < arena->offset + size + padding) {
        if (arena->capacity == 0) arena->capacity = ARENA_INIT_CAPACITY;
        else arena->capacity *= 2;  
        while (arena->capacity < arena->offset + size + padding) {
            arena->capacity *= 2;  
        }
        arena->start = realloc(arena->start, arena->capacity);
        panic_if(arena->start == nullptr, "Error: Arena alloc failed");
        padding = Arena_get_padding(arena, align);
    }
    void* ret_ptr = arena->start + arena->offset + padding;
    arena->offset += padding + size;
    return ret_ptr;
}

#define ARENA_alloc(arena, val)                                                      \
    ({                                                                               \
        typeof(val) tmp = (val);                                                     \
        typeof(tmp)* ptr = Arena_alloc((arena), sizeof(tmp), alignof(typeof(val)));  \
        *ptr = tmp;                                                                  \
        ptr;                                                                         \
    })

DA void Arena_reset(Arena* arena) {
    arena->offset = 0;
}

DA void Arena_free(Arena* arena) {
    free(arena->start);
}

#endif // DA_H_