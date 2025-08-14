#include "my_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/// Lookup table mapping ARRAY_ERROR enum values to their corresponding string descriptions.
const char *enum_to_string[] = {
    [INDEX_OUT_OF_BOUND] = "Index out of bound",
    [ARRAY_UNINITIALIZED] = "Array uninitialized",
    [DATA_NULL] = "Data is null"
};

/**
 * @brief Prints an error message from an ARRAY_RETURN.
 *
 * If the ARRAY_RETURN contains an error, prints it in red and frees the allocated message string.
 *
 * @param ret The ARRAY_RETURN to inspect.
 */
void print_array_err(ARRAY_RETURN ret) {
    if (ret.has_value) return;
    printf("\033[31m%s\033[0m\n", ret.error.error_msg);
    free((void*)ret.error.error_msg);
}

/**
 * @brief Creates an ARRAY_RETURN object representing an error, with a formatted message.
 *
 * Allocates memory for the error message so that it can persist after the function returns.
 * The caller is responsible for freeing the allocated error message string.
 *
 * @param error_code The ARRAY_ERROR code to store.
 * @param fmt Format string for the error message (printf-style).
 * @param ... Arguments to be formatted into the message.
 * @return ARRAY_RETURN containing the error code and dynamically allocated error message.
 */
ARRAY_RETURN create_return_error(ARRAY_ERROR error_code, const char* fmt, ...) {
    ARRAY_RETURN ret;
    va_list args;
    va_start(args, fmt);

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
    ret.error.error_msg = buf;
    return ret;
}

/**
 * @brief Retrieves an element at the specified index.
 *
 * Returns a pointer to the element inside the array's data buffer.
 * The pointer is valid as long as the array is not reallocated or freed.
 *
 * @param self Pointer to the Array instance.
 * @param index Index of the element to retrieve.
 * @return ARRAY_RETURN containing a pointer to the element, or an error if out of bounds.
 */
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

/**
 * @brief Appends an element to the end of the array.
 *
 * Resizes the array by one element and copies the provided data into the new slot.
 *
 * @param self Pointer to the Array instance.
 * @param elem Pointer to the element to add.
 * @return ARRAY_RETURN containing a pointer to the newly added element, or an error.
 */
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

/**
 * @brief Inserts an element at a specific index in the array.
 *
 * Shifts elements to the right to make space for the new element.
 *
 * @param self Pointer to the Array instance.
 * @param index Index where the element should be inserted.
 * @param elem Pointer to the element to insert.
 * @return ARRAY_RETURN containing a pointer to the inserted element, or an error.
 */
ARRAY_RETURN array_add_at(Array *self, size_t index, const void *elem) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (index > self->length)
        return create_return_error(INDEX_OUT_OF_BOUND, "Index %zu out of bound for insert", index);

    void *new_data = realloc(self->data, (self->length + 1) * self->elem_size);
    if (!new_data)
        return create_return_error(DATA_NULL, "Memory allocation failed in add_at");

    self->data = new_data;

    memmove(
        (char*)self->data + (index + 1) * self->elem_size,
        (char*)self->data + index * self->elem_size,
        (self->length - index) * self->elem_size
    );

    memcpy((char*)self->data + index * self->elem_size, elem, self->elem_size);
    self->length++;

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = (char*)self->data + index * self->elem_size;
    return ret;
}

/**
 * @brief Removes an element at a specific index from the array.
 *
 * Shifts remaining elements to fill the gap. The removed element is returned
 * in newly allocated memory, which must be freed by the caller.
 *
 * @param self Pointer to the Array instance.
 * @param index Index of the element to remove.
 * @return ARRAY_RETURN containing a pointer to the removed element, or an error.
 */
ARRAY_RETURN array_remove_at(Array *self, size_t index) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (index >= self->length)
        return create_return_error(INDEX_OUT_OF_BOUND, "Index %zu out of bound for remove", index);

    void *removed_elem = malloc(self->elem_size);
    if (!removed_elem)
        return create_return_error(DATA_NULL, "Memory allocation failed in remove_at");
    memcpy(removed_elem, (char*)self->data + index * self->elem_size, self->elem_size);

    memmove(
        (char*)self->data + index * self->elem_size,
        (char*)self->data + (index + 1) * self->elem_size,
        (self->length - index - 1) * self->elem_size
    );

    self->length--;

    if (self->length > 0) {
        void *new_data = realloc(self->data, self->length * self->elem_size);
        if (new_data) self->data = new_data;
    } else {
        free(self->data);
        self->data = NULL;
    }

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = removed_elem;
    return ret;
}

/**
 * @brief Removes the last element from the array.
 *
 * @param self Pointer to the Array instance.
 * @return ARRAY_RETURN containing a pointer to the removed element, or an error.
 */
ARRAY_RETURN array_remove(Array *self) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot remove from empty array");

    size_t last_index = self->length - 1;
    return array_remove_at(self, last_index);
}

/**
 * @brief Initializes an array with the given element size.
 *
 * @param array Pointer to the Array instance to initialize.
 * @param elem_size Size of each element in bytes.
 * @return ARRAY_RETURN containing a pointer to the initialized array.
 */
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

