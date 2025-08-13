#include "my_array.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Array array_filter(struct Array *self, bool (*predicate)(const void *)) {
    size_t count = 0;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem)) count++;
    }

    Array result;
    result.length = count;
    result.elem_size = self->elem_size;
    result.state = INITIALIZED;
    result.data = malloc(count * self->elem_size);

    size_t j = 0;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem)) {
            memcpy((char*)result.data + j * self->elem_size, elem, self->elem_size);
            j++;
        }
    }
    return result;
}

void* array_at(struct Array *self, size_t index) {
    if (index >= self->length || self->state != INITIALIZED) return NULL;
    return (char*)self->data + index * self->elem_size;
}

bool array_add(struct Array *self, const void *elem) {
    if (self->state != INITIALIZED && self->data != NULL) return false;

    void *new_data = realloc(self->data, (self->length + 1) * self->elem_size);
    if (!new_data) return false;

    self->data = new_data;
    memcpy((char*)self->data + self->length * self->elem_size, elem, self->elem_size);
    self->length++;
    self->state = INITIALIZED;
    return true;
}


void array_init(Array *array, size_t elem_size) {
    array->data = NULL;
    array->length = 0;
    array->elem_size = elem_size;
    array->state = INITIALIZED;
}

// Init avec des données existantes
void array_init_with_data(Array *array, void *data, size_t length, size_t elem_size) {
    array->data = data;
    array->length = length;
    array->elem_size = elem_size;
    array->state = INITIALIZED;
}

void array_print(Array *array, void (*print_element_callback)(const void *)) {
    if (array->state != INITIALIZED) return;

    for (size_t i = 0; i < array->length; i++) {
        void *elem = (char*)array->data + i * array->elem_size;
        print_element_callback(elem);
    }
    printf("\n");
}

// Interface statique
Jarray jarray = {
    .filter = array_filter,
    .at = array_at,
    .add = array_add,
    .print = array_print
};
