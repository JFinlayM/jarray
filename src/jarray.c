#include "../inc/jarray.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

/**
 * @file jarray.c
 * @brief Implementation of the JARRAY library.
 */

/// Static error trace
JARRAY_RETURN last_error_trace = {0};

/// Lookup table mapping JARRAY_ERROR enum values to their corresponding string descriptions.
static const char *enum_to_string[] = {
    [JARRAY_NO_ERROR]                                   = "No error",
    [JARRAY_INDEX_OUT_OF_BOUND]                         = "Index out of bound",
    [JARRAY_UNINITIALIZED]                              = "JARRAY uninitialized",
    [JARRAY_DATA_NULL]                                  = "Data is null",
    [JARRAY_PRINT_ELEMENT_CALLBACK_UNINTIALIZED]        = "Print callback not set",
    [JARRAY_ELEMENT_TO_STRING_CALLBACK_UNINTIALIZED]    = "Element to string callback not set",
    [JARRAY_EMPTY]                                      = "Empty jarray",
    [JARRAY_INVALID_ARGUMENT]                           = "Invalid argument",
    [JARRAY_COMPARE_CALLBACK_UNINTIALIZED]              = "Compare callback not set",
    [JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED]             = "is_equal callback not set",
    [JARRAY_ELEMENT_NOT_FOUND]                          = "Element not found",
    [JARRAY_UNIMPLEMENTED_FUNCTION]                     = "Function not implemented",
};

static inline size_t max_size_t(size_t a, size_t b) {return (a > b ? a : b);}

static inline void* memcpy_elem(JARRAY *self, void *__restrict__ __dest, const void *__restrict__ __elem, size_t __count){
    void *ret = __dest;

    if (self->_data_type == JARRAY_TYPE_VALUE) {
        ret = memcpy(__dest, __elem, self->_elem_size * __count);
    } else if (self->_data_type == JARRAY_TYPE_POINTER){
        for (size_t i = 0; i < __count; i++) {
            const void *src_elem = (const char*)__elem + i * self->_elem_size;
            if (!src_elem || !(*(void**)src_elem)) continue;
            const void *src_elem_copy = self->user_callbacks.copy_elem_override(src_elem);
            memcpy((char*)__dest + i * self->_elem_size, src_elem_copy, self->_elem_size);
        }
    }
    return ret;
}


static void print_array_err(const char *file, int line) {
    if (last_error_trace.ret_source->user_overrides.print_error_override) {
        last_error_trace.ret_source->user_overrides.print_error_override(last_error_trace);
        return;
    }
    if (!last_error_trace.has_error) return;
    if (last_error_trace.error_code < 0 || last_error_trace.error_code >= sizeof(enum_to_string) / sizeof(enum_to_string[0]) || enum_to_string[last_error_trace.error_code] == NULL) {
        fprintf(stderr, "%s:%d [\033[31mUnknown error: %d\033[0m] : ", file, line, last_error_trace.error_code);
    } else {
        fprintf(stderr, "%s:%d [\033[31mError: %s\033[0m] : ", file, line, enum_to_string[last_error_trace.error_code]);
    }
    fprintf(stderr, "%s\n", last_error_trace.error_msg);
}

static void array_free(JARRAY *array) {
    if (!array) return;
    if (array->_data_type == JARRAY_TYPE_POINTER) {
        for (size_t i = 0; i < array->_length; i++){
            void **ptr = (void**)jarray.at(array, i);
            free(*ptr);
        }
    }
    free(array->_data);
    array->_data = NULL;
    array->_length = 0;
    array->_elem_size = 0;
    array->_min_alloc = 0;
    array->_capacity = 0;
    array->_capacity_multiplier = 1.5f;
    memset(&array->user_callbacks, 0, sizeof(array->user_callbacks));
    memset(&array->user_overrides, 0, sizeof(array->user_overrides));
}

static void create_return_error(const JARRAY* ret_source, JARRAY_ERROR error_code, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    last_error_trace.has_error = true;
    last_error_trace.error_code = error_code;
    vsnprintf(last_error_trace.error_msg, MAX_ERR_MSG_LENGTH, fmt, args);
    last_error_trace.ret_source = ret_source;
    va_end(args);
}

static void reset_error_trace(){
    last_error_trace.has_error = false;
    last_error_trace.ret_source = NULL;
    last_error_trace.error_code = JARRAY_NO_ERROR;
    snprintf(last_error_trace.error_msg, MAX_ERR_MSG_LENGTH, "no error");
}