/**
 * @brief Initializes an array with pre-existing data.
 *
 * @param array Pointer to the Array instance to initialize.
 * @param data Pointer to the data buffer.
 * @param length Number of elements in the data buffer.
 * @param elem_size Size of each element in bytes.
 * @return ARRAY_RETURN containing a pointer to the initialized array.
 */
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

/**
 * @brief Filters an array using a predicate function.
 *
 * Creates a new array containing only the elements that satisfy the predicate.
 *
 * @param self Pointer to the Array instance.
 * @param predicate Function that returns true for elements to keep.
 * @return ARRAY_RETURN containing a pointer to the new filtered Array, or an error.
 */
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

/**
 * @brief Prints the contents of the array using a user-defined callback.
 *
 * @param array Pointer to the Array instance.
 * @return ARRAY_RETURN containing a pointer to the array, or an error.
 */
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

/**
 * @brief Sorts the array and returns a new sorted copy.
 *
 * Uses the specified sorting algorithm and the user-provided compare function.
 *
 * @param self Pointer to the Array instance.
 * @param method Sorting method to use (QSORT, BUBBLE_SORT, INSERTION_SORT, SELECTION_SORT).
 * @return ARRAY_RETURN containing a pointer to the new sorted Array, or an error.
 */
ARRAY_RETURN array_sort(Array *self, SORT_METHOD method) {
    int (*compare)(const void*, const void*) = self->user_implementation.compare;
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot sort an empty array");

    if (compare == NULL)
        return create_return_error(PRINT_ELEMENT_CALLBACK_UNINTIALIZED, "Compare callback not set");

    void *copy_data = malloc(self->length * self->elem_size);
    if (!copy_data)
        return create_return_error(DATA_NULL, "Memory allocation failed in array_sort");

    memcpy(copy_data, self->data, self->length * self->elem_size);

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

/**
 * @brief Finds the first element in the array that satisfies a given predicate.
 *
 * This function iterates over each element of the array and applies the provided
 * predicate function. If the predicate returns true for an element, the search stops,
 * and that element is returned. If no element satisfies the predicate, an error is returned.
 *
 * @param self       Pointer to the Array structure.
 * @param predicate  Function pointer to a predicate function that takes a `const void*` 
 *                   (pointer to the element) and returns a boolean indicating whether
 *                   the element matches the desired condition.
 *
 * @return ARRAY_RETURN
 *         - On success: `.has_value = true` and `.value` points to the matching element.
 *         - On failure: `.has_value = false` and `.error` contains error information:
 *              - ARRAY_UNINITIALIZED: The array has not been initialized.
 *              - ELEMENT_NOT_FOUND: No element satisfies the predicate.
 */
ARRAY_RETURN array_find_by_predicate(struct Array *self, bool (*predicate)(const void *)){
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");

    ARRAY_RETURN ret;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem)) {
            ret.has_value = true;
            ret.value = elem;
            return ret;
        }
    }
    return create_return_error(ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
}

/**
 * @brief Gets the data in array self.
 * @param self Pointer to the Array structure.
 * @return ARRAY_RETURN
 *         - On success: `.has_value = true` and `.value` points to the data.
 *         - On failure: `.has_value = false` and `.error` contains error information:
 *              - ARRAY_UNINITIALIZED: The array has not been initialized.
 */
ARRAY_RETURN array_data(struct Array *self){
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized");
    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = self->data;
    return ret;
}

ARRAY_RETURN array_subarray(struct Array *self, size_t low_index, size_t high_index){
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "Array not initialized\n");
    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot determine a sub array with an empty array\n");
    if (low_index > high_index)
        return create_return_error(INVALID_ARGUMENT, "low_index cannot be higher than high_index. It is also possible that low_index < 0 which would also trigger this error\n");
    if (low_index >= self->length)
        return create_return_error(INVALID_ARGUMENT, "low_index cannot be higher than the length of array\n");

    // Clamp high_index to last element if it's out of bounds
    if (high_index >= self->length)
        high_index = self->length - 1;
    size_t sub_length = high_index - low_index + 1;

    // Allocate the Array struct itself
    Array *ret_array = malloc(sizeof(Array));
    if (!ret_array)
        return create_return_error(DATA_NULL, "Failed to allocate memory for subarray struct\n");

    ret_array->state = INITIALIZED;
    ret_array->elem_size = self->elem_size;
    ret_array->length = sub_length;
    ret_array->data = malloc(sub_length * self->elem_size);
    ret_array->user_implementation = self->user_implementation;
    if (!ret_array->data) {
        free(ret_array);
        return create_return_error(DATA_NULL, "Failed to allocate memory for subarray data\n");
    }

    // Copy relevant elements
    for (size_t i = 0; i < sub_length; i++) {
        void *src = (char*)self->data + (low_index + i) * self->elem_size;
        void *dst = (char*)ret_array->data + i * self->elem_size;
        memcpy(dst, src, self->elem_size);
    }

    ARRAY_RETURN ret;
    ret.has_value = true;
    ret.value = ret_array; // directly store pointer to new Array
    return ret;
}





/// Static interface implementation for easier usage.
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
    .sort = array_sort,
    .find_by_predicate = array_find_by_predicate,
    .data = array_data,
    .subarray = array_subarray,
};
