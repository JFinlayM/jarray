/**
 * @file jarray.h
 * @brief Header file for the JARRAY library.
 * This file defines the JARRAY structure and its interface, including error handling and result access macros.
 * It provides a dynamic array implementation with various operations such as adding, removing, filtering, sorting, and accessing elements.
 * A JARRAY can contain any type of data, from primitive types to user-defined structures.
 * The library needs user-defined functions for printing, comparing, and checking equality of elements. These functions can be mandatory for certain operations.
 */

#ifndef JARRAY_H
#define JARRAY_H
 
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ERR_MSG_LENGTH 100

typedef struct JARRAY JARRAY;

/**
 * @brief JARRAY_ERROR enum.
 * This enum represents various error codes that can occur in the JARRAY library.
 * Each error code corresponds to a specific issue that can arise during operations on the JARRAY.
 */
typedef enum {
    JARRAY_NO_ERROR = 0,
    JARRAY_INDEX_OUT_OF_BOUND,
    JARRAY_UNINITIALIZED,
    JARRAY_DATA_NULL,
    JARRAY_PRINT_ELEMENT_CALLBACK_UNINTIALIZED,
    JARRAY_ELEMENT_TO_STRING_CALLBACK_UNINTIALIZED,
    JARRAY_COMPARE_CALLBACK_UNINTIALIZED,
    JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED,
    JARRAY_EMPTY,
    JARRAY_ELEMENT_NOT_FOUND,
    JARRAY_INVALID_ARGUMENT,
    JARRAY_UNIMPLEMENTED_FUNCTION,
} JARRAY_ERROR;

typedef enum {
    STRING_PRESET = 0,
    INT_PRESET,
    FLOAT_PRESET,
    CHAR_PRESET,
    DOUBLE_PRESET,
    LONG_PRESET,
    SHORT_PRESET,
    UINT_PRESET,
    ULONG_PRESET,
    USHORT_PRESET,
} TYPE_PRESET;

/**
 * @brief JARRAY_RETURN structure.
 * This structure is used to return results from JARRAY functions.
 * It can contain either a pointer to the value or an error code and message.
 * The members should not be accessed directly; instead, use the provided macros for safe access.
 */
typedef struct JARRAY_RETURN {
    JARRAY_ERROR error_code;
    char error_msg[MAX_ERR_MSG_LENGTH];
    bool has_error;
    const JARRAY* ret_source;
} JARRAY_RETURN;

/**
 * @brief User-defined function implementations for JARRAY.
 * This structure contains pointers to user-defined functions for printing, comparing, and checking equality of elements.
 * These functions can be set by the user to customize the behavior of the JARRAY. These functions are used by the JARRAY_INTERFACE to perform operations on the elements.
 */
typedef struct JARRAY_USER_CALLBACK_IMPLEMENTATION {
    // Function to print an element. This function is mandatory if you want to use the jarray.print function.
    void (*print_element_callback)(const void*);
    // Function to convert an element to a string. This function is NOT mandatory but can be useful for functions like join.
    char *(*element_to_string)(const void*);
    // Function to compare two elements. This function is mandatory if you want to use the jarray.sort function.
    int (*compare)(const void*, const void*);
    // Function to check if two elements are equal. This function is mandatory if you want to use the jarray.contains, jarray.find_first, jarray.indexes_of functions.
    bool (*is_equal)(const void*, const void*);
} JARRAY_USER_CALLBACK_IMPLEMENTATION;

typedef struct JARRAY_USER_OVERRIDE_IMPLEMENTATION {
    // Override function to print errors. This function is NOT mandatory.
    void (*print_error_override)(const JARRAY_RETURN);
    // Override function to print the whole array. This function is NOT mandatory.
    void (*print_array_override)(const JARRAY*);
    // This function is MANDATORY if storing pointers (Example : strdup for char*).
    void *(*copy_elem_override)(const void*);
}JARRAY_USER_OVERRIDE_IMPLEMENTATION;


/**
 * @brief JARRAY structure.
 * JARRAY is a the main structure of the library.
 * It represents a dynamic array that can hold elements of a specified size. It supports various operations such as adding, removing, filtering, sorting, and accessing elements.
 * User should use any member of this structure only through the JARRAY_INTERFACE "jarray" functions. The only member to be accessed directly is user_implementation, which contains pointers to user-defined functions for printing, comparing, and checking equality of elements.
 * User can set these functions to customize the behavior of the JARRAY.
 */ 
