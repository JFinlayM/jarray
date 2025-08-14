#include "my_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Convert enum to string
const char *enum_to_string[] = {
    [INDEX_OUT_OF_BOUND] = "Index out of bound",
    [ARRAY_UNINITIALIZED] = "Array uninitialized",
    [DATA_NULL] = "Data is null"
};

// Crée un ARRAY_RETURN avec message formaté
ARRAY_RETURN create_return_error(ARRAY_ERROR error_code, const char* fmt, ...) {
    ARRAY_RETURN ret;
    va_list args;
    va_start(args, fmt);

    // allocation dynamique pour garder le message après sortie
    char *buf = malloc(256);
    if (!buf) {
        ret.has_value = false;
        ret.error.error_msg = "Memory allocation failed";
        ret.error.error_code = ARRAY_UNINITIALIZED;
        va_end(args);
        return ret;
    }

    vsnprintf(buf, 256, fmt, args);
    va_end(args);

    ret.has_value = false;
    ret.error.error_code = error_code;
    ret.error.error_msg = buf;  // mémorisation dynamique
    return ret;
}

// array_at avec ARRAY_RETURN
ARRAY_RETURN array_at(Array *self, size_t index) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (!self->data) 
        return create_return_error(DATA_NULL, "Data field of array is null");

    if (index >= self->length) 
        return create_return_error(INDEX_OUT_OF_BOUND, "Index %zu is out of bound", index);

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = (char*)self->data + index * self->elem_size;
    return ret;
}

// array_add avec ARRAY_RETURN
ARRAY_RETURN array_add(Array *self, const void *elem) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    void *new_data = realloc(self->data, (self->length + 1) * self->elem_size);
    if (!new_data) 
        return create_return_error(DATA_NULL, "Memory allocation failed in add");

    self->data = new_data;
    memcpy((char*)self->data + self->length * self->elem_size, elem, self->elem_size);
    self->length++;

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = (char*)self->data + (self->length-1) * self->elem_size;
    return ret;
}

// array_init
ARRAY_RETURN array_init(Array *array, size_t elem_size) {
    array->data = NULL;
    array->length = 0;
    array->elem_size = elem_size;
    array->state = INITIALIZED;

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = array;
    return ret;
}

// array_init_with_data
ARRAY_RETURN array_init_with_data(Array *array, void *data, size_t length, size_t elem_size) {
    array->data = data;
    array->length = length;
    array->elem_size = elem_size;
    array->state = INITIALIZED;

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = array;
    return ret;
}

// array_filter
ARRAY_RETURN array_filter(Array *self, bool (*predicate)(const void *)) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    size_t count = 0;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem)) count++;
    }

    Array *result = malloc(sizeof(Array));
    result->length = count;
    result->elem_size = self->elem_size;
    result->state = INITIALIZED;
    result->data = malloc(count * self->elem_size);

    size_t j = 0;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem)) {
            memcpy((char*)result->data + j * self->elem_size, elem, self->elem_size);
            j++;
        }
    }

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = result;
    return ret;
}

// array_print
ARRAY_RETURN array_print(Array *array, void (*print_element_callback)(const void *)) {
    if (array->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    for (size_t i = 0; i < array->length; i++) {
        void *elem = (char*)array->data + i * array->elem_size;
        print_element_callback(elem);
    }
    printf("\n");

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = array;
    return ret;
}

// print error
void print_array_err(ARRAY_RETURN ret) {
    if (ret.has_value) return;
    printf("\033[31m%s\033[0m\n", ret.error.error_msg);
    free((void*)ret.error.error_msg); // libération mémoire dynamique
}

// interface statique
Jarray jarray = {
    .filter = array_filter,
    .at = array_at,
    .add = array_add,
    .print = array_print,
    .init = array_init,
    .init_with_data = array_init_with_data,
    .print_array_err = print_array_err
};
