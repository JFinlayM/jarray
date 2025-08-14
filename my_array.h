#ifndef MY_ARRAY_H
#define MY_ARRAY_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    INDEX_OUT_OF_BOUND = 0,
    ARRAY_UNINITIALIZED,
    DATA_NULL,
    PRINT_ELEMENT_CALLBACK_UNINTIALIZED,
    EMPTY_ARRAY,
    ELEMENT_NOT_FOUND,
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
} ARRAY_RETURN;


typedef struct USER_FUNCTION_IMPLEMENTATION {
    void (*print_element_callback)(const void*);
    int (*compare)(const void*, const void*);
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
     * @brief Filters an array using a predicate function.
     *
     * Creates a new array containing only the elements that satisfy the predicate.
     *
     * @param self Pointer to the Array instance.
     * @param predicate Function that returns true for elements to keep.
     * @return ARRAY_RETURN containing a pointer to the new filtered Array, or an error.
     */
    ARRAY_RETURN (*filter)(struct Array *self, bool (*predicate)(const void *));
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
    ARRAY_RETURN (*find_by_predicate)(struct Array *self, bool (*predicate)(const void *));
    /**
     * @brief Prints an error message from an ARRAY_RETURN.
     *
     * If the ARRAY_RETURN contains an error, prints it in red and frees the allocated message string.
     *
     * @param ret The ARRAY_RETURN to inspect.
     */
    void (*print_array_err)(ARRAY_RETURN ret);
} Jarray;

extern Jarray jarray;

ARRAY_RETURN array_filter(struct Array *self, bool (*predicate)(const void *));
ARRAY_RETURN array_at(struct Array *self, size_t index);
ARRAY_RETURN array_add(struct Array *self, const void * elem);
ARRAY_RETURN array_remove(struct Array *self);
ARRAY_RETURN array_remove_at(struct Array *self, size_t index);
ARRAY_RETURN array_add_at(struct Array *self, size_t index, const void * elem);
ARRAY_RETURN array_init(Array *array, size_t elem_size);
ARRAY_RETURN array_init_with_data(Array *array, void *data, size_t length, size_t elem_size);
ARRAY_RETURN array_print(struct Array *array);
ARRAY_RETURN array_sort(struct Array *self, SORT_METHOD method);
ARRAY_RETURN array_find_by_predicate(struct Array *self, bool (*predicate)(const void *));
void print_array_err(ARRAY_RETURN ret);

// Extract the *value* (by dereferencing) from ARRAY_RETURN, casting it to `type`.
// Example: int x = GET_VALUE(int, ret);
#define GET_VALUE(type, array_return) (*(type*)(array_return).value)

// Get the *pointer* directly from ARRAY_RETURN, casting it to `type*`.
// Example: int* ptr = GET_POINTER(int, ret);
#define GET_POINTER(type, array_return) (type*)(array_return.value)

// Create a temporary variable of `type` with the given `value`
// and return its pointer (useful for passing literals to functions expecting void*).
// Example: jarray.add(&arr, TO_POINTER(int, 42));
#define TO_POINTER(type, value) (&(type){value})

// Like GET_VALUE but safe: if `.has_value` is false, return `default_value` instead.
// Example: int x = GET_VALUE_SAFE(int, ret, -1);
#define GET_VALUE_SAFE(type, array_return, default_value) \
    ((array_return).has_value == true ? *(type*)((array_return).value) : (default_value))

// Check if a function returning ARRAY_RETURN was successful.
// If not, print the error and return from the current function.
#define CHECK_RET(ret)      if (!ret.has_value) {                       \
                                jarray.print_array_err(ret);            \
                                return;                                 \
                            }

// Same as CHECK_RET but doesn't return — just prints the error and continues execution.
#define CHECK_RET_CONTINUE(ret) if (!ret.has_value) { \
                                jarray.print_array_err(ret);            \
                            }

// Same as CHECK_RET_CONTINUE but also frees ret.value if it exists.
#define CHECK_RET_CONTINUE_FREE(ret) if (!ret.has_value) { \
                                jarray.print_array_err(ret);            \
                                free(ret.value);                        \
                            }

#endif
