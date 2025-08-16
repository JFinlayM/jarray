#ifndef JARRAY_H
#define JARRAY_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    INDEX_OUT_OF_BOUND = 0,
    ARRAY_UNINITIALIZED,
    DATA_NULL,
    PRINT_ELEMENT_CALLBACK_UNINTIALIZED,
    COMPARE_CALLBACK_UNINTIALIZED,
    IS_EQUAL_CALLBACK_UNINTIALIZED,
    EMPTY_ARRAY,
    ELEMENT_NOT_FOUND,
    INVALID_ARGUMENT,
    UNIMPLEMENTED_FUNCTION,
} ARRAY_ERROR;

typedef enum {
    UNINITIALIZED = 0,
    INITIALIZED
} Array_state_t;

typedef struct{
    ARRAY_ERROR error_code;
    char* error_msg;
} ARRAY_RETURN_ERROR;

typedef struct {
    union {
        void* value;
        ARRAY_RETURN_ERROR error;
    };
    bool has_value;
    bool has_error;
} ARRAY_RETURN;


typedef struct USER_FUNCTION_IMPLEMENTATION {
    void (*print_element_callback)(const void*);
    int (*compare)(const void*, const void*);
    bool (*is_equal)(const void*, const void*);
}USER_FUNCTION_IMPLEMENTATION;

typedef struct Array {
    void *data;
    size_t length;
    size_t elem_size;
    Array_state_t state;
    USER_FUNCTION_IMPLEMENTATION user_implementation;
} Array;


typedef enum SORT_METHOD {
    QSORT = 0,
    BUBBLE_SORT,
    INSERTION_SORT,
    SELECTION_SORT,
} SORT_METHOD;

