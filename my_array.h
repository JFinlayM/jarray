#ifndef MY_ARRAY_H
#define MY_ARRAY_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    UNINITIALIZED = 0,
    INITIALIZED
} Array_state_t;

typedef struct Array {
    void *data;
    size_t length;
    size_t elem_size;
    struct Array* self;
    Array_state_t state;
} Array;

typedef struct Jarray {
    struct Array (*filter)(struct Array *self, bool (*predicate)(const void *));
    void* (*at)(struct Array *self, size_t index);
    bool (*add)(struct Array *self, const void * elem);
    void (*print)(struct Array *self, void (*print_element_callback)(const void *));
} Jarray;


extern Jarray jarray;

struct Array array_filter(struct Array *self, bool (*predicate)(const void *));
void array_init(Array *array, size_t elem_size);
void array_init_with_data(Array *array, void *data, size_t length, size_t elem_size);

#endif
