#include "jarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

/// Lookup table mapping JARRAY_ERROR enum values to their corresponding string descriptions.
const char *enum_to_string[] = {
    [INDEX_OUT_OF_BOUND]               = "Index out of bound",
    [ARRAY_UNINITIALIZED]              = "JArray uninitialized",
    [DATA_NULL]                        = "Data is null",
    [PRINT_ELEMENT_CALLBACK_UNINTIALIZED] = "Print callback not set",
    [EMPTY_ARRAY]                      = "Empty array",
    [INVALID_ARGUMENT]                 = "Invalid argument",
    [COMPARE_CALLBACK_UNINTIALIZED]    = "Compare callback not set",
    [IS_EQUAL_CALLBACK_UNINTIALIZED]   = "is_equal callback not set",
    [ELEMENT_NOT_FOUND]                = "Element not found",
    [UNIMPLEMENTED_FUNCTION]           = "Function not implemented",
};


/**
 * @brief Prints an error message from an JARRAY_RETURN.
 *
 * If the JARRAY_RETURN contains an error, prints it in red and frees the allocated message string.
 *
 * @param ret The JARRAY_RETURN to inspect.
 */
void print_array_err(JARRAY_RETURN ret) {
    if (ret.has_value) return;
    if (!ret.has_error) return;
    if (ret.error.error_code < 0 || ret.error.error_code >= sizeof(enum_to_string) / sizeof(enum_to_string[0]) || enum_to_string[ret.error.error_code] == NULL) {
        fprintf(stderr, "[\033[31mUnknown error code: %d\033[0m]", ret.error.error_code);
    } else {
        fprintf(stderr, "[\033[31mError: %s\033[0m]", enum_to_string[ret.error.error_code]);
    }
    if (ret.error.error_msg) {
        fprintf(stderr, "\t%s\n", ret.error.error_msg);
    }
    free((void*)ret.error.error_msg);
}

/**
 * @brief Frees the memory allocated for an JArray.
 *
 * This function frees the data buffer and resets the JArray's state.
 *
 * @param array Pointer to the JArray instance to free.
 */
void array_free(JArray *array) {
    if (!array) return;
    free(array->data);
    array->data = NULL;
    array->length = 0;
    array->elem_size = 0;
    array->state = UNINITIALIZED;
    memset(&array->user_implementation, 0, sizeof(array->user_implementation));
}


/**
 * @brief Creates an JARRAY_RETURN object representing an error, with a formatted message.
 *
 * Allocates memory for the error message so that it can persist after the function returns.
 * The caller is responsible for freeing the allocated error message string.
 *
 * @param error_code The JARRAY_ERROR code to store.
 * @param fmt Format string for the error message (printf-style).
 * @param ... Arguments to be formatted into the message.
 * @return JARRAY_RETURN containing the error code and dynamically allocated error message.
 */
JARRAY_RETURN create_return_error(JARRAY_ERROR error_code, const char* fmt, ...) {
    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = true;
    va_list args;
    va_start(args, fmt);

    char *buf = malloc(256);
    if (!buf) {
        ret.error.error_msg = "Memory allocation failed";
        ret.error.error_code = ARRAY_UNINITIALIZED;
        va_end(args);
        return ret;
    }

    vsnprintf(buf, 256, fmt, args);
    va_end(args);

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
 * @param self Pointer to the JArray instance.
 * @param index Index of the element to retrieve.
 * @return JARRAY_RETURN containing a pointer to the element, or an error if out of bounds.
 */
JARRAY_RETURN array_at(JArray *self, size_t index) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    if (!self->data) 
        return create_return_error(DATA_NULL, "Data field of array is null");

    if (index >= self->length) 
        return create_return_error(INDEX_OUT_OF_BOUND, "Index %zu is out of bound", index);

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = (char*)self->data + index * self->elem_size;
    return ret;
}

/**
 * @brief Appends an element to the end of the array.
 *
 * Resizes the array by one element and copies the provided data into the new slot.
 *
 * @param self Pointer to the JArray instance.
 * @param elem Pointer to the element to add.
 * @return JARRAY_RETURN containing a pointer to the newly added element, or an error.
 */
JARRAY_RETURN array_add(JArray *self, const void *elem) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    void *new_data = realloc(self->data, (self->length + 1) * self->elem_size);
    if (!new_data) 
        return create_return_error(DATA_NULL, "Memory allocation failed in add");

    self->data = new_data;
    memcpy((char*)self->data + self->length * self->elem_size, elem, self->elem_size);
    self->length++;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Inserts an element at a specific index in the array.
 *
 * Shifts elements to the right to make space for the new element.
 *
 * @param self Pointer to the JArray instance.
 * @param index Index where the element should be inserted.
 * @param elem Pointer to the element to insert.
 * @return JARRAY_RETURN containing a pointer to the inserted element, or an error.
 */
