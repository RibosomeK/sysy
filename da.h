#ifndef DA_H_
#define DA_H_
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAPACITY 16

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
    } while (0)

#define DA_pop(da, ptr)                                                              \
    do {                                                                             \
        assert((da)->length != 0 && "ERROR: pop from empty array");                  \
        *(ptr) = (da)->items[(da)->length-1];                                        \
        (da)->length -= 1;                                                           \
    } while (0) 

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
    } while (0)

#define DA_pop_str(da, ptr)                                                          \
    do {                                                                             \
        assert((da)->length != 0 && "ERROR: pop from empty array");                  \
        (ptr) = (da)->items[(da)->length-1];                                         \
        (da)->length -= 1;                                                           \
    } while (0) 

#define DA_foreach(da, item)                                                         \
    for (auto (item) = (da)->items; (item) < (da)->items + (da)->length; (item)++)

#define DA_clear(da) (da)->length = 0

typedef struct {
    char*  items;
    size_t length;
    size_t capacity;
} Str;

void STR_append(Str* builder, char* str) {
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

void STR_extend(Str* builder, char* str, ...) {
    va_list va;
    va_start(va, str);
    do {
        STR_append(builder, str);
        str = va_arg(va, char*);
    } while (str != NULL);
    va_end(va);
}

typedef struct {
    char*  items;
    size_t length;
} StrView;

StrView SV_from(char* cstr) {
    return (StrView){.items=cstr, .length=strlen(cstr)};
}

#define SV_eq(sv, str_like)                                                          \
    _Generic((str_like),                                                             \
        char*:    SV_cstr_eq,                                                        \
        Str*:     SV_str_eq,                                                         \
        StrView*: SV_sv_eq                                                           \
    )(sv, str_like) 


bool SV_cstr_eq(StrView* sv, char* cstr) {
    if (sv->length != strlen(cstr)) return false;
    return memcmp(sv->items, cstr, sv->length) == 0;
}

bool SV_str_eq(StrView* sv, Str* str) {
    if (sv->length != str->length) return false;
    return memcmp(sv->items, str->items, sv->length) == 0;
}

bool SV_sv_eq(StrView* sv1, StrView* sv2) {
    if (sv1->length != sv2->length) 
        return false;
    if (sv1->items == sv2->items) 
        return true;
    else
        return memcpy(sv1->items, sv2->items, sv1->length);
}

#endif // DA_H_