static void init_array_callbacks(JARRAY *array){
    array->user_callbacks.print_element_callback = NULL;
    array->user_callbacks.element_to_string = NULL;
    array->user_callbacks.compare = NULL;
    array->user_callbacks.is_equal = NULL;
    array->user_callbacks.copy_elem_override = NULL;
}

static void init_array_overrides(JARRAY *array){
    array->user_overrides.print_array_override = NULL;
    array->user_overrides.print_error_override = NULL;
}

static void* array_at(const JARRAY *self, size_t index) {
    if (!self){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (!self->_data) {
        create_return_error(self, JARRAY_DATA_NULL, "Data field of array is null");
        return NULL;
    }
    if (index >= self->_length) {
        create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND, "Index %zu is out of bound", index);
        return NULL;
    }
    reset_error_trace();
    return (char*)self->_data + index * self->_elem_size;
}

static void array_add(JARRAY *self, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot add to a NULL JARRAY");
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot insert NULL element");

    if (self->_length + 1 > self->_capacity) {
        size_t new_cap = (self->_capacity > 0)
                             ? (size_t)((float)self->_capacity * self->_capacity_multiplier)
                             : 1;

        if (new_cap <= self->_capacity)
            new_cap = self->_capacity + 1;

        while (new_cap < self->_length + 1) {
            size_t next = (size_t)((float)new_cap * self->_capacity_multiplier);
            if (next <= new_cap) next = new_cap + 1;
            new_cap = next;
        }

        void *new_data = realloc(self->_data, new_cap * self->_elem_size);
        if (!new_data)
            return create_return_error(self, JARRAY_DATA_NULL,
                                       "Memory allocation failed in add");

        self->_data = new_data;
        self->_capacity = new_cap;
    }

    memcpy_elem(self, (char *)self->_data + self->_length * self->_elem_size, elem, 1);
    self->_length++;
    reset_error_trace();
}


static void array_add_at(JARRAY *self, size_t index, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot insert into a NULL JARRAY");
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot insert NULL element");
    if (index > self->_length)
        return create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND,
                                   "Index %zu out of bound for insert", index);

    if (self->_length + 1 > self->_capacity) {
        size_t new_cap = (self->_capacity > 0)
                             ? (size_t)((float)self->_capacity * self->_capacity_multiplier)
                             : 1;
        if (new_cap <= self->_capacity)
            new_cap = self->_capacity + 1;

        while (new_cap < self->_length + 1) {
            size_t next = (size_t)((float)new_cap * self->_capacity_multiplier);
            if (next <= new_cap) next = new_cap + 1;
            new_cap = next;
        }

        void *new_data = realloc(self->_data, new_cap * self->_elem_size);
        if (!new_data)
            return create_return_error(self, JARRAY_DATA_NULL,
                                       "Memory allocation failed in add_at");

        self->_data = new_data;
        self->_capacity = new_cap;
    }

    if (index < self->_length) {
        memmove(
            (char *)self->_data + (index + 1) * self->_elem_size,
            (char *)self->_data + index * self->_elem_size,
            (self->_length - index) * self->_elem_size
        );
    }

    memcpy_elem(self, (char *)self->_data + index * self->_elem_size, elem, 1);
    self->_length++;
    reset_error_trace();
}


static void array_remove_at(JARRAY *self, size_t index) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot remove from a NULL JARRAY");
    if (index >= self->_length)
        return create_return_error(self, JARRAY_INDEX_OUT_OF_BOUND,
                                   "Index %zu out of bound for remove", index);

    size_t move_count = self->_length - index - 1;
    if (move_count > 0) {
        memmove(
            (char *)self->_data + index * self->_elem_size,
            (char *)self->_data + (index + 1) * self->_elem_size,
            move_count * self->_elem_size
        );
    }

    self->_length--;

    if (self->_length == 0) {
        free(self->_data);
        self->_data = NULL;
        self->_capacity = 0;
    } else {
        size_t new_cap = self->_capacity / self->_capacity_multiplier;
        if (new_cap < self->_min_alloc)
            new_cap = self->_min_alloc;

        if (self->_length <= new_cap && self->_length < self->_capacity / 2) {
            void *new_data = realloc(self->_data, new_cap * self->_elem_size);
            if (new_data) {
                self->_data = new_data;
                self->_capacity = new_cap;
            }
        }
    }

    reset_error_trace();
}

