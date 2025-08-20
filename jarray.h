#ifndef JARRAY_H
#define JARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/**
 * @file jarray.h
 * @brief Header file for the JARRAY library.
 * This file defines the JARRAY structure and its interface, including error handling and result access macros.
 * It provides a dynamic array implementation with various operations such as adding, removing, filtering, sorting, and accessing elements.
 * A JARRAY can contain any type of data, from primitive types to user-defined structures.
 * The library needs user-defined functions for printing, comparing, and checking equality of elements. These functions can be mandatory for certain operations.
 */

/**
 * @brief JARRAY_ERROR enum.
 * This enum represents various error codes that can occur in the JARRAY library.
 * Each error code corresponds to a specific issue that can arise during operations on the JARRAY.
 */
typedef enum {
    JARRAY_INDEX_OUT_OF_BOUND = 0,
    JARRAY_UNINITIALIZED,
    JARRAY_DATA_NULL,
    JARRAY_PRINT_ELEMENT_CALLBACK_UNINTIALIZED,
    JARRAY_COMPARE_CALLBACK_UNINTIALIZED,
    JARRAY_IS_EQUAL_CALLBACK_UNINTIALIZED,
    JARRAY_EMPTY,
    JARRAY_ELEMENT_NOT_FOUND,
    JARRAY_INVALID_ARGUMENT,
    JARRAY_UNIMPLEMENTED_FUNCTION,
} JARRAY_ERROR;


/**
 * @brief JARRAY_RETURN_ERROR structure.
 * This structure is used to return error information from JARRAY functions.
 * It contains an error code and an error message.
 */
typedef struct JARRAY_RETURN_ERROR {
    JARRAY_ERROR error_code;
    char* error_msg;
} JARRAY_RETURN_ERROR;



/**
 * @brief User-defined function implementations for JARRAY.
 * This structure contains pointers to user-defined functions for printing, comparing, and checking equality of elements.
 * These functions can be set by the user to customize the behavior of the JARRAY. These functions are used by the JARRAY_INTERFACE to perform operations on the elements.
 */
typedef struct JARRAY_USER_FUNCTION_IMPLEMENTATION {
    // Function to print an element. This function is mandatory if you want to use the jarray.print function.
    void (*print_element_callback)(const void*);
    // Function to print error. This function is NOT mandatory but you can override the default one with it if you want to.
    void (*print_error_callback)(const JARRAY_RETURN_ERROR);
    // Function to compare two elements. This function is mandatory if you want to use the jarray.sort function.
    int (*compare)(const void*, const void*);
    // Function to check if two elements are equal. This function is mandatory if you want to use the jarray.contains, jarray.find_first, jarray.find_indexes functions.
    bool (*is_equal)(const void*, const void*);
} JARRAY_USER_FUNCTION_IMPLEMENTATION;

// TYPE_PRESET is not to be used for now
typedef enum TYPE_PRESET {
    CUSTOM = 0,
    BOOL,
    CHAR,
    SIGNED_CHAR,
    UNSIGNED_CHAR,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    LONG,
    UNSIGNED_LONG,
    LONG_LONG,
    UNSIGNED_LONG_LONG,
    FLOAT,
    DOUBLE,
    LONG_DOUBLE
} TYPE_PRESET;

/**
 * @brief JARRAY structure.
 * JARRAY is a the main structure of the library.
 * It represents a dynamic array that can hold elements of a specified size. It supports various operations such as adding, removing, filtering, sorting, and accessing elements.
 * User should use any member of this structure only through the JARRAY_INTERFACE "jarray" functions. The only member to be accessed directly is user_implementation, which contains pointers to user-defined functions for printing, comparing, and checking equality of elements.
 * User can set these functions to customize the behavior of the JARRAY.
 */ 
typedef struct JARRAY {
    void *_data;
    size_t _elem_size;
    size_t _length;
    TYPE_PRESET _type_preset;
    JARRAY_USER_FUNCTION_IMPLEMENTATION user_implementation;
} JARRAY;

/**
 * @brief JARRAY_RETURN structure.
 * This structure is used to return results from JARRAY functions.
 * It can contain either a pointer to the value or an error code and message.
 * The members should not be accessed directly; instead, use the provided macros for safe access.
 */
typedef struct JARRAY_RETURN {
    union {
        void* value;
        JARRAY_RETURN_ERROR error;
    };
    bool has_value;
    bool has_error;
    JARRAY* ret_source;
} JARRAY_RETURN;

