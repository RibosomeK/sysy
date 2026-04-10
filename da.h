#ifndef DA_H_
#define DA_H_
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAPACITY 16
#define DA [[maybe_unused]] static

#define DA_def(type, name)                                                           \
    typedef struct {                                                                 \
        type items;                                                                  \
        size_t length;                                                               \
        size_t capacity;                                                             \
    } name

#define DA_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->capacity <= (da)->length) {                                        \
            if ((da)->capacity == 0) (da)->capacity = INIT_CAPACITY;                 \
            else (da)->capacity *= 2;                                                \
            (da)->items = realloc((da)->items, sizeof(*(da)->items)*(da)->capacity); \
            assert((da)->items != NULL && "ERROR: append failed");                   \
        }                                                                            \
        (da)->items[(da)->length] = (item);                                            \
        (da)->length += 1;                                                           \
    } while (false)

#define DA_pop(da, ptr)                                                              \
    do {                                                                             \
        assert((da)->length != 0 && "ERROR: pop from empty array");                  \
        *(ptr) = (da)->items[(da)->length-1];                                        \
        (da)->length -= 1;                                                           \
    } while (false)

#define DA_append_str(da, str) \
    do {                                                                             \
        if ((da)->capacity <= (da)->length) {                                        \
            if ((da)->capacity == 0) (da)->capacity = INIT_CAPACITY;                 \
            else (da)->capacity *= 2;                                                \
            (da)->items = realloc((da)->items, sizeof(*(da)->items)*(da)->capacity); \
            assert((da)->items != NULL && "ERROR: append failed");                   \
        }                                                                            \
        char* idx = malloc(sizeof(char)*(strlen(str)+1));                            \
        assert(idx != NULL && "ERROR: append failed");                               \
        strcpy(idx, (str));                                                            \
        (da)->items[(da)->length] = idx;                                             \
        (da)->length += 1;                                                           \
    } while (false)

#define DA_pop_str(da, ptr)                                                          \
    do {                                                                             \
        assert((da)->length != 0 && "ERROR: pop from empty array");                  \
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
        assert((da)->items != NULL && "ERROR: resize failed");                       \
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
        assert(builder->items != NULL && "ERROR: append failed");
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
    } while (str != NULL);
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
    return (StrView){.items=cstr, .length=strlen(cstr)};
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
#define OUnwarp(opt) opt->is_some ? opt->as.some : assert(false && "ERROR: none unwarp")

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
#define RUnwarp(ret) ret->is_ok ? opt->as.val : assert(false && "ERROR: error unwarp")

#define Unwarp(x)                                                                    \
    _Generic((x->tag),                                                               \
        void*: OUnwarp,                                                              \
        bool:  RUnwarp,                                                              \
    )(x)

[[noreturn]] DA void panic(char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(-1);
}

DA void panic_if(bool cond, char* msg) {
    if (cond) panic(msg);
}

#endif // DA_H_