JARRAY_RETURN array_add_at(JArray *self, size_t index, const void *elem) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

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

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Removes an element at a specific index from the array.
 *
 * Shifts remaining elements to fill the gap. The removed element is returned
 * in newly allocated memory, which must be freed by the caller.
 *
 * @param self Pointer to the JArray instance.
 * @param index Index of the element to remove.
 * @return JARRAY_RETURN containing a pointer to the removed element, or an error.
 */
JARRAY_RETURN array_remove_at(JArray *self, size_t index) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

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

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = removed_elem;
    return ret;
}

/**
 * @brief Removes the last element from the array.
 *
 * @param self Pointer to the JArray instance.
 * @return JARRAY_RETURN containing a pointer to the removed element, or an error.
 */
JARRAY_RETURN array_remove(JArray *self) {
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot remove from empty array");

    size_t last_index = self->length - 1;
    return array_remove_at(self, last_index);
}

/**
 * @brief Initializes an array with the given element size.
 *
 * @param array Pointer to the JArray instance to initialize.
 * @param elem_size Size of each element in bytes.
 * @return JARRAY_RETURN containing a pointer to the initialized array.
 */
JARRAY_RETURN array_init(JArray *array, size_t elem_size) {
    array->data = NULL;
    array->length = 0;
    array->elem_size = elem_size;
    array->state = INITIALIZED;
    
    array->user_implementation.print_element_callback = NULL;
    array->user_implementation.compare = NULL;
    array->user_implementation.is_equal = NULL;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Initializes an array with pre-existing data.
 *
 * @param array Pointer to the JArray instance to initialize.
 * @param data Pointer to the data buffer.
 * @param length Number of elements in the data buffer.
 * @param elem_size Size of each element in bytes.
 * @return JARRAY_RETURN containing a pointer to the initialized array.
 */
JARRAY_RETURN array_init_with_data(JArray *array, void *data, size_t length, size_t elem_size) {
    array->data = data;
    array->length = length;
    array->elem_size = elem_size;
    array->state = INITIALIZED;

    array->user_implementation.print_element_callback = NULL;
    array->user_implementation.compare = NULL;
    array->user_implementation.is_equal = NULL;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Filters an array using a predicate function.
 *
 * Creates a new array containing only the elements that satisfy the predicate.
 *
 * @param self Pointer to the JArray instance.
 * @param predicate Function that returns true for elements to keep.
 * @param ctx Pointer to context of predicate
 * @return JARRAY_RETURN containing a pointer to the new filtered JArray, or an error.
 */
JARRAY_RETURN array_filter(JArray *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    size_t count = 0;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem, ctx)) count++;
    }

    JArray *result = malloc(sizeof(JArray));
    result->length = count;
    result->elem_size = self->elem_size;
    result->state = INITIALIZED;
    result->data = malloc(count * self->elem_size);
    result->user_implementation = self->user_implementation;

    size_t j = 0;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem, ctx)) {
            memcpy((char*)result->data + j * self->elem_size, elem, self->elem_size);
            j++;
        }
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = result;
    return ret;
}

/**
 * @brief Prints the contents of the array using a user-defined callback.
 *
 * @param array Pointer to the JArray instance.
 * @return JARRAY_RETURN containing a pointer to the array, or an error.
 */
