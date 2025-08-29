#include "../inc/jarray.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

/**
 * @file jarray.c
 * @brief Implementation of the JARRAY library.
 */

/// Lookup table mapping JARRAY_ERROR enum values to their corresponding string descriptions.
static const char *enum_to_string[] = {
    [JARRAY_INDEX_OUT_OF_BOUND]                    = "Index out of bound",
    [JARRAY_UNINITIALIZED]                   = "JARRAY uninitialized",
    [JARRAY_DATA_NULL]                             = "Data is null",
    [JARRAY_PRINT_ELEMENT_CALLBACK_UNINTIALIZED]   = "Print callback not set",
    [JARRAY_ELEMENT_TO_STRING_CALLBACK_UNINTIALIZED] = "Element to string callback not set",
    [JARRAY_EMPTY]                           = "Empty jarray",
    [JARRAY_INVALID_ARGUMENT]                      = "Invalid argument",
    [JARRAY_COMPARE_CALLBACK_UNINTIALIZED]         = "Compare callback not set",
    [JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED]        = "is_equal callback not set",
    [JARRAY_ELEMENT_NOT_FOUND]                     = "Element not found",
    [JARRAY_UNIMPLEMENTED_FUNCTION]                = "Function not implemented",
};

static inline size_t max_size_t(size_t a, size_t b) {return (a > b ? a : b);}

static inline void* memcpy_elem(JARRAY *self, void *__restrict__ __dest, const void *__restrict__ __elem, size_t __count){
    void *ret;
    if (!self->user_implementation.copy_elem_override) ret = memcpy(__dest, __elem, self->_elem_size * __count);
    else {
        for (size_t i = 0; i < __count; i++){
            ret = memcpy(__dest + i * self->_elem_size, self->user_implementation.copy_elem_override(__elem + i * self->_elem_size), self->_elem_size);
        }
    }
    return ret;
}

static void print_array_err(const JARRAY_RETURN ret, const char *file, int line) {
    if (ret.ret_source->user_implementation.print_error_override) {
        ret.ret_source->user_implementation.print_error_override(ret.error);
        return;
    }
    if (ret.has_value) return;
    if (!ret.has_error) return;
    if (ret.error.error_code < 0 || ret.error.error_code >= sizeof(enum_to_string) / sizeof(enum_to_string[0]) || enum_to_string[ret.error.error_code] == NULL) {
        fprintf(stderr, "%s:%d [\033[31mUnknown error: %d\033[0m] : ", file, line, ret.error.error_code);
    } else {
        fprintf(stderr, "%s:%d [\033[31mError: %s\033[0m] : ", file, line, enum_to_string[ret.error.error_code]);
    }
    if (ret.error.error_msg) {
        fprintf(stderr, "%s\n", ret.error.error_msg);
    }
}

static void array_free(JARRAY *array) {
    if (!array) return;
    free(array->_data);
    array->_data = NULL;
    array->_length = 0;
    array->_elem_size = 0;
    array->_min_alloc = 0;
    memset(&array->user_implementation, 0, sizeof(array->user_implementation));
}

static JARRAY_RETURN create_return_error(const JARRAY* ret_source, JARRAY_ERROR error_code, const char* fmt, ...) {
    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = true;
    ret.ret_source = ret_source;
    va_list args;
    va_start(args, fmt);
    size_t len = 100*sizeof(char);
    char *buf = (char*)malloc(len);
    if (!buf) {
        ret.error.error_msg = "Error message buffer memory allocation failed";
        ret.error.error_code = JARRAY_UNINITIALIZED;
        va_end(args);
        return ret;
    }

    vsnprintf(buf, len, fmt, args);
    va_end(args);

    ret.error.error_code = error_code;
    ret.error.error_msg = buf;
    return ret;
}

static void init_array_callbacks(JARRAY *array){
    array->user_implementation.print_element_callback = NULL;
    array->user_implementation.element_to_string = NULL;
    array->user_implementation.print_array_override = NULL;
    array->user_implementation.print_error_override = NULL;
    array->user_implementation.copy_elem_override = NULL;
    array->user_implementation.compare = NULL;
    array->user_implementation.is_equal = NULL;
}

