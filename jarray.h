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
} JARRAY_ERROR;

/**
 * @brief JARRAY_STATE_T enum.
 * This enum represents the state of the JARRAY.
 * This enum is not usefull for now, but it is here for future use.
 * It indicates whether the JARRAY has been initialized or not.
 */
typedef enum {
    UNINITIALIZED = 0,
    INITIALIZED
} JARRAY_STATE_T;

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
typedef struct USER_FUNCTION_IMPLEMENTATION {
    // Function to print an element. This function is mandatory if you want to use the jarray.print function.
    void (*print_element_callback)(const void*);
    // Function to print error. This function is NOT mandatory but you can override the default one with it if you want to.
    void (*print_error_callback)(const JARRAY_RETURN_ERROR);
    // Function to compare two elements. This function is mandatory if you want to use the jarray.sort function.
    int (*compare)(const void*, const void*);
    // Function to check if two elements are equal. This function is mandatory if you want to use the jarray.contains, jarray.find_first, jarray.find_indexes functions.
    bool (*is_equal)(const void*, const void*);
} USER_FUNCTION_IMPLEMENTATION;

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
    JARRAY_STATE_T _state;
    size_t _length;
    TYPE_PRESET _type_preset;
    USER_FUNCTION_IMPLEMENTATION user_implementation;
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
    * @brief Prints an error message from an JARRAY_RETURN.
    *
    * If the JARRAY_RETURN contains an error, prints it in red and frees the allocated message string.
    *
    * @param ret The JARRAY_RETURN to inspect.
    */
    void (*print_array_err)(const JARRAY_RETURN ret, const char *file, int line);
    void (*free)(JARRAY *array);
    /**
     * @brief Filters an array using a predicate function.
     *
     * Creates a new array containing only the elements that satisfy the predicate.
     *
     * @param self Pointer to the JARRAY instance.
     * @param predicate Function that returns true for elements to keep.
     * @return JARRAY_RETURN containing a pointer to the new filtered JARRAY, or an error.
     */
    JARRAY_RETURN (*filter)(struct JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Retrieves an element at the specified index.
     *
     * Returns a pointer to the element inside the array's data buffer.
     * The pointer is valid as long as the array is not reallocated or freed.
     *
     * @param self Pointer to the JARRAY instance.
     * @param index Index of the element to retrieve.
     * @return JARRAY_RETURN containing a pointer to the element, or an error if out of bounds.
     */
    JARRAY_RETURN (*at)(struct JARRAY *self, size_t index);
    /**
     * @brief Appends an element to the end of the array.
     *
     * Resizes the array by one element and copies the provided data into the new slot.
     *
     * @param self Pointer to the JARRAY instance.
     * @param elem Pointer to the element to add.
     * @return JARRAY_RETURN containing a pointer to the newly added element, or an error.
     */
    JARRAY_RETURN (*add)(struct JARRAY *self, const void * elem);
    /**
     * @brief Removes the last element from the array.
     *
     * @param self Pointer to the JARRAY instance.
     * @return JARRAY_RETURN containing a pointer to the removed element, or an error.
     */
    JARRAY_RETURN (*remove)(struct JARRAY *self);
    /**
     * @brief Removes an element at a specific index from the array.
     *
     * Shifts remaining elements to fill the gap. The removed element is returned
     * in newly allocated memory, which must be freed by the caller.
     *
     * @param self Pointer to the JARRAY instance.
     * @param index Index of the element to remove.
     * @return JARRAY_RETURN containing a pointer to the removed element, or an error.
     */
    JARRAY_RETURN (*remove_at)(struct JARRAY *self, size_t index);
    /**
     * @brief Inserts an element at a specific index in the array.
     *
     * Shifts elements to the right to make space for the new element.
     *
     * @param self Pointer to the JARRAY instance.
     * @param index Index where the element should be inserted.
     * @param elem Pointer to the element to insert.
     * @return JARRAY_RETURN containing a pointer to the inserted element, or an error.
     */
    JARRAY_RETURN (*add_at)(struct JARRAY *self, size_t index, const void * elem);
    /**
     * @brief Initializes an array with the given element size.
     *
     * @param array Pointer to the JARRAY instance to initialize.
     * @param elem_size Size of each element in bytes.
     * @return JARRAY_RETURN containing a pointer to the initialized array.
     */
    JARRAY_RETURN (*init)(struct JARRAY *array, size_t elem_size);
    /**
     * @brief Initializes an array with pre-existing data.
     *
     * @param array Pointer to the JARRAY instance to initialize.
     * @param data Pointer to the data buffer.
     * @param length Number of elements in the data buffer.
     * @param elem_size Size of each element in bytes.
     * @return JARRAY_RETURN containing a pointer to the initialized array.
     */
    JARRAY_RETURN (*init_with_data)(struct JARRAY *array, void *data, size_t length, size_t elem_size);
    /**
     * @brief Prints the contents of the array using a user-defined callback.
     *
     * @param array Pointer to the JARRAY instance.
     * @return JARRAY_RETURN containing a pointer to the array, or an error.
     */
    JARRAY_RETURN (*print)(struct JARRAY *self);
    /**
     * @brief Sorts the array and returns a new sorted copy.
     *
     * Uses the specified sorting algorithm and the user-provided compare function.
     *
     * @param self Pointer to the JARRAY instance.
     * @param method Sorting method to use (QSORT, BUBBLE_SORT, INSERTION_SORT, SELECTION_SORT).
     * @return JARRAY_RETURN containing a pointer to the new sorted JARRAY, or an error.
     */
    JARRAY_RETURN (*sort)(struct JARRAY *self, SORT_METHOD method);
    /**
     * @brief Finds the first element in the array that satisfies a given predicate.
     *
     * This function iterates over each element of the array and applies the provided
     * predicate function. If the predicate returns true for an element, the search stops,
     * and that element is returned. If no element satisfies the predicate, an error is returned.
     *
     * @param self       Pointer to the JARRAY structure.
     * @param predicate  Function pointer to a predicate function that takes a `const void*` 
     *                   (pointer to the element) and returns a boolean indicating whether
     *                   the element matches the desired condition.
     *
     * @return JARRAY_RETURN
     *         - On success: `.has_value = true` and `.value` points to the matching element.
     *         - On failure: `.has_value = false` and `.error` contains error information:
     *              - ARRAY_UNINITIALIZED: The array has not been initialized.
     *              - ELEMENT_NOT_FOUND: No element satisfies the predicate.
     */
    JARRAY_RETURN (*find_first)(struct JARRAY *self, bool (*predicate)(const void *elem, const void *ctx), const void *ctx);
    /**
     * @brief Gets the data in array self.
     * @param self Pointer to the JARRAY structure.
     * @return JARRAY_RETURN
     *         - On success: `.has_value = true` and `.value` points to the data.
     *         - On failure: `.has_value = false` and `.error` contains error information:
     *              - ARRAY_UNINITIALIZED: The array has not been initialized.
     */
    JARRAY_RETURN (*data)(struct JARRAY *self);
    /**
     * @brief Create a subarray from a given JARRAY.
     * 
     * @details Allocates a new JARRAY containing elements from `low_index` to `high_index` (inclusive) of the original array.
     * Copies the relevant elements into the new JARRAY. The caller is responsible for freeing the subarray's data.
     * 
     * @param self Pointer to the original JARRAY.
     * @param low_index Starting index of the subarray (inclusive).
     * @param high_index Ending index of the subarray (inclusive).
     * @return JARRAY_RETURN On success, contains a pointer to the new subarray. On failure, contains an error code and message.
     */
    JARRAY_RETURN (*subarray)(struct JARRAY *self, size_t low_index, size_t high_index);
    /**
     * @brief Sets the element at the given index in the array.
     * @return JARRAY_RETURN with success or error.
     */
    JARRAY_RETURN (*set)(struct JARRAY *self, size_t index, const void *elem);
    /**
     * @brief Find all indexes in the array whose values match a target value, sorted by distance.
     *
     * This function searches for all elements in the array `self` that are equal to `elem` (by value).
     * It works in several steps:
     *
     * Error cases handled:
     *   - Empty array → returns `EMPTY_ARRAY` error.
     *   - JARRAY not initialized → returns `ARRAY_UNINITIALIZED` error.
     *   - Memory allocation failure → returns `DATA_NULL` error.
     *   - No matches found → returns `ELEMENT_NOT_FOUND` error.
     *
     * @param self Pointer to the JARRAY structure.
     * @param elem Pointer to the target value to find (currently assumes `int` type).
     * @return JARRAY_RETURN containing either:
     *         - `value` → `size_t[]` where first element is match count, followed by match indexes.
     *         - or error code if no match or failure occurs.
     */
    JARRAY_RETURN (*find_indexes)(struct JARRAY *self, const void *elem);
    /**
     * @brief Applies a callback function to each element in the array.
     *
     * Iterates over each element and calls the provided callback with the element and context.
     *
     * @param self Pointer to the JARRAY instance.
     * @param callback Function to call for each element.
     * @param ctx Context pointer passed to the callback.
     * @return JARRAY_RETURN containing success or error information.
     */
    JARRAY_RETURN (*for_each)(struct JARRAY *self, void (*callback)(void *elem, void *ctx), void *ctx);
    /**
     * @brief Clears the array, removing all elements.
     *
     * Resets the array to an empty state without freeing the underlying data buffer.
     *
     * @param self Pointer to the JARRAY instance.
     * @return JARRAY_RETURN containing success or error information.
     */
    JARRAY_RETURN (*clear)(struct JARRAY *self);
    /**
     * @brief Clones the array, creating a new JARRAY with the same elements.
     *
     * Allocates a new JARRAY and copies all elements from the original array.
     *
     * @param self Pointer to the JARRAY instance to clone.
     * @return JARRAY_RETURN containing a pointer to the cloned JARRAY, or an error.
     */
    JARRAY_RETURN (*clone)(struct JARRAY *self);
    /**
     * @brief Adds multiple elements to the array from a data buffer.
     *
     * Resizes the array to accommodate the new elements and copies them into the array.
     *
     * @param self Pointer to the JARRAY instance.
     * @param data Pointer to the data buffer containing elements to add.
     * @param length Number of elements in the data buffer.
     * @return JARRAY_RETURN containing success or error information.
     */
    JARRAY_RETURN (*add_all)(struct JARRAY *self, const void *data, size_t length);
    /**
     * @brief Checks if the array contains a specific element.
     *
     * Uses the user-defined equality function to compare elements.
     *
     * @param self Pointer to the JARRAY instance.
     * @param elem Pointer to the element to check for.
     * @return JARRAY_RETURN containing true if found, false otherwise, or an error.
     */
    JARRAY_RETURN (*contains)(struct JARRAY *self, const void *elem);
    /**
     * @brief Removes all occurrences of elements also contained in the provided data buffer.
     * This function iterates over the array and removes elements that match any in the provided data.
     * @param self Pointer to the JARRAY instance.
     * @param data Pointer to the data buffer containing elements to remove.
     * @return JARRAY_RETURN containing success or error information.
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


#define GET_VALUE(type, val) (*(type*)val)

static inline void* direct_input_impl(size_t size, void *value) {
    void *p = malloc(size);
    if (p) memcpy(p, value, size);
    return p;
}

/**
 * @brief Creates a pointer to a value of type `type` from a value.
 * @param type The type of the value to create a pointer to.
 * @param val The value to create a pointer from.
 * @return A pointer to the value of type `type`.
 */
#define DIRECT_INPUT(type, val) ((type*) direct_input_impl(sizeof(type), &(type){val}))


/**
 * @brief Extracts the value from a JARRAY_RETURN, and frees the data pointed by .value if not NULL.
 * @param type The type of the value to extract.
 * @param ret The JARRAY_RETURN structure to extract the value from.
 * @return The extracted value of type `type`.
 */
#define RET_GET_VALUE_FREE(type, ret) \
    ({ type _tmp = *(type*)(ret).value; FREE_RET(ret); _tmp; })

/**
 * @brief Extracts the value from a JARRAY_RETURN without freeing it. It is the caller's responsibility to free the .value if needed.
 * @param type The type of the value to extract.
 * @param JARRAY_RETURN The JARRAY_RETURN structure to extract the value from.
 * @return The extracted value of type `type`.
 */
#define RET_GET_VALUE(type, JARRAY_RETURN) (*(type*)(JARRAY_RETURN).value)

static inline void* ret_get_pointer_impl(JARRAY_RETURN ret) {
    if (ret.value == NULL) return NULL;
    void *p = ret.value;
    ret.value = NULL; // Clear the value to avoid double free
    return p;
}

/**
 * @brief Extracts the pointer from a JARRAY_RETURN without freeing it. It is the caller's responsibility to free the pointer if needed.
 * @param type The type of the pointer to extract.
 * @param JARRAY_RETURN The JARRAY_RETURN structure to extract the pointer from.
 * @return The extracted pointer of type `type*`.
 */
#define RET_GET_POINTER(type, JARRAY_RETURN) ((type*)ret_get_pointer_impl(JARRAY_RETURN))

/**
 * @bried Extract the value from a JARRAY_RETURN, and returns a default value if the JARRAY_RETURN has no value.
 * @param type The type of the value to extract.
 * @param JARRAY_RETURN The JARRAY_RETURN structure to extract the value from.
 * @param default_value The default value to return if the JARRAY_RETURN has no value.
 * @return The extracted value of type `type`, or the default value if the JARRAY_RETURN has no value.
 */
#define RET_GET_VALUE_SAFE(type, JARRAY_RETURN, default_value) \
    ((JARRAY_RETURN).has_value ? *(type*)((JARRAY_RETURN).value) : (default_value))

/**
 * @brief Checks if a JARRAY_RETURN has an error and prints it if so. Then returns.
 * @param ret The JARRAY_RETURN structure to check.
 */
#define CHECK_RET(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); return EXIT_FAILURE; }

/**
 * @brief Checks if a JARRAY_RETURN has an error and prints it if so, freeing the .value if it exists. Then returns.
 * @param ret The JARRAY_RETURN structure to check.
 */
#define CHECK_RET_FREE(ret) \
    FREE_RET_VALUE(ret);    \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); FREE_RET_ERROR(ret); return EXIT_FAILURE; } \