static void array_remove(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot remove from empty array");

    size_t last_index = self->_length - 1;
    return array_remove_at(self, last_index);
}

static void array_init(JARRAY *array, size_t _elem_size, JARRAY_DATA_TYPE data_type, JARRAY_USER_CALLBACK_IMPLEMENTATION imp) {
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    array->_data = NULL;
    array->_length = 0;
    array->_capacity = 0;
    array->_min_alloc = 0;
    array->_capacity_multiplier = 1.5f;
    array->_elem_size = _elem_size;
    array->_data_type = data_type;
    init_array_callbacks(array);
    init_array_overrides(array);
    array->user_callbacks = imp;
    if (data_type == JARRAY_TYPE_POINTER && !array->user_callbacks.copy_elem_override) return create_return_error(array, JARRAY_UNIMPLEMENTED_FUNCTION, "'copy_elem_override' function must me implemented and referenced in 'user_overrides' struct in array");
    reset_error_trace();
}

static void array_init_with_data_copy(JARRAY *array, const void *data, size_t length, size_t elem_size, JARRAY_DATA_TYPE data_type, JARRAY_USER_CALLBACK_IMPLEMENTATION imp){
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!data)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Provided data cannot be NULL");

    array->_data = malloc(length * elem_size);
    if (!array->_data)
        return create_return_error(array, JARRAY_DATA_NULL, "Memory allocation failed in init_with_data_copy");

    memcpy(array->_data, data, length * elem_size);
    array->_capacity = length;
    array->_capacity_multiplier = 1.5f;
    array->_length = length;
    array->_min_alloc = 0;
    array->_elem_size = elem_size;
    array->_data_type = data_type;

    init_array_callbacks(array);
    init_array_overrides(array);
    array->user_callbacks = imp;
    if (data_type == JARRAY_TYPE_POINTER && !array->user_callbacks.copy_elem_override) return create_return_error(array, JARRAY_UNIMPLEMENTED_FUNCTION, "'copy_elem_override' function must me implemented and referenced in 'user_overrides' struct in array");
    reset_error_trace();
}

static void array_init_with_data(JARRAY *array, void *data, size_t length, size_t elem_size, JARRAY_DATA_TYPE data_type, JARRAY_USER_CALLBACK_IMPLEMENTATION imp){
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!data)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Provided data cannot be NULL");
    
    array->_data = data;
    array->_length = length;
    array->_capacity_multiplier = 1.5f;
    array->_capacity = length;
    array->_min_alloc = 0;
    array->_elem_size = elem_size;
    array->_data_type = data_type;

    init_array_callbacks(array);
    init_array_overrides(array);
    array->user_callbacks = imp;
    if (data_type == JARRAY_TYPE_POINTER && !array->user_callbacks.copy_elem_override) return create_return_error(array, JARRAY_UNIMPLEMENTED_FUNCTION, "'copy_elem_override' function must me implemented and referenced in 'user_overrides' struct in array");
    reset_error_trace();
}

static JARRAY array_filter(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx) {
    if (!self){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return *self;
    }
    if (!predicate){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Predicate cannot be NULL");
        return *self;
    }

    size_t count = 0;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) count++;
    }

    JARRAY result;
    result._length = count;
    result._min_alloc = count;
    result._capacity = count;
    result._capacity_multiplier = self->_capacity_multiplier;
    result._elem_size = self->_elem_size;
    result._data = malloc(count * self->_elem_size);
    result.user_callbacks = self->user_callbacks;
    result.user_overrides = self->user_overrides;

    size_t j = 0;
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            memcpy_elem(self, (char*)result._data + j * self->_elem_size, elem, 1);
            j++;
        }
    }
    reset_error_trace();
    return result;
}

static void array_print(const JARRAY *array) {
    if (!array)
        return create_return_error(array, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (array->user_callbacks.print_element_callback == NULL)
        return create_return_error(array, JARRAY_PRINT_ELEMENT_CALLBACK_UNINTIALIZED, "The print single element callback not set\n");
    if (array->user_overrides.print_array_override) {
        array->user_overrides.print_array_override(array);
        return;
    }

    printf("JARRAY [size: %zu, capacity: %zu, min_alloc: %zu, capacity multiplier: %.2f] =>\n", array->_length, array->_capacity, array->_min_alloc, array->_capacity_multiplier);
    for (size_t i = 0; i < array->_length; i++) {
        void *elem = (char*)array->_data + i * array->_elem_size;
        array->user_callbacks.print_element_callback(elem);
    }
    printf("\n");
    reset_error_trace();
}

static void array_sort(JARRAY *self, SORT_METHOD method, int (*custom_compare)(const void*, const void*)) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot sort an empty array");

    int (*compare)(const void*, const void*) = custom_compare ? custom_compare : self->user_callbacks.compare;

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
    reset_error_trace();
}