static JARRAY_RETURN array_at(const JARRAY *self, size_t index) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!self->_data) 
        return create_return_error(self, JARRAY_DATA_NULL, "Data field of array is null");

    if (index >= self->_length) 
        return create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND, "Index %zu is out of bound", index);

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = (char*)self->_data + index * self->_elem_size;
    return ret;
}

static JARRAY_RETURN array_add(JARRAY *self, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_min_alloc <= self->_length) {
        void *new_data = realloc(self->_data, (self->_length + 1) * self->_elem_size);
        if (!new_data) 
            return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in add");
    
        self->_data = new_data;
    }
    memcpy_elem(self, (char*)self->_data + self->_length * self->_elem_size, elem, 1);
    self->_length++;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_add_at(JARRAY *self, size_t index, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot insert NULL element");
    if (index > self->_length)
        return create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND, "Index %zu out of bound for insert", index);

    if (self->_min_alloc <= self->_length) {
        void *new_data = realloc(self->_data, (self->_length + 1) * self->_elem_size);
        if (!new_data) 
            return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in add");
    
        self->_data = new_data;
    }

    memmove(
        (char*)self->_data + (index + 1) * self->_elem_size,
        (char*)self->_data + index * self->_elem_size,
        (self->_length - index) * self->_elem_size
    );

    memcpy_elem(self, (char*)self->_data + index * self->_elem_size, elem, 1);
    self->_length++;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_remove_at(JARRAY *self, size_t index) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (index >= self->_length)
        return create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND, "Index %zu out of bound for remove", index);

    memmove(
        (char*)self->_data + index * self->_elem_size,
        (char*)self->_data + (index + 1) * self->_elem_size,
        (self->_length - index - 1) * self->_elem_size
    );

    self->_length--;

    if (self->_length > self->_min_alloc) {
        void *new_data = realloc(self->_data, self->_length * self->_elem_size);
        if (new_data) self->_data = new_data;
    } else if(self->_length == 0) {
        free(self->_data);
        self->_data = NULL;
    }

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_remove(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot remove from empty array");

    size_t last_index = self->_length - 1;
    return array_remove_at(self, last_index);
}

static JARRAY_RETURN array_init(JARRAY *array, size_t _elem_size) {
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    array->_data = NULL;
    array->_length = 0;
    array->_min_alloc = 0;
    array->_elem_size = _elem_size;
    
    init_array_callbacks(array);

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_init_with_data_copy(JARRAY *array, const void *data, size_t length, size_t elem_size){
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!data)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Provided data cannot be NULL");

    array->_data = malloc(length * elem_size);
    if (!array->_data)
        return create_return_error(array, JARRAY_DATA_NULL, "Memory allocation failed in init_with_data_copy");

    memcpy(array->_data, data, length * elem_size);
    array->_length = length;
    array->_min_alloc = 0;
    array->_elem_size = elem_size;

    init_array_callbacks(array);

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_init_with_data(JARRAY *array, void *data, size_t length, size_t elem_size){
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!data)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Provided data cannot be NULL");
    
    array->_data = data;
    array->_length = length;
    array->_min_alloc = 0;
    array->_elem_size = elem_size;

    init_array_callbacks(array);

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_filter(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!predicate)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Predicate cannot be NULL");

    size_t count = 0;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) count++;
    }

    JARRAY *result = malloc(sizeof(JARRAY));
    result->_length = count;
    result->_min_alloc = 0;
    result->_elem_size = self->_elem_size;
    result->_data = malloc(count * self->_elem_size);
    result->user_implementation = self->user_implementation;

    size_t j = 0;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            memcpy_elem(self, (char*)result->_data + j * self->_elem_size, elem, 1);
            j++;
        }
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = result;
    return ret;
}