typedef enum SORT_METHOD {
    QSORT = 0,
    BUBBLE_SORT,
    INSERTION_SORT,
    SELECTION_SORT,
} SORT_METHOD;

typedef struct JARRAY_INTERFACE {    
    /**
     * @brief Prints an error message for a JARRAY_RETURN.
     * 
     * @note
     * Uses the `print_error_callback` if defined, otherwise prints to stderr.
     * This function does not allocate memory or return values.
     * 
     * @param ret JARRAY_RETURN containing the error to print.
     * @param file Source file where the error occurred.
     * @param line Line number of the error.
     */
    void (*print_array_err)(const JARRAY_RETURN ret, const char *file, int line);
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
     * Caller is responsible for freeing the new JARRAY and its `_data` via `jarray.free` function.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function returning true for elements to keep.
     * @param ctx Context pointer passed to predicate.
     * @return JARRAY_RETURN containing the new filtered JARRAY or an error.
     */
    JARRAY_RETURN (*filter)(struct JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
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
     * @return JARRAY_RETURN containing:
     *   - `.value` pointing to the element (do NOT free),
     *   - or error if out-of-bounds or array not initialized.
     */
    JARRAY_RETURN (*at)(struct JARRAY *self, size_t index);
    /**
     * @brief Appends an element to the end of the array.
     *
     * @note
     * Resizes the internal buffer if size increases beyond the load factor and copies the element into it.
     * The element data is copied; the caller retains ownership of the original.
     *
     * @param self Pointer to JARRAY.
     * @param elem Pointer to element data to append.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*add)(struct JARRAY *self, const void * elem);
    /**
     * @brief Removes the last element from the array.
     *
     * @note
     * Returns a newly allocated copy of the removed element. Caller must free `.value`.
     *
     * @param self Pointer to JARRAY.
     * @return JARRAY_RETURN containing the removed element or an error.
     */
    JARRAY_RETURN (*remove)(struct JARRAY *self);
    /**
     * @brief Removes an element at a specific index.
     *
     * @note
     * Allocates a new buffer for the removed element and returns it.
     * Caller MUST free the returned `.value`.
     *
     * @param self Pointer to JARRAY.
     * @param index Index of element to remove.
     * @return JARRAY_RETURN containing the removed element or an error.
     */
    JARRAY_RETURN (*remove_at)(struct JARRAY *self, size_t index);
    /**
     * @brief Inserts an element at a specific index, shifting elements to the right.
     *
     * @note
     * The element is copied into the array. The caller retains ownership of the original.
     *
     * @param self Pointer to JARRAY.
     * @param index Index to insert at.
     * @param elem Pointer to element to insert.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*add_at)(struct JARRAY *self, size_t index, const void * elem);
    /**
     * @brief Initializes a JARRAY with a given element size.
     *
     * @note
     * Sets initial state and nullifies user callbacks.
     *
     * @param array Pointer to JARRAY.
     * @param elem_size Size of one element in bytes.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*init)(struct JARRAY *array, size_t elem_size);
    /**
     * @brief Prints all elements using the user-defined callback.
     *
     * @note
     * Callback `print_element_callback` must be set, otherwise an error is returned.
     *
     * @param array Pointer to JARRAY.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*print)(struct JARRAY *self);
    /**
     * @brief Sorts a copy of the array using a specified method.
     *
     * @note
     * The internal data buffer is replaced with a newly allocated sorted buffer.
     * Callback `compare` must be set, otherwise an error is returned.
     *
     * @param self Pointer to JARRAY.
     * @param method Sorting method enum.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*sort)(struct JARRAY *self, SORT_METHOD method);
    /**
     * @brief Finds the first element satisfying a predicate.
     *
     * @note
     * Returns a pointer to internal data; do NOT free.
     *
     * @param self Pointer to JARRAY.
     * @param predicate Function to check elements.
     * @param ctx Context pointer for predicate.
     * @return JARRAY_RETURN pointing to matching element or an error.
     */
    JARRAY_RETURN (*find_first)(struct JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Returns a copy of the internal `_data`.
     *
     * @note
     * Allocates memory; caller must free `.value`.
     *
     * @param self Pointer to JARRAY.
     * @return JARRAY_RETURN containing copy of data or error.
     */
    JARRAY_RETURN (*data)(struct JARRAY *self);
    /**
     * @brief Create a subarray from a given JARRAY.
     * 
     * @note Allocates a new JARRAY containing elements from `low_index` to `high_index` (inclusive) of the original array.
     * Copies the relevant elements into the new JARRAY. The caller is responsible for freeing the subarray's data.
     * 
     * @param self Pointer to the original JARRAY.
     * @param low_index Starting index of the subarray (inclusive).
     * @param high_index Ending index of the subarray (inclusive).
     * @return JARRAY_RETURN On success, contains a pointer to the new subarray. On failure, contains an error code and message.
     */
    JARRAY_RETURN (*subarray)(struct JARRAY *self, size_t low_index, size_t high_index);
    /**
     * @brief Sets the element at a given index.
     *
     * @note
     * Copies data into internal buffer. Caller retains ownership of original.
     *
     * @param self Pointer to JARRAY.
     * @param index Index to set.
     * @param elem Pointer to element data.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*set)(struct JARRAY *self, size_t index, const void *elem);
    /**
     * @brief Finds all indexes matching an element using `is_equal`.
     *
     * @note
     * Allocates array of size_t containing count + indexes. Caller must free `.value`.
     *
     * @param self Pointer to JARRAY.
     * @param elem Pointer to element to find.
     * @return JARRAY_RETURN containing allocated indexes or error.
     */
    JARRAY_RETURN (*find_indexes)(struct JARRAY *self, const void *elem);
    /**
     * @brief Applies a callback to each element.
     *
     * @note
     * Callback `callback` must be non-null. Iterates over all elements.
     *
     * @param self Pointer to JARRAY.
     * @param callback Function to apply to each element.
     * @param ctx Context pointer.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*for_each)(struct JARRAY *self, void (*callback)(void *elem, void *ctx), void *ctx);
    /**
     * @brief Clears the array, freeing internal `_data`.
     *
     * @note
     * After clearing, array length is 0. Internal `_data` buffer is freed.
     *
     * @param self Pointer to JARRAY.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*clear)(struct JARRAY *self);
    /**
     * @brief Clones the array.
     *
     * @note
     * Allocates new JARRAY and copies all elements. Caller must free returned JARRAY and `_data`.
     *
     * @param self Pointer to JARRAY.
     * @return JARRAY_RETURN containing new clone or error.
     */
    JARRAY_RETURN (*clone)(struct JARRAY *self);
    /**
     * @brief Adds multiple elements from a data buffer.
     *
     * @note
     * Resizes internal array if needed and copies elements.
     *
     * @param self Pointer to JARRAY.
     * @param data Pointer to data buffer.
     * @param count Number of elements.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*add_all)(struct JARRAY *self, const void *data, size_t length);
    /**
     * @brief Checks if the array contains an element.
     *
     * @note
     * Uses `is_equal` callback. Returns a pointer to a bool (true/false) using `JARRAY_DIRECT_INPUT`.
     *
     * @param self Pointer to JARRAY.
     * @param elem Pointer to element to search.
     * @return JARRAY_RETURN containing result or error.
     */
    JARRAY_RETURN (*contains)(struct JARRAY *self, const void *elem);
    /**
     * @brief Removes all elements present in a data buffer.
     *
     * @note
     * Iterates over array and removes matching elements. Freed memory of removed elements is managed internally.
     *
     * @param self Pointer to JARRAY.
     * @param data Pointer to elements to remove.
     * @param count Number of elements.
     * @return JARRAY_RETURN indicating success or error.
     */
    JARRAY_RETURN (*remove_all)(struct JARRAY *self, const void *data, size_t length);
    /**
     * @brief Returns the number of elements in the array.
     *
     * @param self Pointer to the JARRAY instance.
     * @return JARRAY_RETURN containing the length of the array, or an error.
     */
    JARRAY_RETURN (*length)(struct JARRAY *self);
} JARRAY_INTERFACE;

extern JARRAY_INTERFACE jarray;


/* ----- MACROS WITH AUTO-FREE ----- */

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
 * @brief Extracts the value from a JARRAY_RETURN and frees its internal pointer.
 *
 * @param type Type of the value to extract.
 * @param ret The JARRAY_RETURN structure to extract the value from.
 * @return The extracted value of type `type`.
 *
 * @note Frees the internal .value pointer automatically.
 */
#define JARRAY_RET_GET_VALUE_FREE(type, ret) \
    ({ type _tmp = *(type*)(ret).value; JARRAY_FREE_RET(ret); _tmp; })

/**
 * @brief Extracts the value from a JARRAY_RETURN without freeing it.
 *
 * @param type Type of the value to extract.
 * @param JARRAY_RETURN The JARRAY_RETURN structure to extract the value from.
 * @return The extracted value of type `type`.
 *
 * @note Caller must free .value manually if needed.
 */
#define JARRAY_RET_GET_VALUE(type, JARRAY_RETURN) (*(type*)(JARRAY_RETURN).value)

/**
 * @brief Helper to extract the internal pointer from a JARRAY_RETURN.
 *
 * @param ret JARRAY_RETURN to extract from.
 * @return Pointer stored in ret.value, or NULL if ret.value is NULL.
 *
 * @note Sets ret.value to NULL to prevent double-free.
 */
static inline void* jarray_ret_get_pointer_impl(JARRAY_RETURN ret) {
    if (ret.value == NULL) return NULL;
    void *p = ret.value;
    ret.value = NULL; // Clear the value to avoid double free
    return p;
}

/**
 * @brief Extracts the pointer from a JARRAY_RETURN without freeing.
 *
 * @param type Type of the pointer.
 * @param JARRAY_RETURN JARRAY_RETURN to extract from.
 * @return Pointer of type `type*`.
 *
 * @note Caller is responsible for freeing the returned pointer.
 */
#define JARRAY_RET_GET_POINTER(type, JARRAY_RETURN) ((type*)jarray_ret_get_pointer_impl(JARRAY_RETURN))

/**
 * @brief Extracts value from a JARRAY_RETURN or returns a default if empty.
 *
 * @param type Type of the value.
 * @param JARRAY_RETURN JARRAY_RETURN to extract from.
 * @param default_value Value to return if JARRAY_RETURN has no value.
 * @return Value from JARRAY_RETURN or default_value.
 */
#define JARRAY_RET_GET_VALUE_SAFE(type, JARRAY_RETURN, default_value) \
    ((JARRAY_RETURN).has_value ? *(type*)((JARRAY_RETURN).value) : (default_value))

/**
 * @brief Checks if a JARRAY_RETURN has an error, prints it, and returns EXIT_FAILURE.
 *
 * @param ret JARRAY_RETURN to check.
 */
#define JARRAY_CHECK_RET(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); return EXIT_FAILURE; }

/**
 * @brief Checks if a JARRAY_RETURN has an error, prints it, frees its value if present, and returns EXIT_FAILURE.
 *
 * @param ret JARRAY_RETURN to check.
 */
#define JARRAY_CHECK_RET_FREE(ret) \
    JARRAY_FREE_RET_VALUE(ret);    \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); JARRAY_FREE_RET_ERROR(ret); return EXIT_FAILURE; } \