static void* array_find_first(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (!predicate){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Predicate cannot be NULL");
        return NULL;
    }
    if (self->_length == 0){
        create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
        return NULL;
    }
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            reset_error_trace();
            return elem;
        }
    }
    create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
    return NULL;
}

static void* array_copy_data(JARRAY *self) {
    if (!self){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    
    void *copy = NULL;
    if (self->_length > 0) {
        copy = malloc(self->_length * self->_elem_size);
        if (!copy){
            create_return_error(self, JARRAY_DATA_NULL, "Failed to allocate _data copy");
            return NULL;
        }
        memcpy_elem(self, copy, self->_data, self->_length);
    }
    reset_error_trace();
    return copy;
}

static JARRAY array_subarray(JARRAY *self, size_t start, size_t end){
    if (!self){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return *self;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot determine a sub array with an empty array\n");
        return *self;
    }
    if (start > end) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "start (%zu) cannot be higher than end (%zu). It is also possible that start < 0 which would also trigger this error\n", start, end);
        return *self;
    }
    if (start >= self->_length){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "start (%zu) cannot be higher or equal than the _length of array (%zu)\n", start, self->_length);
        return *self;
    }

    // Clamp end to last element if it's out of bounds
    if (end >= self->_length)
        end = self->_length - 1;
    size_t sub_length = end - start + 1;

    // Allocate the JARRAY struct itself
    JARRAY ret_array;
    ret_array._elem_size = self->_elem_size;
    ret_array._min_alloc = sub_length;
    ret_array._length = sub_length;
    ret_array._capacity = sub_length;
    ret_array._capacity_multiplier = self->_capacity_multiplier;
    ret_array._data = malloc(sub_length * self->_elem_size);
    ret_array.user_callbacks = self->user_callbacks;
    ret_array.user_overrides = self->user_overrides;
    if (!ret_array._data) {
        create_return_error(self, JARRAY_DATA_NULL, "Failed to allocate memory for subarray _data\n");
        return *self;
    }

    // Copy relevant elements
    for (size_t i = 0; i < sub_length; i++) {
        void *src = (char*)self->_data + (start + i) * self->_elem_size;
        void *dst = (char*)ret_array._data + i * self->_elem_size;
        memcpy_elem(self, dst, src, 1);
    }

    reset_error_trace();
    return ret_array;
}

static void array_set(JARRAY *self, size_t index, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_EMPTY, "Cannot set element in an empty array");

    if (index >= self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Index cannot be higher or equal to the _length of array\n");

    // Copy the new element into the array at the given index
    memcpy_elem(self, (char*)self->_data + index * self->_elem_size, elem, 1);
    reset_error_trace();
}

static size_t* array_indexes_of(JARRAY *self, const void *elem) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (self->_length == 0){
        create_return_error(self, JARRAY_EMPTY, "Cannot search in empty array");
        return NULL;
    }
    if (self->user_callbacks.is_equal == NULL) {
        create_return_error(self, JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED, "is_equal callback not set");
        return NULL;
    }
    size_t *indexes = malloc(self->_length * sizeof(size_t));
    if (!indexes) {
        create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for indexes array");
        return NULL;
    }
    int count = 0;
    for (size_t i = 0; i < self->_length; i++) {
        if (self->user_callbacks.is_equal((char*)self->_data + i * self->_elem_size, elem)) {
            indexes[count+1] = i; // Store the index of the matching element
            count++;
        }
    }
    if (count == 0) {
        free(indexes);
        create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "No matching elements found");
        return NULL;
    }
    indexes[0] = count; // Store the count of matches at the first index
    reset_error_trace();
    return indexes;
}

static void array_for_each(JARRAY *self, void (*callback)(void *elem, void *ctx), void *ctx) {
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
    reset_error_trace();
}

static void array_clear(JARRAY *self) {
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
    jarray.reserve(self, self->_min_alloc);
    reset_error_trace();
}