static JARRAY_RETURN array_print(const JARRAY *array) {
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (array->user_implementation.print_element_callback == NULL)
        return create_return_error(array, JARRAY_PRINT_ELEMENT_CALLBACK_UNINTIALIZED, "The print single element callback not set\n");
    if (array->user_implementation.print_array_override) {
        array->user_implementation.print_array_override(array);
        JARRAY_RETURN ret;
        ret.has_value = false;
        ret.has_error = false;
        ret.value = NULL;
        return ret;
    }

    printf("JARRAY [size: %zu, min_alloc: %zu] =>\n", array->_length, array->_min_alloc);
    for (size_t i = 0; i < array->_length; i++) {
        void *elem = (char*)array->_data + i * array->_elem_size;
        array->user_implementation.print_element_callback(elem);
    }
    printf("\n");

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_sort(JARRAY *self, SORT_METHOD method, int (*custom_compare)(const void*, const void*)) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot sort an empty array");

    int (*compare)(const void*, const void*) = custom_compare ? custom_compare : self->user_implementation.compare;

    if (compare == NULL)
        return create_return_error(self, JARRAY_COMPARE_CALLBACK_UNINTIALIZED, "Either compare callback or custom compare function must be set");

    void *copy_data = malloc(self->_length * self->_elem_size);
    if (!copy_data)
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in array_sort");

    memcpy_elem(self, copy_data, self->_data, self->_length);

    switch(method) {
        case QSORT:
            qsort(copy_data, self->_length, self->_elem_size, compare);
            break;

        case BUBBLE_SORT:
            for (size_t i = 0; i < self->_length - 1; i++) {
                for (size_t j = 0; j < self->_length - i - 1; j++) {
                    void *a = (char*)copy_data + j * self->_elem_size;
                    void *b = (char*)copy_data + (j + 1) * self->_elem_size;
                    if (compare(a, b) > 0) {
                        void *temp = malloc(self->_elem_size);
                        memcpy_elem(self, temp, a, 1);
                        memcpy_elem(self, a, b, 1);
                        memcpy_elem(self, b, temp, 1);
                        free(temp);
                    }
                }
            }
            break;

        case INSERTION_SORT:
            for (size_t i = 1; i < self->_length; i++) {
                void *key = malloc(self->_elem_size);
                memcpy_elem(self, key, (char*)copy_data + i * self->_elem_size, 1);
                size_t j = i;
                while (j > 0 && compare((char*)copy_data + (j - 1) * self->_elem_size, key) > 0) {
                    memcpy_elem(self, (char*)copy_data + j * self->_elem_size,
                           (char*)copy_data + (j - 1) * self->_elem_size, 1);
                    j--;
                }
                memcpy_elem(self, (char*)copy_data + j * self->_elem_size, key, 1);
                free(key);
            }
            break;

        case SELECTION_SORT:
            for (size_t i = 0; i < self->_length - 1; i++) {
                size_t min_idx = i;
                for (size_t j = i + 1; j < self->_length; j++) {
                    void *a = (char*)copy_data + j * self->_elem_size;
                    void *b = (char*)copy_data + min_idx * self->_elem_size;
                    if (compare(a, b) < 0) {
                        min_idx = j;
                    }
                }
                if (min_idx != i) {
                    void *temp = malloc(self->_elem_size);
                    memcpy_elem(self, temp, (char*)copy_data + i * self->_elem_size, 1);
                    memcpy_elem(self, (char*)copy_data + i * self->_elem_size, (char*)copy_data + min_idx * self->_elem_size, 1);
                    memcpy_elem(self, (char*)copy_data + min_idx * self->_elem_size, temp, 1);
                    free(temp);
                }
            }
            break;

        default:
            free(copy_data);
            return create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND, "Sort method %d not implemented", method);
    }

    self->_data = copy_data;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_find_first(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!predicate)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Predicate cannot be NULL");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
    JARRAY_RETURN ret;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            ret.has_value = true;
            ret.has_error = false;
            ret.value = elem;
            return ret;
        }
    }
    return create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
}

static JARRAY_RETURN array_copy_data(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");

    JARRAY_RETURN ret;
    void *copy = NULL;
    if (self->_length > 0) {
        copy = malloc(self->_length * self->_elem_size);
        if (!copy)
            return create_return_error(self, JARRAY_DATA_NULL, "Failed to allocate _data copy");
        memcpy_elem(self, copy, self->_data, self->_length);
    }

    ret.has_value = true;
    ret.has_error = false;
    ret.value = copy; // caller now owns this pointer (may be NULL if _length==0)
    return ret;
}