JARRAY_RETURN array_print(JArray *array) {
    if (array->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    if (array->user_implementation.print_element_callback == NULL)
        return create_return_error(PRINT_ELEMENT_CALLBACK_UNINTIALIZED, "The print single element callback not set\n");
    for (size_t i = 0; i < array->length; i++) {
        void *elem = (char*)array->data + i * array->elem_size;
        array->user_implementation.print_element_callback(elem);
    }
    printf("\n");

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Sorts the array and returns a new sorted copy.
 *
 * Uses the specified sorting algorithm and the user-provided compare function.
 *
 * @param self Pointer to the JArray instance.
 * @param method Sorting method to use (QSORT, BUBBLE_SORT, INSERTION_SORT, SELECTION_SORT).
 * @return JARRAY_RETURN containing a pointer to the new sorted JArray, or an error.
 */
JARRAY_RETURN array_sort(JArray *self, SORT_METHOD method) {
    int (*compare)(const void*, const void*) = self->user_implementation.compare;
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot sort an empty array");

    if (compare == NULL)
        return create_return_error(COMPARE_CALLBACK_UNINTIALIZED, "Compare callback not set");

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

    self->data = copy_data;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Finds the first element in the array that satisfies a given predicate.
 *
 * This function iterates over each element of the array and applies the provided
 * predicate function. If the predicate returns true for an element, the search stops,
 * and that element is returned. If no element satisfies the predicate, an error is returned.
 *
 * @param self       Pointer to the JArray structure.
 * @param predicate  Function pointer to a predicate function that takes a `const void*` 
 *                   (pointer to the element) and returns a boolean indicating whether
 *                   the element matches the desired condition.
 * @param ctx        Pointer to context
 *
 * @return JARRAY_RETURN
 *         - On success: `.has_value = true` and `.value` points to the matching element.
 *         - On failure: `.has_value = false` and `.error` contains error information:
 *              - ARRAY_UNINITIALIZED: The array has not been initialized.
 *              - ELEMENT_NOT_FOUND: No element satisfies the predicate.
 */
JARRAY_RETURN array_find_first(struct JArray *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx){
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot find element in an empty array");
    JARRAY_RETURN ret;
    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        if (predicate(elem, ctx)) {
            ret.has_value = true;
            ret.has_error = false;
            ret.value = elem;
            return ret;
        }
    }
    return create_return_error(ELEMENT_NOT_FOUND, "Found no element corrsponding with predicate conditions\n");
}

/**
 * @brief Gets the data in array self.
 * @param self Pointer to the JArray structure.
 * @return JARRAY_RETURN
 *         - On success: `.has_value = true` and `.value` points to the data.
 *         - On failure: `.has_value = false` and `.error` contains error information:
 *              - ARRAY_UNINITIALIZED: The array has not been initialized.
 */
JARRAY_RETURN array_data(struct JArray *self) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    JARRAY_RETURN ret;

    void *copy = NULL;
    if (self->length > 0) {
        copy = malloc(self->length * self->elem_size);
        if (!copy)
            return create_return_error(DATA_NULL, "Failed to allocate data copy");
        memcpy(copy, self->data, self->length * self->elem_size);
    }

    ret.has_value = true;
    ret.has_error = false;
    ret.value = copy; // caller now owns this pointer (may be NULL if length==0)
    return ret;
}


/**
 * @brief Create a subarray from a given JArray.
 * 
 * @details Allocates a new JArray containing elements from `low_index` to `high_index` (inclusive) of the original array.
 * Copies the relevant elements into the new JArray. The caller is responsible for freeing the subarray's data.
 * 
 * @param self Pointer to the original JArray.
 * @param low_index Starting index of the subarray (inclusive).
 * @param high_index Ending index of the subarray (inclusive).
 * @return JARRAY_RETURN On success, contains a pointer to the new subarray. On failure, contains an error code and message.
 */
JARRAY_RETURN array_subarray(struct JArray *self, size_t low_index, size_t high_index){
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized\n");
    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot determine a sub array with an empty array\n");
    if (low_index > high_index)
        return create_return_error(INVALID_ARGUMENT, "low_index cannot be higher than high_index. It is also possible that low_index < 0 which would also trigger this error\n");
    if (low_index >= self->length)
        return create_return_error(INVALID_ARGUMENT, "low_index cannot be higher or equal than the length of array\n");
    

    // Clamp high_index to last element if it's out of bounds
    if (high_index >= self->length)
        high_index = self->length - 1;
    size_t sub_length = high_index - low_index + 1;

    // Allocate the JArray struct itself
    JArray *ret_array = malloc(sizeof(JArray));
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

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = ret_array; // directly store pointer to new JArray
    return ret;
}

/**
 * @brief Sets the element at the given index in the array.
 * @return JARRAY_RETURN with success or error.
 */
JARRAY_RETURN array_set(struct JArray *self, size_t index, const void *elem) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized\n");

    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot set element in an empty array");

    if (index >= self->length)
        return create_return_error(INVALID_ARGUMENT, "Index cannot be higher or equal to the length of array\n");

    // Copy the new element into the array at the given index
    memcpy((char*)self->data + index * self->elem_size, elem, self->elem_size);

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}