/**
 * @brief Checks if a JARRAY_RETURN has an error, prints it, and continues execution.
 *
 * @param ret JARRAY_RETURN to check.
 */
#define JARRAY_CHECK_RET_CONTINUE(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); }


/**
 * @brief Checks if a JARRAY_RETURN has an error, prints it, frees its value if present, and continues execution.
 *
 * @param ret JARRAY_RETURN to check.
 */
#define JARRAY_CHECK_RET_CONTINUE_FREE(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); } \
    JARRAY_FREE_RET(ret);

/**
 * @brief Frees only the .value in a JARRAY_RETURN if it exists.
 *
 * @param ret JARRAY_RETURN to free value from.
 */
#define JARRAY_FREE_RET_VALUE(ret) \
    do { \
        if ((ret).has_value && (ret).value != NULL) { \
            free((ret).value); \
            (ret).value = NULL; \
        } \
    } while(0)

/**
 * @brief Frees only the error message in a JARRAY_RETURN if it exists.
 *
 * @param ret JARRAY_RETURN to free error from.
 */
#define JARRAY_FREE_RET_ERROR(ret) \
    do { \
        if ((ret).has_error && (ret).error.error_msg != NULL) { \
            free((ret).error.error_msg); \
            (ret).error.error_msg = NULL; \
        } \
    } while(0)


/**
 * @brief Frees both the .value and the error in a JARRAY_RETURN.
 *
 * @param ret JARRAY_RETURN to fully free.
 */
#define JARRAY_FREE_RET(ret) \
    do { \
        JARRAY_FREE_RET_VALUE(ret); \
        JARRAY_FREE_RET_ERROR(ret); \
    } while(0)

#define MAX(a, b) a > b ? a : b

#endif