typedef struct Jarray {    
    /**
    * @brief Prints an error message from an ARRAY_RETURN.
    *
    * If the ARRAY_RETURN contains an error, prints it in red and frees the allocated message string.
    *
    * @param ret The ARRAY_RETURN to inspect.
    */
    void (*print_array_err)(ARRAY_RETURN ret);
    void (*free)(Array *array);
    /**
     * @brief Filters an array using a predicate function.
     *
     * Creates a new array containing only the elements that satisfy the predicate.
     *
     * @param self Pointer to the Array instance.
     * @param predicate Function that returns true for elements to keep.
     * @return ARRAY_RETURN containing a pointer to the new filtered Array, or an error.
     */
    ARRAY_RETURN (*filter)(struct Array *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
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
    ARRAY_RETURN (*at)(struct Array *self, size_t index);
    /**
     * @brief Appends an element to the end of the array.
     *
     * Resizes the array by one element and copies the provided data into the new slot.
     *
     * @param self Pointer to the Array instance.
     * @param elem Pointer to the element to add.
     * @return ARRAY_RETURN containing a pointer to the newly added element, or an error.
     */
    ARRAY_RETURN (*add)(struct Array *self, const void * elem);
    /**
     * @brief Removes the last element from the array.
     *
     * @param self Pointer to the Array instance.
     * @return ARRAY_RETURN containing a pointer to the removed element, or an error.
     */
    ARRAY_RETURN (*remove)(struct Array *self);
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
    ARRAY_RETURN (*remove_at)(struct Array *self, size_t index);
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
    ARRAY_RETURN (*add_at)(struct Array *self, size_t index, const void * elem);
    /**
     * @brief Initializes an array with the given element size.
     *
     * @param array Pointer to the Array instance to initialize.
     * @param elem_size Size of each element in bytes.
     * @return ARRAY_RETURN containing a pointer to the initialized array.
     */
    ARRAY_RETURN (*init)(struct Array *array, size_t elem_size);
    /**
     * @brief Initializes an array with pre-existing data.
     *
     * @param array Pointer to the Array instance to initialize.
     * @param data Pointer to the data buffer.
     * @param length Number of elements in the data buffer.
     * @param elem_size Size of each element in bytes.
     * @return ARRAY_RETURN containing a pointer to the initialized array.
     */
    ARRAY_RETURN (*init_with_data)(struct Array *array, void *data, size_t length, size_t elem_size);
    /**
     * @brief Prints the contents of the array using a user-defined callback.
     *
     * @param array Pointer to the Array instance.
     * @return ARRAY_RETURN containing a pointer to the array, or an error.
     */
    ARRAY_RETURN (*print)(struct Array *self);
    /**
     * @brief Sorts the array and returns a new sorted copy.
     *
     * Uses the specified sorting algorithm and the user-provided compare function.
     *
     * @param self Pointer to the Array instance.
     * @param method Sorting method to use (QSORT, BUBBLE_SORT, INSERTION_SORT, SELECTION_SORT).
     * @return ARRAY_RETURN containing a pointer to the new sorted Array, or an error.
     */
    ARRAY_RETURN (*sort)(struct Array *self, SORT_METHOD method);
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
    ARRAY_RETURN (*find_first)(struct Array *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Gets the data in array self.
     * @param self Pointer to the Array structure.
     * @return ARRAY_RETURN
     *         - On success: `.has_value = true` and `.value` points to the data.
     *         - On failure: `.has_value = false` and `.error` contains error information:
     *              - ARRAY_UNINITIALIZED: The array has not been initialized.
     */
    ARRAY_RETURN (*data)(struct Array *self);
    /**
     * @brief Create a subarray from a given Array.
     * 
     * @details Allocates a new Array containing elements from `low_index` to `high_index` (inclusive) of the original array.
     * Copies the relevant elements into the new Array. The caller is responsible for freeing the subarray's data.
     * 
     * @param self Pointer to the original Array.
     * @param low_index Starting index of the subarray (inclusive).
     * @param high_index Ending index of the subarray (inclusive).
     * @return ARRAY_RETURN On success, contains a pointer to the new subarray. On failure, contains an error code and message.
     */
    ARRAY_RETURN (*subarray)(struct Array *self, size_t low_index, size_t high_index);
    /**
     * @brief Sets the element at the given index in the array.
     * @return ARRAY_RETURN with success or error.
     */
    ARRAY_RETURN (*set)(struct Array *self, size_t index, const void *elem);
    /**
     * @brief Find all indexes in the array whose values match a target value, sorted by distance.
     *
     * This function searches for all elements in the array `self` that are equal to `elem` (by value).
     * It works in several steps:
     *
     * Error cases handled:
     *   - Empty array → returns `EMPTY_ARRAY` error.
     *   - Array not initialized → returns `ARRAY_UNINITIALIZED` error.
     *   - Memory allocation failure → returns `DATA_NULL` error.
     *   - No matches found → returns `ELEMENT_NOT_FOUND` error.
     *
     * @param self Pointer to the Array structure.
     * @param elem Pointer to the target value to find (currently assumes `int` type).
     * @return ARRAY_RETURN containing either:
     *         - `value` → `size_t[]` where first element is match count, followed by match indexes.
     *         - or error code if no match or failure occurs.
     */
    ARRAY_RETURN (*find_indexes)(struct Array *self, const void *elem);
    /**
     * @brief Applies a callback function to each element in the array.
     *
     * Iterates over each element and calls the provided callback with the element and context.
     *
     * @param self Pointer to the Array instance.
     * @param callback Function to call for each element.
     * @param ctx Context pointer passed to the callback.
     * @return ARRAY_RETURN containing success or error information.
     */
    ARRAY_RETURN (*for_each)(struct Array *self, void (*callback)(void *elem, void *ctx), void *ctx);
    /**
     * @brief Clears the array, removing all elements.
     *
     * Resets the array to an empty state without freeing the underlying data buffer.
     *
     * @param self Pointer to the Array instance.
     * @return ARRAY_RETURN containing success or error information.
     */
    ARRAY_RETURN (*clear)(struct Array *self);
    /**
     * @brief Clones the array, creating a new Array with the same elements.
     *
     * Allocates a new Array and copies all elements from the original array.
     *
     * @param self Pointer to the Array instance to clone.
     * @return ARRAY_RETURN containing a pointer to the cloned Array, or an error.
     */
    ARRAY_RETURN (*clone)(struct Array *self);
    /**
     * @brief Adds multiple elements to the array from a data buffer.
     *
     * Resizes the array to accommodate the new elements and copies them into the array.
     *
     * @param self Pointer to the Array instance.
     * @param data Pointer to the data buffer containing elements to add.
     * @param length Number of elements in the data buffer.
     * @return ARRAY_RETURN containing success or error information.
     */
    ARRAY_RETURN (*add_all)(struct Array *self, const void *data, size_t length);
    /**
     * @brief Checks if the array contains a specific element.
     *
     * Uses the user-defined equality function to compare elements.
     *
     * @param self Pointer to the Array instance.
     * @param elem Pointer to the element to check for.
     * @return ARRAY_RETURN containing true if found, false otherwise, or an error.
     */
    ARRAY_RETURN (*contains)(struct Array *self, const void *elem);
    /**
     * @brief Removes all occurrences of elements also contained in the provided data buffer.
     * This function iterates over the array and removes elements that match any in the provided data.
     * @param self Pointer to the Array instance.
     * @param data Pointer to the data buffer containing elements to remove.
     * @return ARRAY_RETURN containing success or error information.
     */
    ARRAY_RETURN (*remove_all)(struct Array *self, const void *data, size_t length);
} Jarray;

extern Jarray jarray;

/* ----- PROTOTYPES ----- */

void print_array_err(ARRAY_RETURN ret);
void array_free(Array *array);
ARRAY_RETURN array_filter(struct Array *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
ARRAY_RETURN array_at(struct Array *self, size_t index);
ARRAY_RETURN array_add(struct Array *self, const void * elem);
ARRAY_RETURN array_remove(struct Array *self);
ARRAY_RETURN array_remove_at(struct Array *self, size_t index);
ARRAY_RETURN array_add_at(struct Array *self, size_t index, const void * elem);
ARRAY_RETURN array_init(Array *array, size_t elem_size);
ARRAY_RETURN array_init_with_data(Array *array, void *data, size_t length, size_t elem_size);
ARRAY_RETURN array_print(struct Array *array);
ARRAY_RETURN array_sort(struct Array *self, SORT_METHOD method);
ARRAY_RETURN array_find_first(struct Array *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
ARRAY_RETURN array_data(struct Array *self);
ARRAY_RETURN array_subarray(struct Array *self, size_t low_index, size_t high_index);
ARRAY_RETURN array_set(struct Array *self, size_t index, const void *elem);
ARRAY_RETURN array_find_indexes(struct Array *self, const void *elem);
ARRAY_RETURN array_for_each(struct Array *self, void (*callback)(void *elem, void *ctx), void *ctx);
ARRAY_RETURN array_clear(struct Array *self);
ARRAY_RETURN array_clone(struct Array *self);
ARRAY_RETURN array_add_all(struct Array *self, const void *data, size_t length);
ARRAY_RETURN array_contains(struct Array *self, const void *elem);
ARRAY_RETURN array_remove_all(struct Array *self, const void *data, size_t length);

/* ----- MACROS WITH AUTO-FREE ----- */

#define GET_VALUE(type, val) (*(type*)val)

// Extract the *value* (by dereferencing) from ARRAY_RETURN, casting it to `type`,
// then free the underlying pointer to avoid leaks.
// Example: int x = RET_GET_VALUE_FREE(int, ret);
#define RET_GET_VALUE_FREE(type, array_return) \
    ({ type _tmp = *(type*)(array_return).value; free((array_return).value); _tmp; })

// Get the *pointer* directly from ARRAY_RETURN, casting it to `type*`,
// then set .value to NULL so we don’t accidentally double-free later.
#define RET_GET_POINTER_OWNED(type, array_return) \
    ({ type* _tmp = (type*)(array_return).value; (array_return).value = NULL; _tmp; })

// Standard GET_VALUE (no free)
#define RET_GET_VALUE(type, array_return) (*(type*)(array_return).value)

// Standard GET_POINTER (no free)
#define RET_GET_POINTER(type, array_return) ((type*)(array_return).value)

// Create temporary variable pointer for literals.
#define TO_POINTER(type, value) (&(type){value})

// Safe GET_VALUE with default
#define RET_GET_VALUE_SAFE(type, array_return, default_value) \
    ((array_return).has_value ? *(type*)((array_return).value) : (default_value))

// Error handling macros
#define CHECK_RET(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret); return; }

#define CHECK_RET_FREE(ret) \
    if ((ret).has_value) free((ret).value); if ((ret).has_error) { jarray.print_array_err(ret);  return; }

#define CHECK_RET_CONTINUE(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret); }

#define CHECK_RET_CONTINUE_FREE(ret) \
    if ((ret).has_value) free((ret).value); if ((ret).has_error) { jarray.print_array_err(ret); }

#define FREE_RET(ret) \
    if ((ret).has_value && (ret).value != NULL) { free((ret).value); (ret).value = NULL; }

#define MAX(a, b) a > b ? a : b

#endif