static void swap_size_t(size_t *a, size_t *b) {
    size_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static int distance_int(const int *data, size_t index, int target) {
    return abs(data[index] - target);
}

static void quicksort_indexes(size_t *indexes, int *data, size_t left, size_t right, int target) {
    if (left >= right) return;

    size_t i = left, j = right;
    int pivot = distance_int(data, indexes[(left + right) / 2], target);

    while (i <= j) {
        while (distance_int(data, indexes[i], target) < pivot) i++;
        while (distance_int(data, indexes[j], target) > pivot) j--;

        if (i <= j) {
            swap_size_t(&indexes[i], &indexes[j]);
            i++;
            if (j > 0) j--;
        }
    }

    if (j > left) quicksort_indexes(indexes, data, left, j, target);
    if (i < right) quicksort_indexes(indexes, data, i, right, target);
}

/**
 * @brief Find all indexes in the array whose values match a target value.
 *
 * Error cases handled:
 *   - Empty array → returns `EMPTY_ARRAY` error.
 *   - JArray not initialized → returns `ARRAY_UNINITIALIZED` error.
 *   - Memory allocation failure → returns `DATA_NULL` error.
 *   - No matches found → returns `ELEMENT_NOT_FOUND` error.
 *
 * @param self Pointer to the JArray structure.
 * @param elem Pointer to the target value to find (currently assumes `int` type).
 * @return JARRAY_RETURN containing either:
 *         - `value` → `size_t[]` where first element is match count, followed by match indexes.
 *         - or error code if no match or failure occurs.
 */
JARRAY_RETURN array_find_indexes(struct JArray *self, const void *elem) {
    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot search in empty array");
    if (self->state != INITIALIZED)
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    if (self->user_implementation.is_equal == NULL) {
        return create_return_error(IS_EQUAL_CALLBACK_UNINTIALIZED, "is_equal callback not set");
    }
    size_t *indexes = malloc(self->length * sizeof(size_t));
    if (!indexes) {
        return create_return_error(DATA_NULL, "Memory allocation failed for indexes array");
    }
    int count = 0;
    for (size_t i = 0; i < self->length; i++) {
        if (self->user_implementation.is_equal((char*)self->data + i * self->elem_size, elem)) {
            indexes[i+1] = count; // Store the index of the matching element
            count++;
        }
    }
    if (count == 0) {
        free(indexes);
        return create_return_error(ELEMENT_NOT_FOUND, "No matching elements found");
    }
    indexes[0] = count; // Store the count of matches at the first index

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = indexes; // Caller must free this pointer
    return ret;
}

/**
 * @brief Applies a callback function to each element in the array.
 *
 * Iterates over each element and calls the provided callback with the element and context.
 *
 * @param self Pointer to the JArray instance.
 * @param callback Function to call for each element.
 * @param ctx Context pointer passed to the callback.
 * @return JARRAY_RETURN containing success or error information.
 */
JARRAY_RETURN array_for_each(struct JArray *self, void (*callback)(void *elem, void *ctx), void *ctx) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    if (!callback) 
        return create_return_error(INVALID_ARGUMENT, "Callback function is null");
    if (self->length == 0)
        return create_return_error(EMPTY_ARRAY, "Cannot iterate over an empty array");

    for (size_t i = 0; i < self->length; i++) {
        void *elem = (char*)self->data + i * self->elem_size;
        callback(elem, ctx);
    }

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Clears the array, removing all elements.
 *
 * Resets the array to an empty state without freeing the underlying data buffer.
 *
 * @param self Pointer to the JArray instance.
 * @return JARRAY_RETURN containing success or error information.
 */
JARRAY_RETURN array_clear(struct JArray *self) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    if (self->data == NULL) 
        return create_return_error(DATA_NULL, "Data field of array is null");
    // Free existing data
    free(self->data);
    self->data = NULL;
    self->length = 0;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Clones the array, creating a new JArray with the same elements.
 *
 * Allocates a new JArray and copies all elements from the original array.
 *
 * @param self Pointer to the JArray instance to clone.
 * @return JARRAY_RETURN containing a pointer to the cloned JArray, or an error.
 */
JARRAY_RETURN array_clone(struct JArray *self) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");
    if (self->length == 0) 
        return create_return_error(EMPTY_ARRAY, "Cannot clone an empty array");

    JArray *clone = malloc(sizeof(JArray));
    if (!clone) 
        return create_return_error(DATA_NULL, "Memory allocation failed for clone");

    clone->length = self->length;
    clone->elem_size = self->elem_size;
    clone->state = INITIALIZED;
    clone->data = malloc(self->length * self->elem_size);
    if (!clone->data) {
        free(clone);
        return create_return_error(DATA_NULL, "Memory allocation failed for clone data");
    }
    memcpy(clone->data, self->data, self->length * self->elem_size);
    clone->user_implementation = self->user_implementation;

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = clone; // caller must free
    return ret;
}

/**
 * @brief Adds multiple elements to the array from a data buffer.
 *
 * Resizes the array to accommodate the new elements and copies them into the array.
 *
 * @param self Pointer to the JArray instance.
 * @param data Pointer to the data buffer containing elements to add.
 * @param length Number of elements in the data buffer.
 * @return JARRAY_RETURN containing success or error information.
 */