static JARRAY_RETURN array_subarray(JARRAY *self, size_t start, size_t end){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot determine a sub array with an empty array\n");
    if (start > end)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "start (%zu) cannot be higher than end (%zu). It is also possible that start < 0 which would also trigger this error\n", start, end);
    if (start >= self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "start (%zu) cannot be higher or equal than the _length of array (%zu)\n", start, self->_length);

    // Clamp end to last element if it's out of bounds
    if (end >= self->_length)
        end = self->_length - 1;
    size_t sub_length = end - start + 1;

    // Allocate the JARRAY struct itself
    JARRAY *ret_array = malloc(sizeof(JARRAY));
    if (!ret_array)
        return create_return_error(self, JARRAY_DATA_NULL, "Failed to allocate memory for subarray struct\n");

    ret_array->_elem_size = self->_elem_size;
    ret_array->_min_alloc = 0;
    ret_array->_length = sub_length;
    ret_array->_data = malloc(sub_length * self->_elem_size);
    ret_array->user_implementation = self->user_implementation;
    if (!ret_array->_data) {
        free(ret_array);
        return create_return_error(self, JARRAY_DATA_NULL, "Failed to allocate memory for subarray _data\n");
    }

    // Copy relevant elements
    for (size_t i = 0; i < sub_length; i++) {
        void *src = (char*)self->_data + (start + i) * self->_elem_size;
        void *dst = (char*)ret_array->_data + i * self->_elem_size;
        memcpy_elem(self, dst, src, 1);
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = ret_array; // directly store pointer to new JARRAY
    return ret;
}

static JARRAY_RETURN array_set(JARRAY *self, size_t index, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot set element in an empty array");

    if (index >= self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Index cannot be higher or equal to the _length of array\n");

    // Copy the new element into the array at the given index
    memcpy_elem(self, (char*)self->_data + index * self->_elem_size, elem, 1);

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_indexes_of(JARRAY *self, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot search in empty array");
    if (self->user_implementation.is_equal == NULL) {
        return create_return_error(self, JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED, "is_equal callback not set");
    }
    size_t *indexes = malloc(self->_length * sizeof(size_t));
    if (!indexes) {
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for indexes array");
    }
    int count = 0;
    for (size_t i = 0; i < self->_length; i++) {
        if (self->user_implementation.is_equal((char*)self->_data + i * self->_elem_size, elem)) {
            indexes[count+1] = i; // Store the index of the matching element
            count++;
        }
    }
    if (count == 0) {
        free(indexes);
        return create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "No matching elements found");
    }
    indexes[0] = count; // Store the count of matches at the first index

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = indexes; // Caller must free this pointer
    return ret;
}

static JARRAY_RETURN array_for_each(JARRAY *self, void (*callback)(void *elem, void *ctx), void *ctx) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!callback) 
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Callback function is null");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot iterate over an empty array");

    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        callback(elem, ctx);
    }

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_clear(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_data == NULL) 
        return create_return_error(self, JARRAY_DATA_NULL, "Data field of array is null");
    // Free existing _data
    if (self->_min_alloc == 0){
        free(self->_data);
        self->_data = NULL;
    }
    self->_length = 0;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_clone(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0) 
        return create_return_error(self, JARRAY_EMPTY, "Cannot clone an empty array");

    JARRAY *clone = malloc(sizeof(JARRAY));
    if (!clone) 
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for clone");

    clone->_length = self->_length;
    clone->_min_alloc = self->_min_alloc;
    clone->_elem_size = self->_elem_size;
    clone->_data = malloc(max_size_t(self->_length, self->_min_alloc) * self->_elem_size);
    if (!clone->_data) {
        free(clone);
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for clone _data");
    }
    memcpy_elem(self, clone->_data, self->_data, max_size_t(self->_length, self->_min_alloc));
    clone->user_implementation = self->user_implementation;

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = clone; // caller must free
    return ret;
}