static JARRAY array_clone(JARRAY *self) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return *self;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot clone an empty array");
        return *self;
    }

    JARRAY clone;
    clone._length = self->_length;
    clone._min_alloc = self->_min_alloc;
    clone._elem_size = self->_elem_size;
    clone._data_type = self->_data_type;
    clone._capacity = self->_capacity;
    clone._capacity_multiplier = self->_capacity_multiplier;
    clone._data = malloc(self->_capacity * self->_elem_size);
    if (!clone._data) {
        free(clone._data);
        create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for clone _data");
        return *self;
    }
    memcpy_elem(self, clone._data, self->_data, max_size_t(self->_length, self->_min_alloc));
    clone.user_callbacks = self->user_callbacks;
    clone.user_overrides = self->user_overrides;

    reset_error_trace();
    return clone;
}

static void array_add_all(JARRAY *self, const void *data, size_t count) {
    if (!data || count == 0)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Data is null or count is zero");

    if (self->_length + count > self->_capacity) {
        size_t new_cap = (self->_capacity > 0)
                             ? (size_t)((float)self->_capacity * self->_capacity_multiplier)
                             : 1;

        if ((float)new_cap < (float)self->_capacity * self->_capacity_multiplier)
            new_cap++;

        while (new_cap < self->_length + count) {
            size_t next = (size_t)((float)new_cap * self->_capacity_multiplier);
            if ((float)next < (float)new_cap * self->_capacity_multiplier)
                next++;
            if (next <= new_cap)
                next = new_cap + 1;
            new_cap = next;
        }

        self->_capacity = new_cap;
        void *new_data = realloc(self->_data,
                                    self->_capacity * self->_elem_size);
        if (!new_data)
            return create_return_error(self, JARRAY_DATA_NULL,
                                       "Memory allocation failed in add_all");
        self->_data = new_data;
    }

    memcpy_elem(self,
                (char *)self->_data + self->_length * self->_elem_size,
                data, count);
    self->_length += count;
    reset_error_trace();
}


static bool array_contains(JARRAY *self, const void *elem) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return false;
    }
    if (!elem) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Array cannot contain a NULL element");
        return false;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot check containment in an empty array");
        return false;
    }
    if (self->user_callbacks.is_equal == NULL) {
        create_return_error(self, JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED, "is_equal callback not set");
        return false;
    }

    reset_error_trace();

    for (size_t i = 0; i < self->_length; i++) {
        void *current_elem = (char*)self->_data + i * self->_elem_size;
        if (self->user_callbacks.is_equal(current_elem, elem)) {
            return true;
        }
    }

    return false;
}

static int compare_size_t(const void *a, const void *b) {
    size_t val_a = *(const size_t*)a;
    size_t val_b = *(const size_t*)b;
    return (val_a > val_b) - (val_a < val_b);
}

static void array_remove_all(JARRAY *self, const void *data, size_t count) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
    if (!data || count == 0) 
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Data is null or count is zero");
    
    JARRAY temp_array = jarray.init_preset(JARRAY_ULONG_PRESET);
    size_t* indexes;
    for (size_t i = 0; i < count; i++) {
        const void *elem = (const char*)data + i * self->_elem_size;
        indexes = array_indexes_of(self, elem);

        if (last_error_trace.error_code == JARRAY_ELEMENT_NOT_FOUND) {
            continue; // No matches for this element
        } 
        else if (last_error_trace.error_code == JARRAY_EMPTY) break;
        else if (last_error_trace.has_error) {
            return;
        } 

        size_t match_count = indexes[0];
        array_clear(&temp_array); // Clear temp array for new indexes
        jarray.reserve(&temp_array, match_count);
        // Store only the real indexes
        for (size_t j = 0; j < match_count; j++) {
            array_add(&temp_array, &indexes[j + 1]); // j+1 because indexes[0] is count
        }

        // Sort indexes ascending, so we can remove from 
        array_sort(&temp_array, QSORT, compare_size_t);
        // Remove in reverse order to avoid shifting
        for (size_t j = match_count; j > 0; j--) {
            size_t idx;
            idx = *(size_t*)array_at(&temp_array, j-1);
            if (last_error_trace.has_error) {
                free(indexes);
                return;
            }
            array_remove_at(self, idx);
            if (last_error_trace.has_error) {
                free(indexes);
                return;
            }
        }
        free(indexes);
    }

    reset_error_trace();
}