JARRAY_RETURN array_add_all(JArray *self, const void *data, size_t count) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    if (!data || count == 0) 
        return create_return_error(INVALID_ARGUMENT, "Data is null or count is zero");

    void *new_data = realloc(self->data, (self->length + count) * self->elem_size);
    if (!new_data) 
        return create_return_error(DATA_NULL, "Memory allocation failed in add_all");

    self->data = new_data;
    memcpy((char*)self->data + self->length * self->elem_size, data, count * self->elem_size);
    self->length += count;

    JARRAY_RETURN ret;
    ret.has_value = false;
    ret.has_error = false;
    ret.value = NULL;
    return ret;
}

/**
 * @brief Checks if the array contains a specific element.
 *
 * Uses the user-defined equality function to compare elements.
 *
 * @param self Pointer to the JArray instance.
 * @param elem Pointer to the element to check for.
 * @return JARRAY_RETURN containing true if found, false otherwise, or an error.
 */
JARRAY_RETURN array_contains(struct JArray *self, const void *elem) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    if (self->length == 0) 
        return create_return_error(EMPTY_ARRAY, "Cannot check containment in an empty array");

    if (self->user_implementation.is_equal == NULL) 
        return create_return_error(IS_EQUAL_CALLBACK_UNINTIALIZED, "is_equal callback not set");

    for (size_t i = 0; i < self->length; i++) {
        void *current_elem = (char*)self->data + i * self->elem_size;
        if (self->user_implementation.is_equal(current_elem, elem)) {
            JARRAY_RETURN ret;
            ret.has_value = true;
            ret.has_error = false;
            ret.value = TO_POINTER(bool, true); // return pointer to the found element
            return ret;
        }
    }

    JARRAY_RETURN ret;
    ret.has_value = true;
    ret.has_error = false;
    ret.value = TO_POINTER(bool, false); // return pointer to false if not found
    return ret;
}

/**
 * @brief Removes all occurrences of elements also contained in the provided data buffer.
 * This function iterates over the array and removes elements that match any in the provided data.
 * @param self Pointer to the JArray instance.
 * @param data Pointer to the data buffer containing elements to remove.
 * @return JARRAY_RETURN containing success or error information.
 */
JARRAY_RETURN array_remove_all(JArray *self, const void *data, size_t count) {
    if (self->state != INITIALIZED) 
        return create_return_error(ARRAY_UNINITIALIZED, "JArray not initialized");

    if (!data || count == 0) 
        return create_return_error(INVALID_ARGUMENT, "Data is null or count is zero");


    JArray temp_array;
    array_init(&temp_array, sizeof(size_t));
    for (size_t i = 0; i < count; i++) {
        const void *elem = (const char*)data + i * self->elem_size;
        JARRAY_RETURN ret = array_find_indexes(self, elem);

        if (ret.has_error && ret.error.error_code == ELEMENT_NOT_FOUND) {
            continue; // No matches for this element
        } 
        else if (ret.has_error && ret.error.error_code == EMPTY_ARRAY) break;
        else if (ret.has_error) {
            return ret; // Some other error
        } 
        else if (!ret.has_value) {
            return create_return_error(INVALID_ARGUMENT, "Unexpected return value from find_indexes");
        }

        size_t *indexes = (size_t*)ret.value;
        size_t match_count = indexes[0];
        printf("Found %zu matches for element %zu\n", match_count, i);
        array_clear(&temp_array); // Clear temp array for new indexes
        // Store only the real indexes
        for (size_t j = 0; j < match_count; j++) {
            array_add(&temp_array, &indexes[j + 1]); // j+1 because indexes[0] is count
        }

        // Sort indexes ascending, so we can remove from the end
        array_sort(&temp_array, QSORT);

        // Remove in reverse order to avoid shifting
        for (size_t j = match_count; j > 0; j--) {
            size_t idx;
            JARRAY_RETURN ret_at = array_at(&temp_array, j-1); // j-1 is safe now
            if (ret_at.has_error) {
                free(indexes);
                return ret_at; // Error retrieving index
            }
            idx = RET_GET_VALUE(size_t, ret_at);
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
    .init_with_data = array_init_with_data,
    .print_array_err = print_array_err,
    .free = array_free,
    .sort = array_sort,
    .find_first = array_find_first,
    .data = array_data,
    .subarray = array_subarray,
    .set = array_set,
    .find_indexes = array_find_indexes,
    .for_each = array_for_each,
    .clear = array_clear,
    .clone = array_clone,
    .add_all = array_add_all,
    .contains = array_contains,
    .remove_all = array_remove_all
};