static JARRAY_RETURN array_add_all(JARRAY *self, const void *data, size_t count) {
    if (!data || count == 0) 
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Data is null or count is zero");

    if (self->_min_alloc <= self->_length + count - 1) {
        void *new_data = realloc(self->_data, (self->_length + count) * self->_elem_size);
        if (!new_data) 
            return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in add");
    
        self->_data = new_data;
    }

    memcpy_elem(self, (char*)self->_data + self->_length * self->_elem_size, data, count);
    self->_length += count;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_contains(JARRAY *self, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Array cannot contain a NULL element");
    if (self->_length == 0) 
        return create_return_error(self, JARRAY_EMPTY, "Cannot check containment in an empty array");
    if (self->user_implementation.is_equal == NULL) 
        return create_return_error(self, JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED, "is_equal callback not set");

    for (size_t i = 0; i < self->_length; i++) {
        void *current_elem = (char*)self->_data + i * self->_elem_size;
        if (self->user_implementation.is_equal(current_elem, elem)) {
            JARRAY_RETURN ret;
            ret.has_value = true;
            ret.has_error = false;
            ret.value = JARRAY_DIRECT_INPUT(bool, true); // return pointer to the found element
            return ret;
        }
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = JARRAY_DIRECT_INPUT(bool, false); // return pointer to false if not found
    return ret;
}

static int compare_size_t(const void *a, const void *b) {
    size_t val_a = *(const size_t*)a;
    size_t val_b = *(const size_t*)b;
    return (val_a > val_b) - (val_a < val_b);
}

static JARRAY_RETURN array_remove_all(JARRAY *self, const void *data, size_t count) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!data || count == 0) 
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Data is null or count is zero");

    JARRAY temp_array;
    JARRAY_RETURN ret = jarray.init_reserve(&temp_array, sizeof(size_t), self->_length);
    if (ret.has_error) return ret;
    for (size_t i = 0; i < count; i++) {
        const void *elem = (const char*)data + i * self->_elem_size;
        ret = array_indexes_of(self, elem);

        if (ret.has_error && ret.error.error_code == JARRAY_ELEMENT_NOT_FOUND) {
            continue; // No matches for this element
        } 
        else if (ret.has_error && ret.error.error_code == JARRAY_EMPTY) break;
        else if (ret.has_error) {
            return ret; // Some other error
        } 
        else if (!ret.has_value) {
            return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Unexpected return value from indexes_of");
        }

        size_t *indexes = (size_t*)ret.value;
        size_t match_count = indexes[0];
        array_clear(&temp_array); // Clear temp array for new indexes
        // Store only the real indexes
        for (size_t j = 0; j < match_count; j++) {
            array_add(&temp_array, &indexes[j + 1]); // j+1 because indexes[0] is count
        }

        // Sort indexes ascending, so we can remove from 
        array_sort(&temp_array, QSORT, compare_size_t);
        // Remove in reverse order to avoid shifting
        for (size_t j = match_count; j > 0; j--) {
            size_t idx;
            JARRAY_RETURN ret_at = array_at(&temp_array, j-1);
            if (ret_at.has_error) {
                free(indexes);
                return ret_at; // Error retrieving index
            }
            idx = JARRAY_RET_GET_VALUE(size_t, ret_at);
            JARRAY_RETURN remove_ret = array_remove_at(self, idx);
            if (remove_ret.has_error) {
                free(indexes);
                return remove_ret;
            }
        }
        free(indexes);
    }

    JARRAY_RETURN ok = { .has_value = false, .has_error = false, .value = NULL };
    return ok;
}

static JARRAY_RETURN array_length(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = JARRAY_DIRECT_INPUT(size_t, self->_length);
    return ret;
}

static JARRAY_RETURN array_reduce(JARRAY *self, void *(*reducer)(const void *accumulator, const void *elem, const void *ctx), const void *initial_value, const void *ctx) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!reducer) 
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function is null");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot reduce an empty array");

    void *accumulator = malloc(self->_elem_size);
    if (!accumulator)
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for accumulator");

    size_t start_index;
    if (initial_value) {
        memcpy_elem(self, accumulator, initial_value, 1);
        start_index = 0;
    } else {
        memcpy_elem(self, accumulator, self->_data, 1);
        start_index = 1;
    }

    for (size_t i = start_index; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        void *new_accumulator = reducer(accumulator, elem, ctx);
        if (!new_accumulator) {
            free(accumulator);
            return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function returned null");
        }
        memcpy_elem(self, accumulator, new_accumulator, 1);
        free(new_accumulator);
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = accumulator;
    return ret;
}

