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
// Insère un élément à l'index donné
ARRAY_RETURN array_add_at(Array *self, size_t index, const void *elem) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (index > self->length) // autorise index == length (ajout à la fin)
        return create_return_error(INDEX_OUT_OF_BOUND, "Index %zu out of bound for insert", index);

    void *new_data = realloc(self->data, (self->length + 1) * self->elem_size);
    if (!new_data)
        return create_return_error(DATA_NULL, "Memory allocation failed in add_at");

    self->data = new_data;

    // Décaler les éléments vers la droite
    memmove(
        (char*)self->data + (index + 1) * self->elem_size,
        (char*)self->data + index * self->elem_size,
        (self->length - index) * self->elem_size
    );

    // Copier le nouvel élément
    memcpy((char*)self->data + index * self->elem_size, elem, self->elem_size);

    self->length++;

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = (char*)self->data + index * self->elem_size;
    return ret;
}

// Supprime un élément à un index donné
ARRAY_RETURN array_remove_at(Array *self, size_t index) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (index >= self->length)
        return create_return_error(INDEX_OUT_OF_BOUND, "Index %zu out of bound for remove", index);

    // Sauvegarde l'adresse de l'élément à retourner
    void *removed_elem = malloc(self->elem_size);
    if (!removed_elem)
        return create_return_error(DATA_NULL, "Memory allocation failed in remove_at");
    memcpy(removed_elem, (char*)self->data + index * self->elem_size, self->elem_size);

    // Décaler les éléments restants vers la gauche
    memmove(
        (char*)self->data + index * self->elem_size,
        (char*)self->data + (index + 1) * self->elem_size,
        (self->length - index - 1) * self->elem_size
    );

    self->length--;

    // Réduire la taille mémoire
    if (self->length > 0) {
        void *new_data = realloc(self->data, self->length * self->elem_size);
        if (new_data) self->data = new_data; // éviter de perdre data si realloc échoue
    } else {
        free(self->data);
        self->data = NULL;
    }

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = removed_elem; // pointeur alloué que l'appelant devra free
    return ret;
}

ARRAY_RETURN array_remove(Array *self) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot remove from empty array");

    size_t last_index = self->length - 1;
    return array_remove_at(self, last_index);
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
    result->user_implementation = self->user_implementation;

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
ARRAY_RETURN array_print(Array *array) {
    if (array->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");
    if (array->user_implementation.print_element_callback == NULL)
        return create_return_error(PRINT_ELEMENT_CALLBACK_UNINTIALIZED, "The print single element callback not set\n");
    for (size_t i = 0; i < array->length; i++) {
        void *elem = (char*)array->data + i * array->elem_size;
        array->user_implementation.print_element_callback(elem);
    }
    printf("\n");

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = array;
    return ret;
}


// array_sort
ARRAY_RETURN array_sort(Array *self, SORT_METHOD method) {
    int (*compare)(const void*, const void*) = self->user_implementation.compare;
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot sort an empty array");

    if (compare == NULL)
        return create_return_error(PRINT_ELEMENT_CALLBACK_UNINTIALIZED, "Compare callback not set");

    // Création d'une copie des données
    void *copy_data = malloc(self->length * self->elem_size);
    if (!copy_data)
        return create_return_error(DATA_NULL, "Memory allocation failed in array_sort");

    memcpy(copy_data, self->data, self->length * self->elem_size);

    // Appliquer le tri sur la copie
    switch(method) {
        case QSORT:
            qsort(copy_data, self->length, self->elem_size, compare);
            break;

        case BUBBLE_SORT:
            for (size_t i = 0; i < self->length - 1; i++) {
                for (size_t j = 0; j < self->length - i - 1; j++) {
                    void *a = (char*)copy_data + j * self->elem_size;
                    void *b = (char*)copy_data + (j + 1) * self->elem_size;
                    if (compare(a, b) > 0) {
                        void *temp = malloc(self->elem_size);
                        memcpy(temp, a, self->elem_size);
                        memcpy(a, b, self->elem_size);
                        memcpy(b, temp, self->elem_size);
                        free(temp);
                    }
                }
            }
            break;

        case INSERTION_SORT:
            for (size_t i = 1; i < self->length; i++) {
                void *key = malloc(self->elem_size);
                memcpy(key, (char*)copy_data + i * self->elem_size, self->elem_size);
                size_t j = i;
                while (j > 0 && compare((char*)copy_data + (j - 1) * self->elem_size, key) > 0) {
                    memcpy((char*)copy_data + j * self->elem_size,
                           (char*)copy_data + (j - 1) * self->elem_size,
                           self->elem_size);
                    j--;
                }
                memcpy((char*)copy_data + j * self->elem_size, key, self->elem_size);
                free(key);
            }
            break;

        case SELECTION_SORT:
            for (size_t i = 0; i < self->length - 1; i++) {
                size_t min_idx = i;
                for (size_t j = i + 1; j < self->length; j++) {
                    void *a = (char*)copy_data + j * self->elem_size;
                    void *b = (char*)copy_data + min_idx * self->elem_size;
                    if (compare(a, b) < 0) {
                        min_idx = j;
                    }
                }
                if (min_idx != i) {
                    void *temp = malloc(self->elem_size);
                    memcpy(temp, (char*)copy_data + i * self->elem_size, self->elem_size);
                    memcpy((char*)copy_data + i * self->elem_size, (char*)copy_data + min_idx * self->elem_size, self->elem_size);
                    memcpy((char*)copy_data + min_idx * self->elem_size, temp, self->elem_size);
                    free(temp);
                }
            }
            break;

        default:
            free(copy_data);
            return create_return_error(INDEX_OUT_OF_BOUND, "Sort method %d not implemented", method);
    }

    // Créer un Array pour la copie triée
    Array *sorted_array = malloc(sizeof(Array));
    if (!sorted_array) {
        free(copy_data);
        return create_return_error(DATA_NULL, "Memory allocation failed for sorted array struct");
    }

    sorted_array->data = copy_data;
    sorted_array->length = self->length;
    sorted_array->elem_size = self->elem_size;
    sorted_array->state = INITIALIZED;
    sorted_array->user_implementation = self->user_implementation;

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = sorted_array;
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
    .remove = array_remove,
    .remove_at = array_remove_at,
    .add_at = array_add_at,
    .print = array_print,
    .init = array_init,
    .init_with_data = array_init_with_data,
    .print_array_err = print_array_err,
    .sort = array_sort
};