static size_t array_length(JARRAY *self) {
    if (!self){
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return 0;
    }

    return self->_length;
}

static void* array_reduce(JARRAY *self, void *(*reducer)(const void *accumulator, const void *elem, const void *ctx), const void *initial_value, const void *ctx) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (!reducer) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function is null");
        return NULL;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot reduce an empty array");
        return NULL;
    }

    void *accumulator = malloc(self->_elem_size);
    if (!accumulator) {
        create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for accumulator");
        return NULL;
    }

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
            create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function returned null");
            return NULL;
        }
        memcpy_elem(self, accumulator, new_accumulator, 1);
        free(new_accumulator);
    }

    reset_error_trace();
    return accumulator;
}

static JARRAY array_concat(JARRAY *arr1, JARRAY *arr2) {
    if (!arr1) {
        create_return_error(arr1, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY (arr1)");
        return *arr1;
    }
    if (!arr2) {
        create_return_error(arr2, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY (arr2)");
        return *arr1;
    }
    if (arr1->_elem_size != arr2->_elem_size) {
        create_return_error(NULL, JARRAY_INVALID_ARGUMENT, "Element sizes do not match for concatenation");
        return *arr1;
    }

    JARRAY new_array;

    new_array._elem_size = arr1->_elem_size;
    new_array._length = arr1->_length + arr2->_length;
    new_array._min_alloc = arr1->_length + arr2->_length;
    new_array._capacity = new_array._length;
    new_array._capacity_multiplier = max_size_t(arr1->_capacity_multiplier, arr2->_capacity_multiplier);
    new_array._data = malloc(new_array._length * new_array._elem_size);
    if (!new_array._data) {
        free(new_array._data);
        create_return_error(NULL, JARRAY_DATA_NULL, "Memory allocation failed for new array data");
        return *arr1;
    }

    memcpy_elem(arr1, new_array._data, arr1->_data, 1);
    memcpy_elem(arr2, (char*)new_array._data + arr1->_length * arr1->_elem_size, arr2->_data, arr2->_length);
    new_array.user_callbacks = arr1->user_callbacks;
    new_array.user_overrides = arr1->user_overrides;

    reset_error_trace();
    return new_array;
}

static char* array_join(JARRAY *self, const char *separator) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot join elements of an empty array");
        return NULL;
    }
    if (self->user_callbacks.element_to_string == NULL) {
        create_return_error(self, JARRAY_ELEMENT_TO_STRING_CALLBACK_UNINTIALIZED, "element_to_string callback not set");
        return NULL;
    }

    size_t total_length = 0;
    char **string_representations = malloc(self->_length * sizeof(char*));
    if (!string_representations) {
        create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for string representations");
        return NULL;
    }

    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        char *str = self->user_callbacks.element_to_string(elem);
        if (!str) {
            for (size_t j = 0; j < i; j++) free(string_representations[j]);
            free(string_representations);
            create_return_error(self, JARRAY_DATA_NULL, "element_to_string callback returned null");
            return NULL;
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
        create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for result string");
        return NULL;
    }

    result[0] = '\0';
    for (size_t i = 0; i < self->_length; i++) {
        strcat(result, string_representations[i]);
        if (i < self->_length - 1) strcat(result, separator);
        free(string_representations[i]);
    }
    free(string_representations);

    reset_error_trace();
    return result;
}

static void array_reverse(JARRAY *self) {
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
    reset_error_trace();
}

static bool array_any(const JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return false;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot check any on an empty array");
        return false;
    }
    if (!predicate) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Predicate function is null");
        return false;
    }

    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            return true;
        }
    }

    return false;
}

static void* array_reduce_right(JARRAY *self, void *(*reducer)(const void *accumulator, const void *elem, const void *ctx), const void *initial_value, const void *ctx) {
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (!reducer) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function is null");
        return NULL;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot reduce an empty array");
        return NULL;
    }

    void *accumulator = malloc(self->_elem_size);
    if (!accumulator) {
        create_return_error(self, JARRAY_DATA_NULL, "Memory allocation failed for accumulator");
        return NULL;
    }

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
            create_return_error(self, JARRAY_INVALID_ARGUMENT, "Reducer function returned null");
            return NULL;
        }
        memcpy_elem(self, accumulator, new_accumulator, 1);
        free(new_accumulator);
    }

    reset_error_trace();
    return accumulator;
}