static JARRAY_RETURN array_concat(JARRAY *arr1, JARRAY *arr2) {
    if (!arr1)
        return create_return_error(arr1, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY (arr1)");
    if (!arr2)
        return create_return_error(arr2, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY (arr2)");
    if (arr1->_elem_size != arr2->_elem_size) {
        return create_return_error(NULL, JARRAY_INVALID_ARGUMENT, "Element sizes do not match for concatenation");
    }

    JARRAY *new_array = malloc(sizeof(JARRAY));
    if (!new_array) {
        return create_return_error(NULL, JARRAY_DATA_NULL, "Memory allocation failed for new array");
    }

    new_array->_elem_size = arr1->_elem_size;
    new_array->_length = arr1->_length + arr2->_length;
    new_array->_min_alloc = arr1->_length + arr2->_length;
    new_array->_data = malloc(new_array->_length * new_array->_elem_size);
    if (!new_array->_data) {
        free(new_array);
        return create_return_error(NULL, JARRAY_DATA_NULL, "Memory allocation failed for new array data");
    }

    memcpy_elem(arr1, new_array->_data, arr1->_data, 1);
    memcpy_elem(arr2, (char*)new_array->_data + arr1->_length * arr1->_elem_size, arr2->_data, arr2->_length);
    new_array->user_implementation = arr1->user_implementation;

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = new_array; // caller must free
    return ret;
}

static JARRAY_RETURN array_join(JARRAY *self, const char *separator) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot join elements of an empty array");

    if (self->user_implementation.element_to_string == NULL)
        return create_return_error(self, JARRAY_ELEMENT_TO_STRING_CALLBACK_UNINTIALIZED, "element_to_string callback not set");

    size_t total_length = 0;
    char **string_representations = malloc(self->_length * sizeof(char*));
    if (!string_representations)
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for string representations");

    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        char *str = self->user_implementation.element_to_string(elem);
        if (!str) {
            for (size_t j = 0; j < i; j++) free(string_representations[j]);
            free(string_representations);
            return create_return_error(self, JARRAY_DATA_NULL, "element_to_string callback returned null");
        }
        string_representations[i] = str;
        total_length += strlen(str);
    }

    if (!separator) separator = "";
    size_t separator_length = strlen(separator);
    total_length += separator_length * (self->_length - 1) + 1; // for separators and null terminator

    char *result = malloc(total_length);
    if (!result) {
        for (size_t i = 0; i < self->_length; i++) free(string_representations[i]);
        free(string_representations);
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for result string");
    }

    result[0] = '\0';
    for (size_t i = 0; i < self->_length; i++) {
        strcat(result, string_representations[i]);
        if (i < self->_length - 1) strcat(result, separator);
        free(string_representations[i]);
    }
    free(string_representations);

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = result; // caller must free
    return ret;
}

static JARRAY_RETURN array_reverse(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot reverse an empty array");

    size_t n = self->_length;
    void *temp = malloc(self->_elem_size);
    if (!temp)
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed during reverse");
    for (size_t i = 0; i < n / 2; i++) {
        void *a = (char*)self->_data + i * self->_elem_size;
        void *b = (char*)self->_data + (n - i - 1) * self->_elem_size;
        memcpy_elem(self, temp, a, 1);
        memcpy_elem(self, a, b, 1);
        memcpy_elem(self, b, temp, 1);
    }
    free(temp);

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_any(const JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot check any on an empty array");
    if (!predicate)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Predicate function is null");

    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            JARRAY_RETURN ret;
            ret.has_value = true;
            ret.has_error = false;
            ret.value = JARRAY_DIRECT_INPUT(bool, true);
            return ret;
        }
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = JARRAY_DIRECT_INPUT(bool, false);
    return ret;
}