typedef struct JARRAY {
    void * _data;
    size_t _elem_size;
    size_t _length;
    size_t _min_alloc;
    JARRAY_USER_CALLBACK_IMPLEMENTATION user_callbacks;
    JARRAY_USER_OVERRIDE_IMPLEMENTATION user_overrides;
} JARRAY;



typedef enum SORT_METHOD {
    QSORT = 0,
    BUBBLE_SORT,
    INSERTION_SORT,
    SELECTION_SORT,
} SORT_METHOD;

typedef struct JARRAY_INTERFACE {
    /**
     * @brief Prints the error message of the last jarray call.
     * 
     * @note
     * Uses the `print_error_override` if defined, otherwise uses a default printing method.
     * 
     * @param file Source file where the error occurred.
     * @param line Line number of the error.
     */
    void (*print_array_err)(const char *file, int line);
    /**
     * @brief Frees a JARRAY instance and its internal data buffer.
     *
     * @note 
     * Clears all allocated memory in the JARRAY and resets its internal state.
     * Does not free the JARRAY pointer itself (caller must free if dynamically allocated).
     *
     * @param array Pointer to the JARRAY to free.
     */
    void (*free)(JARRAY *array);
    /**
     * @brief Filters elements based on a predicate.
     *
     * @note
     * Allocates a new JARRAY for the filtered elements.
     * Caller is responsible for freeing the new JARRAY and its data via `jarray.free` function.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function returning true for elements to keep.
     * @param ctx (Optionnal) Context pointer passed to predicate.
     * @return filtered jarray.
     */
    JARRAY (*filter)(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Retrieves a pointer to the element at a given index.
     *
     * @note
     * The pointer points directly inside the array's internal buffer.
     * The caller must NOT free this pointer. If the array is reallocated or freed,
     * the pointer becomes invalid.
     *
     * @param self Pointer to JARRAY.
     * @param index Index of the element.
     * @return pointer to elem at `index`.
     */
    void* (*at)(const JARRAY *self, size_t index);
    /**
     * @brief Appends an element to the end of the array.
     *
     * @note
     * The element data is copied; the caller retains ownership of the original.
     *
     * @param self Pointer to JARRAY.
     * @param elem Pointer to element data to append.
     */
    void (*add)(JARRAY *self, const void * elem);
    /**
     * @brief Removes the last element from the array.
     *
     * @note
     * Returns a newly allocated copy of the removed element. Caller must free `.value`.
     *
     * @param self Pointer to JARRAY.
     */
    void (*remove)(JARRAY *self);
    /**
     * @brief Removes an element at a specific index.
     *
     * @note
     * Allocates a new buffer for the removed element and returns it.
     * Caller MUST free the returned `.value`.
     *
     * @param self Pointer to JARRAY.
     * @param index Index of element to remove.
     */
    void (*remove_at)(JARRAY *self, size_t index);
    /**
     * @brief Inserts an element at a specific index, shifting elements to the right.
     *
     * @note
     * The element is copied into the array. The caller retains ownership of the original.
     *
     * @param self Pointer to JARRAY.
     * @param index Index to insert at.
     * @param elem Pointer to element to insert.
     */
    void (*add_at)(JARRAY *self, size_t index, const void * elem);
    /**
     * @brief Initializes a JARRAY with a given element size.
     *
     * @note
     * Sets initial state and nullifies user callbacks.
     *
     * @param array Pointer to JARRAY.
     * @param elem_size Size of one element in bytes.
     * @param user_callbacks Structure containing the implementation of callbacks functions.
     */
    void (*init)(JARRAY *array, size_t elem_size, JARRAY_USER_CALLBACK_IMPLEMENTATION user_callbacks);
    /**
     * @brief Initializes a JARRAY with pre-existing data.
     *
     * @note
     * Copies the provided data into the JARRAY. The caller retains ownership of the original data.
     *
     * @param array Pointer to JARRAY.
     * @param data Pointer to existing data buffer.
     * @param length Number of elements in the data buffer.
     * @param elem_size Size of one element in bytes.
     * @param user_callbacks Structure containing the implementation of callbacks functions.
     */
    void (*init_with_data_copy)(JARRAY *array, const void *data, size_t length, size_t elem_size, JARRAY_USER_CALLBACK_IMPLEMENTATION user_callbacks);
    /**
     * @brief Initializes a JARRAY with pre-existing (if heap allocated!! Otherwise use `init_with_data_copy`) data.
     *
     * @note
     * Takes ownership of the provided data into the JARRAY. The caller should set data pointer to NULL.
     *
     * @param array Pointer to JARRAY.
     * @param data Pointer to existing data buffer.
     * @param length Number of elements in the data buffer.
     * @param elem_size Size of one element in bytes.
     * @param user_callbacks Structure containing the implementation of callbacks functions.
     */
    void (*init_with_data)(JARRAY *array, void *data, size_t length, size_t elem_size, JARRAY_USER_CALLBACK_IMPLEMENTATION user_callbacks);
    JARRAY (*init_preset)(TYPE_PRESET preset);
    /**
     * @brief Prints all elements using the user-defined callback.
     *
     * @note
     * Callback `print_element_callback` must be set, otherwise an error is returned.
     *
     * @param array Pointer to JARRAY.
     */
    void (*print)(const JARRAY *self);
    /**
     * @brief Sorts a copy of the array using a specified method.
     *
     * @note
     * The internal data buffer is replaced with a newly allocated sorted buffer.
     * Callback `compare` must be set, otherwise an error is returned.
     *
     * @param self Pointer to JARRAY.
     * @param method Sorting method enum.
     * @param custom_compare (Optional) Custom compare function (overrides user callback if provided).
     */
    void (*sort)(JARRAY *self, SORT_METHOD method, int (*custom_compare)(const void*, const void*));
    /**
     * @brief Finds the first element satisfying a predicate.
     *
     * @note
     * Returns a pointer to internal data; do NOT free.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function to check elements.
     * @param ctx (Optionnal) Context pointer for predicate.
     * @return pointer to element.
     */
    void* (*find_first)(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Returns a copy of the internal `_data`.
     *
     * @note
     * Allocates memory; caller must free `.value`.
     *
     * @param self Pointer to JARRAY.
     * @return pointer to the first element of data array.
     */
    void* (*copy_data)(JARRAY *self);
    /**
     * @brief Create a subarray from a given JARRAY.
     * 
     * @note Allocates a new JARRAY containing elements from `low_index` to `high_index` (inclusive) of the original array.
     * Copies the relevant elements into the new JARRAY. The caller is responsible for freeing the subarray's data.
     * 
     * @param self Pointer to the original JARRAY.
     * @param low_index Starting index of the subarray (inclusive).
     * @param high_index Ending index of the subarray (inclusive).
     * @return sub jarray.
     */
    JARRAY (*subarray)(JARRAY *self, size_t low_index, size_t high_index);
    /**
     * @brief Sets the element at a given index.
     *
     * @note
     * Copies data into internal buffer. Caller retains ownership of original.
     *
     * @param self Pointer to JARRAY.
     * @param index Index to set.
     * @param elem Pointer to element data.
     */
    void (*set)(JARRAY *self, size_t index, const void *elem);
    /**
     * @brief Finds all indexes matching an element using `is_equal`.
     *
     * @note
     * Allocates array of size_t containing count + indexes. Caller has responsabilité to free result.
     *
     * @param self Pointer to JARRAY.
     * @param elem Pointer to element to find.
     * @return pointer to array of the indexes.
     */
    size_t* (*indexes_of)(JARRAY *self, const void *elem);
    /**
     * @brief Applies a callback to each element.
     *
     * @note
     * Callback `callback` must be non-null. Iterates over all elements.
     *
     * @param self Pointer to JARRAY.
     * @param callback Function to apply to each element.
     * @param ctx (Optionnal) Context pointer.
     */
    void (*for_each)(JARRAY *self, void (*callback)(void *elem, void *ctx), void *ctx);
    /**
     * @brief Clears the array, freeing internal `_data`.
     *
     * @note
     * After clearing, array length is 0. Internal `_data` buffer is freed.
     *
     * @param self Pointer to JARRAY.
     */
    void (*clear)(JARRAY *self);
    /**
     * @brief Clones the array.
     *
     * @note
     * Allocates new JARRAY and copies all elements. Caller must free returned JARRAY and `_data`.
     *
     * @param self Pointer to JARRAY.
     * @return cloned jarray
     */
    JARRAY (*clone)(JARRAY *self);
    /**
     * @brief Adds multiple elements from a data buffer.
     *
     * @note
     * Resizes internal array if needed and copies elements.
     *
     * @param self Pointer to JARRAY.
     * @param data Pointer to data buffer.
     * @param count Number of elements.
     */
    void (*add_all)(JARRAY *self, const void *data, size_t length);
    /**
     * @brief Checks if the array contains an element.
     *
     * @note
     * Uses `is_equal` callback. Returns a pointer to a bool (true/false) using `JARRAY_DIRECT_INPUT`.
     *
     * @param self Pointer to JARRAY.
     * @param elem Pointer to element to search.
     * @return boolean : true if element is in jarray, false otherwise
     */
    bool (*contains)(JARRAY *self, const void *elem);
    /**
     * @brief Removes all elements present in a data buffer.
     *
     * @note
     * Iterates over array and removes matching elements. Freed memory of removed elements is managed internally.
     *
     * @param self Pointer to JARRAY.
     * @param data Pointer to elements to remove.
     * @param count Number of elements.
     */
    void (*remove_all)(JARRAY *self, const void *data, size_t length);
    /**
     * @brief Returns the number of elements in the array. 
     * @note Or just access the _length member directly if you want to.
     * @param self Pointer to the JARRAY instance.
     * @return length of the jarray
     */
    size_t (*length)(JARRAY *self);
    /**
     * @brief Reduces the array to a single value using a reducer function.
     *
     * @note
     * Allocates memory for the reduced value; caller has responsability to free result. 
     *
     * @param self Pointer to JARRAY.
     * @param reducer Function to combine elements.
     * @param initial_value (Optionnal) Pointer to initial accumulator value.
     * @param ctx (Optionnal) Context pointer for reducer.
     * @return pointer to result.
     */
    void* (*reduce)(JARRAY *self, void* (*reducer)(const void* accumulator, const void* elem, const void* ctx), const void* initial_value, const void* ctx);
    /**
     * @brief Concatenates two JARRAYs of the same element type.
     * 
     * @note Allocates a new JARRAY containing elements from both input arrays. The caller is responsible for freeing the concatenated array's data.
     * 
     * @param arr1 Pointer to the first JARRAY.
     * @param arr2 Pointer to the second JARRAY.
     * @return jarray concatanated.
     */
    JARRAY (*concat)(JARRAY *arr1, JARRAY *arr2);
    /**
     * @brief Joins the string representations of all elements into a single string, separated by a specified delimiter.
     * 
     * @note Uses the `element_to_string` callback to convert each element to a string. Allocates memory for the resulting string; caller must free `.value`.
     * 
     * @param self Pointer to the JARRAY instance.
     * @param separator String to insert between elements.
     * @return string result.
     */
    char* (*join)(JARRAY *self, const char *separator);
    /**
     * @brief Reverses the order of elements in the array.
     * 
     * @note This function modifies the array in place and does not allocate new memory.
     * 
     * @param self Pointer to the JARRAY instance.
     */
    void (*reverse)(JARRAY *self);
    /**
     * @brief Checks if any element satisfies a predicate.
     *
     * @note
     * Returns a pointer to a bool via JARRAY_RETURN
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function to check elements.
     * @param ctx (Optionnal) Context pointer for predicate.
     * @return boolean : true if any satisfies the predicate, false otherwise.
     */
    bool (*any)(const JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Reduces the array to a single value using a reducer function. The reducer function is applied from rigth to left of array.
     *
     * @note
     * Allocates memory for the reduced value; caller has responsability to free result.
     *
     * @param self Pointer to JARRAY.
     * @param reducer Function to combine elements.
     * @param initial_value (Optionnal) Pointer to initial accumulator value.
     * @param ctx (Optionnal) Context pointer for reducer.
     * @return pointer to result.
     */
    void* (*reduce_right)(JARRAY *self, void* (*reducer)(const void* accumulator, const void* elem, const void* ctx), const void* initial_value, const void* ctx);
    /**
     * @brief Finds the last element satisfying a predicate.
     *
     * @note
     * Returns a pointer to internal data; do NOT free.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function to check elements.
     * @param ctx (Optionnal) Context pointer for predicate.
     * @return pointer to last element.
     */
    void* (*find_last)(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Finds the index of the first element satisfying a predicate.
     *
     * @note
     * Returns a pointer to internal data; do NOT free.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function to check elements.
     * @param ctx (Optionnal) Context pointer for predicate.
     * @return index of first element.
     */
    size_t (*find_first_index)(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Finds the index of the last element satisfying a predicate.
     *
     * @note
     * Returns a pointer to internal data; do NOT free.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function to check elements.
     * @param ctx (Optionnal) Context pointer for predicate.
     * @return index of last element.
     */
    size_t (*find_last_index)(JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Fills array with the same element. The number of element in the array filled is specified with argument `count`.
     * 
     * @note
     * If `count` is superior than the arrays length then the data is reallocated. If lower, the data not within `count` are not replaced.
     * @param self Pointer to JARRAY.
     * @param elem element to insert.
     * @param start the index where inserting begins.
     * @param end the index where inserting ends.
     */
    void (*fill)(JARRAY *self, const void *elem, size_t start, size_t end);
    /**
     * @brief Shifts the array to the left and discards the first element.
     * 
     * @param self Pointer to JARRAY.
     */
    void (*shift)(JARRAY *self);
    /**
     * @brief Shifts the array to the right and adds elem at index 0.
     * 
     * @param self Pointer to JARRAY.
     * @param elem Element to add.
     */
    void (*shift_right)(JARRAY *self, const void *elem);
    /**
     * @brief Adds and/or removes array elements.
     * 
     * @note If you do not add any elements, you must have NULL as last argument. 
     * Example: jarray.splice(array, 2, 1, NULL);
     * 
     * @param self Pointer to JARRAY.
     * @param index position to add and/or remove items.
     * @param count number of element to remove.
     * @param ... the new elements to be added. ALWAYS add `NULL` as last argument after the elements.
     */
    void (*splice)(JARRAY *self, size_t index, size_t count, ...);
    /**
     * @brief Appends elements to the end of the array.
     *
     * @note
     * The elements data are copied; the caller retains ownership of the original elements.
     *
     * @param self Pointer to JARRAY.
     * @param ... elements to append. ALWAYS add `NULL` as last argument after the elements to append.
     */
    void (*addm)(JARRAY *self, ...);
    /**
     * @brief Reserves `capacity * self->_elem_size` bytes for the array, and sets `self->_min_alloc` to `capacity`.
     * 
     * @note
     * `self->_min_alloc` only changes via this function, and will be spreaded when cloning array for example.
     * 
     * @param self Pointer o JARRAY.
     * @param capacity Number of element to reserve in memory.
     */
    void (*reserve)(JARRAY *self, size_t capacity);
    /**
     * @brief Initialize array and reserve memory.
     * 
     * @param self pointer to array
     * @param elem_size size in bytes of the elements to be contained in array.
     * @param capacity number of element to reserve in memory.
     */
    void (*init_reserve)(JARRAY *self, size_t elem_size, size_t capacity, JARRAY_USER_CALLBACK_IMPLEMENTATION imp);
} JARRAY_INTERFACE;

extern JARRAY_INTERFACE jarray;
extern JARRAY_RETURN last_error_trace;

/* ----- MACROS ----- */



/**
 * @brief Extracts the value from a pointer returned by JARRAY.
 *
 * @param type The type of the value stored in the pointer.
 * @param val Pointer to the value.
 * @return The value pointed by val, cast to type `type`.
 *
 * @note Does NOT free the pointer.
 */
#define JARRAY_GET_VALUE(type, val) (*(type*)val)

#define JARRAY_GET_POINTER(type, val) ((type*)val)

/**
 * @brief Allocates memory and copies a value into it.
 *
 * @param size Size of the value in bytes.
 * @param value Pointer to the value to copy.
 * @return Pointer to the newly allocated copy, or NULL if allocation fails.
 *
 * @note Caller is responsible for freeing the returned pointer.
 */
static inline void* jarray_direct_input_impl(size_t size, void *value) {
    void *p = malloc(size);
    if (p) memcpy(p, value, size);
    return p;
}


/**
 * @brief Creates a pointer to a temporary value of a given type.
 *
 * @param type Type of the value.
 * @param val Value to copy into allocated memory.
 * @return Pointer to the allocated copy of the value.
 *
 * @note Caller must free the returned pointer if needed.
 */
#define JARRAY_DIRECT_INPUT(type, val) ((type*) jarray_direct_input_impl(sizeof(type), &(type){val}))


/**
 * @brief Checks if global error trace contains error.
 *
 * @return true if error, false otherwise.
 */
#define JARRAY_CHECK_RET \
    ({ \
        bool ret_val = false; \
        if (last_error_trace.has_error) { \
            jarray.print_array_err(__FILE__, __LINE__); \
            ret_val = true; \
        } \
        ret_val; \
    })

#define JARRAY_CKECK_RET_RETURN \
    if (last_error_trace.has_error) { \
        jarray.print_array_err(__FILE__, __LINE__); \
        return EXIT_FAILURE; \
    }


#endif