static void* array_find_last(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return NULL;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
        return NULL;
    }
    if (!predicate) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element with a NULL predicate");
        return NULL;
    }
    
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + (self->_length -1 - i) * self->_elem_size;
        if (predicate(elem, ctx)) {
            reset_error_trace();
            return elem;
        }
    }
    create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
    return NULL;
}

static size_t array_find_first_index(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return self->_length;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
        return self->_length;
    }
    if (!predicate) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element with a NULL predicate");
        return self->_length;
    }
    
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + i * self->_elem_size;
        if (predicate(elem, ctx)) {
            reset_error_trace();
            return i;
        }
    }
    create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
    return self->_length;
}

static size_t array_find_last_index(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (!self) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");
        return self->_length;
    }
    if (self->_length == 0) {
        create_return_error(self, JARRAY_EMPTY, "Cannot find element in an empty array");
        return self->_length;
    }
    if (!predicate) {
        create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element with a NULL predicate");
        return self->_length;
    }
    
    for (size_t i = 0; i < self->_length; i++) {
        void *elem = (char*)self->_data + (self->_length -1 - i) * self->_elem_size;
        if (predicate(elem, ctx)) {
            reset_error_trace();
            return self->_length -1 - i;
        }
    }
    create_return_error(self, JARRAY_ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
    return self->_length;
}

static void array_fill(JARRAY *self, const void *elem, size_t start, size_t end) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot fill a NULL JARRAY");
    if (start > end)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "start (%zu) cannot be higher than end (%zu)", start, end);
    if (start >= self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "start (%zu) must be strictly lower than the length of the jarray (%zu)",
                                   start, self->_length);
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot insert NULL in a jarray");

    if (end >= self->_length) {
        size_t new_length = end + 1;

        if (new_length > self->_capacity) {
            size_t new_cap = (self->_capacity > 0)
                                 ? (size_t)((float)self->_capacity * self->_capacity_multiplier)
                                 : 1;
            if (new_cap <= self->_capacity)
                new_cap = self->_capacity + 1;

            while (new_cap < new_length) {
                size_t next = (size_t)((float)new_cap * self->_capacity_multiplier);
                if (next <= new_cap) next = new_cap + 1; /* anti-boucle */
                new_cap = next;
            }

            void *new_data = realloc(self->_data, new_cap * self->_elem_size);
            if (!new_data)
                return create_return_error(self, JARRAY_DATA_NULL,
                                           "Memory allocation failed in fill");

            self->_data = new_data;
            self->_capacity = new_cap;
        }

        self->_length = new_length;
    }

    for (size_t i = start; i <= end; i++) {
        array_set(self, i, elem);
        if (last_error_trace.has_error)
            return;
    }

    reset_error_trace();
}


static void array_shift(JARRAY *self) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot shift in a NULL JARRAY");
    if (self->_length == 0)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot shift an empty array");

    memmove((char *)self->_data,
            (char *)self->_data + self->_elem_size,
            (self->_length - 1) * self->_elem_size);

    self->_length--;

    if (self->_length == 0) {
        free(self->_data);
        self->_data = NULL;
        self->_capacity = 0;
        reset_error_trace();
        return;
    }

    size_t new_cap = self->_capacity / self->_capacity_multiplier;
    if (new_cap < self->_min_alloc)
        new_cap = self->_min_alloc;

    if (self->_length <= new_cap) {
        self->_capacity = new_cap;
        void *new_data = realloc(self->_data, new_cap * self->_elem_size);
        if (new_data)
            self->_data = new_data;
    }

    reset_error_trace();
}


static void array_shift_right(JARRAY *self, const void *elem) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot shift in a NULL JARRAY");
    if (!elem)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot insert NULL in a jarray");

    if (self->_length >= self->_capacity) {
        size_t new_cap = (self->_capacity > 0)
                             ? (size_t)(self->_capacity * self->_capacity_multiplier)
                             : 1;
        if (new_cap <= self->_capacity)
            new_cap = self->_capacity + 1;
        void *new_data =
            realloc(self->_data, new_cap * self->_elem_size);
        if (!new_data)
            return create_return_error(self, JARRAY_DATA_NULL,
                                       "Memory allocation failed in shift_right");
        self->_data = new_data;
        self->_capacity = new_cap;
    }

    if (self->_length > 0) {
        memmove((char *)self->_data + self->_elem_size,
                (char *)self->_data,
                self->_length * self->_elem_size);
    }

    self->_length++;
    return array_set(self, 0, elem);
}