static JARRAY_RETURN array_reduce_right(JARRAY *self, void *(*reducer)(const void *accumulator, const void *elem, const void *ctx), const void *initial_value, const void *ctx) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!reducer) 
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function is null");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot reduce an empty array");

    void *accumulator = malloc(self->_elem_size);
    if (!accumulator)
        return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for accumulator");

    size_t start_index;
    if (initial_value) {
        memcpy_elem(self, accumulator, initial_value, 1);
        start_index = 0;
    } else {
        memcpy_elem(self, accumulator, self->_data + (self->_length - 1) * self->_elem_size, 1);
        start_index = 1;
    }

    for (size_t i = start_index; i < self->_length; i++) {
        void *elem = (char*)self->_data + (self->_length - 1 - i) * self->_elem_size;
        void *new_accumulator = reducer(accumulator, elem, ctx);
        if (!new_accumulator) {
            free(accumulator);
            return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function returned null");
        }
        memcpy_elem(self, accumulator, new_accumulator, 1);
        free(new_accumulator);
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = accumulator;
    return ret;
}

static JARRAY_RETURN array_find_last(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
    if (!predicate)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element with a NULL predicate");
    JARRAY_RETURN ret;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + (self->_length -1 - i) * self->_elem_size;
        if (predicate(elem, ctx)) {
            ret.has_value = true;
            ret.has_error = false;
            ret.value = elem;
            return ret;
        }
    }
    return create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
}

static JARRAY_RETURN array_find_first_index(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
    if (!predicate)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element with a NULL predicate");
    JARRAY_RETURN ret;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            ret.has_value = true;
            ret.has_error = false;
            ret.value = JARRAY_DIRECT_INPUT(size_t, i);
            return ret;
        }
    }
    return create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
}

static JARRAY_RETURN array_find_last_index(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
    if (!predicate)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element with a NULL predicate");
    JARRAY_RETURN ret;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + (self->_length -1 - i) * self->_elem_size;
        if (predicate(elem, ctx)) {
            ret.has_value = true;
            ret.has_error = false;
            ret.value = JARRAY_DIRECT_INPUT(size_t, (self->_length -1 - i));
            return ret;
        }
    }
    return create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
}

static JARRAY_RETURN array_fill(JARRAY *self, const void *elem, size_t start, size_t end){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (start > end)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "start (%zu) cannot be higher than end (%zu)", start, end);
    if (start >= self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "start (%zu) must be stricly lower than the length of the jarray (%zu)", start, self->_length);
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot insert NULL in a jarray");
    if (end >= self->_length){
        size_t new_length = end + 1;
        if (self->_min_alloc <= new_length) {
            void *new_data = realloc(self->_data, new_length * self->_elem_size);
            if (!new_data) 
                return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in add");
        
            self->_data = new_data;
        }
        self->_length = new_length;
    }

    JARRAY_RETURN ret;
    for (size_t i = start; i <= end; i++){
        ret = array_set(self, i, elem);
        if (ret.has_error) return ret;
    }
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    ret.ret_source = self;
    return ret;
}