/**
 * @brief Checks if a JARRAY_RETURN has an error and prints it if so, freeing the .value if it exists.
 * @param ret The JARRAY_RETURN structure to check.
 * @return true if the JARRAY_RETURN has an error, false otherwise.
 */
#define CHECK_RET_CONTINUE(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); }

/**
 * @brief Checks if a JARRAY_RETURN has an error and prints it if so, freeing the .value if it exists.
 * @param ret The JARRAY_RETURN structure to check.
 * @return true if the JARRAY_RETURN has an error, false otherwise.
 */
#define CHECK_RET_CONTINUE_FREE(ret) \
    if ((ret).has_error) { jarray.print_array_err(ret, __FILE__, __LINE__); } \
    FREE_RET(ret);

/**
 * @brief Frees only the value in a JARRAY_RETURN if it has a value.
 */
#define FREE_RET_VALUE(ret) \
    do { \
        if ((ret).has_value && (ret).value != NULL) { \
            free((ret).value); \
            (ret).value = NULL; \
        } \
    } while(0)

/**
 * @brief Frees only the error message in a JARRAY_RETURN if it exists.
 */
#define FREE_RET_ERROR(ret) \
    do { \
        if ((ret).has_error && (ret).error.error_msg != NULL) { \
            free((ret).error.error_msg); \
            (ret).error.error_msg = NULL; \
        } \
    } while(0)

/**
 * @brief Frees both the value and error message in a JARRAY_RETURN.
 */
#define FREE_RET(ret) \
    do { \
        FREE_RET_VALUE(ret); \
        FREE_RET_ERROR(ret); \
    } while(0)

#define MAX(a, b) a > b ? a : b

#endif