static void array_splice_ext(JARRAY *self, size_t index, size_t count, va_list args) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot splice a NULL JARRAY");
    if (index > self->_length)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "index (%zu) must be <= length (%zu)", index, self->_length);

    

    // --- Suppression ---
    for (size_t i = 0; i < count; i++) {
        array_remove_at(self, index);
        if (last_error_trace.has_error) {
            if (last_error_trace.error_code == JARRAY_INDEX_OUT_OF_BOUND) break;
            else return;
        }
    }

    // --- Insertion ---
    size_t offset = 0;
    void *element = va_arg(args, void*);
    while (element != NULL) {
        array_add_at(self, index + offset, element);
        if (last_error_trace.has_error) {
            return;
        }
        offset++;
        element = va_arg(args, void*);
    }
    reset_error_trace();
}

static void array_splice(JARRAY *self, size_t index, size_t count, ...) {
    va_list args;
    va_start(args, count);
    array_splice_ext(self, index, count, args);
    va_end(args);
}

static void array_addm_ext(JARRAY *self, va_list args) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT, "Cannot find element in a NULL JARRAY");

    
    void *element = va_arg(args, void*);
    while (element != NULL) {
        array_add(self, element);
        if (last_error_trace.has_error) {
            return;
        }
        element = va_arg(args, void*);
    }
    reset_error_trace();
}

static void array_addm(JARRAY *self, ...){
    va_list args;
    va_start(args, self);
    array_addm_ext(self, args);
    va_end(args);
}

static void array_reserve(JARRAY *self, size_t capacity) {
    if (!self)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot reserve capacity on a NULL JARRAY");

    if (capacity == 0)
        return create_return_error(self, JARRAY_INVALID_ARGUMENT,
                                   "Cannot reserve zero capacity");

    size_t required_cap = (capacity > self->_length) ? capacity : self->_length;
    if (required_cap <= self->_capacity) {
        self->_min_alloc = capacity;
        reset_error_trace();
        return;
    }

    void *new_data = (self->_data)
                         ? realloc(self->_data, required_cap * self->_elem_size)
                         : malloc(required_cap * self->_elem_size);

    if (!new_data)
        return create_return_error(self, JARRAY_DATA_NULL,
                                   "Memory allocation failed when reserving");

    self->_data = new_data;
    self->_capacity = required_cap;
    self->_min_alloc = capacity;

    reset_error_trace();
}


static void array_init_reserve(JARRAY *self, size_t elem_size, size_t capacity, JARRAY_DATA_TYPE data_type, JARRAY_USER_CALLBACK_IMPLEMENTATION imp){
    jarray.init(self, elem_size, data_type, imp);
    if (last_error_trace.has_error) return;
    jarray.reserve(self, capacity);
}

extern JARRAY create_jarray_string(void);
extern JARRAY create_jarray_int(void);
extern JARRAY create_jarray_float(void);
extern JARRAY create_jarray_char(void);
extern JARRAY create_jarray_double(void);
extern JARRAY create_jarray_long(void);
extern JARRAY create_jarray_short(void);
extern JARRAY create_jarray_ulong(void);
extern JARRAY create_jarray_ushort(void);
extern JARRAY create_jarray_uint(void);

static JARRAY array_init_preset(JARRAY_TYPE_PRESET preset){
    JARRAY (*ret_func)(void) = NULL;
    switch (preset){
        case JARRAY_STRING_PRESET:
            ret_func = create_jarray_string;
            break;
        case JARRAY_INT_PRESET:
            ret_func = create_jarray_int;
            break;
        case JARRAY_FLOAT_PRESET:
            ret_func = create_jarray_float;
            break;
        case JARRAY_CHAR_PRESET:
            ret_func = create_jarray_char;
            break;
        case JARRAY_DOUBLE_PRESET:
            ret_func = create_jarray_double;
            break;
        case JARRAY_LONG_PRESET:
            ret_func = create_jarray_long;
            break;
        case JARRAY_SHORT_PRESET:
            ret_func = create_jarray_short;
            break;
        case JARRAY_UINT_PRESET:
            ret_func = create_jarray_uint;
            break;
        case JARRAY_USHORT_PRESET:
            ret_func = create_jarray_ushort;
            break;
        case JARRAY_ULONG_PRESET:
            ret_func = create_jarray_ulong;
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