static JARRAY_RETURN array_shift(JARRAY *self){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    memmove(
        (char*)self->_data,
        (char*)self->_data + self->_elem_size,
        (self->_length - 1) * self->_elem_size
    );
    if (self->_min_alloc <= self->_length) {
        void *new_data = realloc(self->_data, (self->_length - 1) * self->_elem_size);
        if (!new_data) 
            return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in add");
    
        self->_data = new_data;
    }
    self->_length--;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_shift_right(JARRAY *self, const void *elem){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot insert NULL in a jarray");
    memmove(
        (char*)self->_data + self->_elem_size,
        (char*)self->_data,
        (self->_length + 1) * self->_elem_size
    );
    if (self->_min_alloc <= self->_length) {
        void *new_data = realloc(self->_data, (self->_length + 1) * self->_elem_size);
        if (!new_data) 
            return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed in add");
    
        self->_data = new_data;
    }
    self->_length++;

    return array_set(self, 0, elem);
}

static JARRAY_RETURN array_splice_ext(JARRAY *self, size_t index, size_t count, va_list args) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot splice a NULL JARRAY");
    if (index > self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "index (%zu) must be <= length (%zu)", index, self->_length);

    JARRAY_RETURN ret;

    // --- Suppression ---
    for (size_t i = 0; i < count; i++) {
        ret = array_remove_at(self, index);
        if (ret.has_error) {
            if (ret.error.error_code == JARRAY_INDEX_OUT_OF_BOUND) break;
            else return ret;
        } else {
            JARRAY_FREE_RET(ret);
        }
    }

    // --- Insertion ---
    size_t offset = 0;
    void *element = va_arg(args, void*);
    while (element != NULL) {
        ret = array_add_at(self, index + offset, element);
        if (ret.has_error) {
            return ret;
        } else {
            JARRAY_FREE_RET(ret);
        }
        offset++;
        element = va_arg(args, void*);
    }

    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_splice(JARRAY *self, size_t index, size_t count, ...) {
    va_list args;
    va_start(args, count);
    JARRAY_RETURN ret = array_splice_ext(self, index, count, args);
    va_end(args);
    return ret;
}

static JARRAY_RETURN array_addm_ext(JARRAY *self, va_list args) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");

    JARRAY_RETURN ret;
    void *element = va_arg(args, void*);
    while (element != NULL) {
        ret = array_add(self, element);
        if (ret.has_error) {
            return ret;
        } else {
            JARRAY_FREE_RET(ret);
        }
        element = va_arg(args, void*);
    }
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_addm(JARRAY *self, ...){
    va_list args;
    va_start(args, self);
    JARRAY_RETURN ret = array_addm_ext(self, args);
    va_end(args);
    return ret;
}

static JARRAY_RETURN array_reserve(JARRAY *self, size_t capacity){
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");

    self->_min_alloc = capacity;
    if (capacity > self->_length) {
        void* new_data = NULL;
        if (self->_length > 0) new_data = realloc(self->_data, capacity * self->_elem_size);
        else new_data = malloc(capacity * self->_elem_size);
        if (!new_data) 
            return create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed when reallocating for reserve");
        self->_data = new_data;
    }

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

static JARRAY_RETURN array_init_reserve(JARRAY *self, size_t elem_size, size_t capacity){
    JARRAY_RETURN ret = jarray.init(self, elem_size);
    if (ret.has_error) return ret;
    ret = jarray.reserve(self, capacity);
    return ret;
}

extern JARRAY create_jarray_string(void);
extern JARRAY create_jarray_int(void);
extern JARRAY create_jarray_float(void);
extern JARRAY create_jarray_char(void);

static JARRAY array_init_preset(TYPE_PRESET preset){
    JARRAY (*ret_func)(void) = NULL;
    switch (preset){
        case STRING_PRESET:
            ret_func = create_jarray_string;
            break;
        case INT_PRESET:
            ret_func = create_jarray_int;
            break;
        case FLOAT_PRESET:
            ret_func = create_jarray_float;
            break;
        case CHAR_PRESET:
            ret_func = create_jarray_char;
            break;
    }
    JARRAY array = ret_func();
    return array;
}

/// Static interface implementation for easier usage.
JARRAY_INTERFACE jarray = {
    .filter = array_filter,
    .at = array_at,
    .add = array_add,
    .remove = array_remove,
    .remove_at = array_remove_at,
    .add_at = array_add_at,
    .print = array_print,
    .init = array_init,
    .init_with_data_copy = array_init_with_data_copy,
    .init_with_data = array_init_with_data,
    .init_preset = array_init_preset,
    .print_array_err = print_array_err,
    .free = array_free,
    .sort = array_sort,
    .find_first = array_find_first,
    .copy_data = array_copy_data,
    .subarray = array_subarray,
    .set = array_set,
    .indexes_of = array_indexes_of,
    .for_each = array_for_each,
    .clear = array_clear,
    .clone = array_clone,
    .add_all = array_add_all,
    .contains = array_contains,
    .remove_all = array_remove_all,
    .length = array_length,
    .reduce = array_reduce,
    .concat = array_concat,
    .join = array_join,
    .reverse = array_reverse,
    .any = array_any,
    .reduce_right = array_reduce_right,
    .find_last = array_find_last,
    .find_first_index = array_find_first_index,
    .find_last_index = array_find_last_index,
    .fill = array_fill,
    .shift = array_shift,
    .shift_right = array_shift_right,
    .splice = array_splice,
    .addm = array_addm,
    .reserve = array_reserve,
    .init_reserve = array_init_reserve,
};