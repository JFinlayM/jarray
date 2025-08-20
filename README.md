# JARRAY - A Generic Dynamic Array Library for C

JARRAY is a flexible, type-safe dynamic array implementation for C that provides a rich set of operations including filtering, sorting, searching, and more. It uses a function pointer interface pattern and macros to achieve type safety while maintaining the flexibility of void pointers.

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Core Concepts](#core-concepts)
- [API Reference](#api-reference)
- [Memory Management](#memory-management)
- [Examples](#examples)
- [Error Handling](#error-handling)
- [Building](#building)

## Features

- **Generic**: Works with any data type
- **Type-safe**: Macros provide compile-time type checking
- **Rich API**: 20+ operations including filter, sort, find, clone, etc.
- **Multiple sorting algorithms**: QuickSort, Bubble Sort, Insertion Sort, Selection Sort
- **Flexible callbacks**: User-defined print, compare, and equality functions
- **Comprehensive error handling**: Detailed error messages with file/line information
- **Memory efficient**: Automatic memory management with clear ownership rules

## Quick Start

```c
#include "jarray.h"
#include <stdio.h>

// Helper functions
void print_int(const void *x) {
    printf("%d ", JARRAY_GET_VALUE(const int, x));
}

int compare_int(const void *a, const void *b) {
    return JARRAY_GET_VALUE(const int, a) - JARRAY_GET_VALUE(const int, b);
}

int main() {
    JARRAY array;
    JARRAY_RETURN ret;
    
    // Initialize array for integers
    ret = jarray.init(&array, sizeof(int));
    JARRAY_CHECK_RET_FREE(ret);
    
    // Set up callbacks
    array.user_implementation.print_element_callback = print_int;
    array.user_implementation.compare = compare_int;
    
    // Add some elements
    for (int i = 1; i <= 5; i++) {
        ret = jarray.add(&array, JARRAY_DIRECT_INPUT(int, i));
        JARRAY_CHECK_RET_FREE(ret);
    }
    
    // Print array
    jarray.print(&array); // Output: JARRAY [size: 5] => 1 2 3 4 5
    
    // Clean up
    jarray.free(&array);
    return 0;
}
```

## Core Concepts

### JARRAY Structure

```c
typedef struct JARRAY {
    void *_data;                    // Internal data buffer
    size_t _length;                 // Number of elements
    size_t _elem_size;             // Size of each element
    JARRAY_USER_IMPLEMENTATION user_implementation; // User callbacks
} JARRAY;
```

### Return Value System

All functions return `JARRAY_RETURN` which contains:
- `bool has_value`: Whether the operation returned a value
- `bool has_error`: Whether an error occurred
- `void *value`: The returned value (if any)
- `JARRAY_RETURN_ERROR error`: Error information

### User Callbacks

```c
typedef struct JARRAY_USER_IMPLEMENTATION {
    void (*print_element_callback)(const void *elem);
    void (*print_error_callback)(const JARRAY_RETURN_ERROR error);
    int (*compare)(const void *a, const void *b);
    bool (*is_equal)(const void *a, const void *b);
} JARRAY_USER_IMPLEMENTATION;
```

## API Reference

### Initialization and Cleanup

#### `jarray.init(JARRAY *array, size_t elem_size)`
Initializes an empty array for elements of the specified size.

**Parameters:**
- `array`: Pointer to JARRAY to initialize
- `elem_size`: Size of each element in bytes

**Returns:** `JARRAY_RETURN` with success/error status

**Example:**
```c
JARRAY array;
JARRAY_RETURN ret = jarray.init(&array, sizeof(int));
JARRAY_CHECK_RET_FREE(ret);
```

#### `jarray.free(JARRAY *array)`
Frees all memory associated with the array and resets its state.

**Parameters:**
- `array`: Pointer to JARRAY to free

**Memory:** Always call this to avoid memory leaks.

### Adding Elements

#### `jarray.add(JARRAY *array, const void *elem)`
Appends an element to the end of the array.

**Parameters:**
- `array`: Pointer to JARRAY
- `elem`: Pointer to element to add

**Example:**
```c
int value = 42;
ret = jarray.add(&array, &value);
// Or using macro:
ret = jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42));
```

#### `jarray.add_at(JARRAY *array, size_t index, const void *elem)`
Inserts an element at the specified index, shifting existing elements.

**Parameters:**
- `array`: Pointer to JARRAY
- `index`: Index where to insert (0 to length)
- `elem`: Pointer to element to insert

#### `jarray.add_all(JARRAY *array, const void *data, size_t count)`
Adds multiple elements from a data buffer.

**Parameters:**
- `array`: Pointer to JARRAY
- `data`: Pointer to data buffer
- `count`: Number of elements to add

### Accessing Elements

#### `jarray.at(JARRAY *array, size_t index)`
Returns a pointer to the element at the specified index.

**Parameters:**
- `array`: Pointer to JARRAY
- `index`: Index of element to access

**Returns:** `JARRAY_RETURN` with pointer to element in `.value`

**Memory:** Pointer is valid until array is modified or freed.

**Example:**
```c
ret = jarray.at(&array, 3);
JARRAY_CHECK_RET(ret);
int value = JARRAY_RET_GET_VALUE(int, ret);
```

#### `jarray.data(JARRAY *array)`
Returns a copy of the entire data buffer.

**Returns:** `JARRAY_RETURN` with pointer to copied data

**Memory:** ⚠️ **Caller must free** the returned pointer.

### Searching

#### `jarray.find_first(JARRAY *array, bool (*predicate)(const void *elem, const void *ctx), const void *ctx)`
Finds the first element matching a predicate function.

**Parameters:**
- `array`: Pointer to JARRAY
- `predicate`: Function that returns true for matching elements
- `ctx`: Context pointer passed to predicate

**Returns:** `JARRAY_RETURN` with pointer to matching element

**Example:**
```c
bool is_even(const void *x, const void *ctx) {
    return JARRAY_GET_VALUE(const int, x) % 2 == 0;
}

ret = jarray.find_first(&array, is_even, NULL);
if (!ret.has_error) {
    printf("First even: %d\n", JARRAY_RET_GET_VALUE(int, ret));
}
```

#### `jarray.find_indexes(JARRAY *array, const void *elem)`
Finds all indexes where the element appears.

**Returns:** `JARRAY_RETURN` with `size_t*` array where `[0]` is count, `[1...]` are indexes

**Memory:** ⚠️ **Caller must free** the returned pointer.

**Requires:** `is_equal` callback must be set.

#### `jarray.contains(JARRAY *array, const void *elem)`
Checks if the array contains a specific element.

**Returns:** `JARRAY_RETURN` with boolean value

**Example:**
```c
ret = jarray.contains(&array, JARRAY_DIRECT_INPUT(int, 42));
bool found = JARRAY_RET_GET_VALUE_FREE(bool, ret);
```

### Modification

#### `jarray.set(JARRAY *array, size_t index, const void *elem)`
Sets the element at the specified index.

#### `jarray.remove_at(JARRAY *array, size_t index)`
Removes the element at the specified index.

**Returns:** `JARRAY_RETURN` with pointer to removed element

**Memory:** ⚠️ **Caller must free** the returned element.

#### `jarray.remove(JARRAY *array)`
Removes the last element.

**Returns:** `JARRAY_RETURN` with pointer to removed element

**Memory:** ⚠️ **Caller must free** the returned element.

#### `jarray.remove_all(JARRAY *array, const void *data, size_t count)`
Removes all elements that match any element in the provided data buffer.

### Filtering and Transformation

#### `jarray.filter(JARRAY *array, bool (*predicate)(const void *elem, const void *ctx), const void *ctx)`
Creates a new array containing only elements that match the predicate.

**Returns:** `JARRAY_RETURN` with pointer to new filtered JARRAY

**Memory:** ⚠️ **Caller must free** the returned JARRAY using `jarray.free()`.

**Example:**
```c
ret = jarray.filter(&array, is_even, NULL);
JARRAY_CHECK_RET(ret);
JARRAY *evens = JARRAY_RET_GET_POINTER(JARRAY, ret);
jarray.print(evens);
jarray.free(evens); // Don't forget to free!
```

#### `jarray.for_each(JARRAY *array, void (*callback)(void *elem, void *ctx), void *ctx)`
Applies a function to each element in the array.

**Example:**
```c
void multiply_by_2(void *x, void *ctx) {
    JARRAY_GET_VALUE(int, x) *= 2;
}

jarray.for_each(&array, multiply_by_2, NULL);
```

### Sorting

#### `jarray.sort(JARRAY *array, SORT_METHOD method)`
Sorts the array in-place using the specified algorithm.

**Sort Methods:**
- `QSORT`: Standard library quicksort (fastest)
- `BUBBLE_SORT`: Bubble sort algorithm
- `INSERTION_SORT`: Insertion sort algorithm  
- `SELECTION_SORT`: Selection sort algorithm

**Requires:** `compare` callback must be set.

**Example:**
```c
ret = jarray.sort(&array, QSORT);
JARRAY_CHECK_RET_FREE(ret);
```

### Array Operations

#### `jarray.clone(JARRAY *array)`
Creates a deep copy of the array.

**Returns:** `JARRAY_RETURN` with pointer to new cloned JARRAY

**Memory:** ⚠️ **Caller must free** the returned JARRAY.

#### `jarray.subarray(JARRAY *array, size_t low_index, size_t high_index)`
Creates a new array containing elements from low_index to high_index (inclusive).

**Returns:** `JARRAY_RETURN` with pointer to new subarray JARRAY

**Memory:** ⚠️ **Caller must free** the returned JARRAY.

#### `jarray.clear(JARRAY *array)`
Removes all elements from the array, but keeps it initialized.

#### `jarray.length(JARRAY *array)`
Returns the number of elements in the array.

**Returns:** `JARRAY_RETURN` with `size_t` value

### Utility

#### `jarray.print(JARRAY *array)`
Prints the array contents using the print callback.

**Requires:** `print_element_callback` must be set.

## Memory Management

### Ownership Rules

| Function | Returns | Caller Must Free? |
|----------|---------|------------------|
| `jarray.at()` | Element pointer | ❌ No (pointer into array) |
| `jarray.data()` | Data copy | ✅ Yes |
| `jarray.find_first()` | Element pointer | ❌ No (pointer into array) |
| `jarray.find_indexes()` | Index array | ✅ Yes |
| `jarray.remove()` | Removed element | ✅ Yes |
| `jarray.remove_at()` | Removed element | ✅ Yes |
| `jarray.filter()` | New JARRAY | ✅ Yes (with `jarray.free()`) |
| `jarray.clone()` | New JARRAY | ✅ Yes (with `jarray.free()`) |
| `jarray.subarray()` | New JARRAY | ✅ Yes (with `jarray.free()`) |

### Memory Safety Tips

1. **Always call `jarray.free()`** on the main array when done
2. **Free returned arrays** from `filter()`, `clone()`, `subarray()`
3. **Free returned data** from `data()`, `find_indexes()`, `remove()`, `remove_at()`
4. **Don't free pointers** from `at()`, `find_first()` - they point into the array
5. **Use error checking macros** to ensure cleanup on errors

## Examples

### Complete Example with Error Handling

```c
#include "jarray.h"
#include <stdio.h>
#include <stdlib.h>

// Helper functions
void print_int(const void *x) {
    printf("%d ", JARRAY_GET_VALUE(const int, x));
}

int compare_int(const void *a, const void *b) {
    return JARRAY_GET_VALUE(const int, a) - JARRAY_GET_VALUE(const int, b);
}

bool is_equal_int(const void *a, const void *b) {
    return JARRAY_GET_VALUE(const int, a) == JARRAY_GET_VALUE(const int, b);
}

bool is_even(const void *x, const void *ctx) {
    return JARRAY_GET_VALUE(const int, x) % 2 == 0;
}

int main() {
    JARRAY array;
    JARRAY_RETURN ret;
    
    // Initialize
    ret = jarray.init(&array, sizeof(int));
    JARRAY_CHECK_RET_FREE(ret);
    
    // Set callbacks
    array.user_implementation.print_element_callback = print_int;
    array.user_implementation.compare = compare_int;
    array.user_implementation.is_equal = is_equal_int;
    
    // Add numbers 1-10
    for (int i = 1; i <= 10; i++) {
        ret = jarray.add(&array, JARRAY_DIRECT_INPUT(int, i));
        JARRAY_CHECK_RET_FREE(ret);
    }
    
    printf("Original array: ");
    jarray.print(&array);
    
    // Filter even numbers
    ret = jarray.filter(&array, is_even, NULL);
    JARRAY_CHECK_RET(ret);
    JARRAY *evens = JARRAY_RET_GET_POINTER(JARRAY, ret);
    
    printf("Even numbers: ");
    jarray.print(evens);
    
    // Sort original array
    ret = jarray.sort(&array, QSORT);
    JARRAY_CHECK_RET_FREE(ret);
    
    printf("Sorted array: ");
    jarray.print(&array);
    
    // Find element
    ret = jarray.find_first(&array, is_even, NULL);
    if (!ret.has_error) {
        printf("First even number: %d\n", JARRAY_RET_GET_VALUE(int, ret));
    }
    
    // Cleanup
    jarray.free(evens);
    jarray.free(&array);
    
    return 0;
}
```

### Working with Custom Types

```c
typedef struct Point {
    int x, y;
} Point;

void print_point(const void *p) {
    Point point = JARRAY_GET_VALUE(const Point, p);
    printf("(%d,%d) ", point.x, point.y);
}

int compare_points(const void *a, const void *b) {
    Point pa = JARRAY_GET_VALUE(const Point, a);
    Point pb = JARRAY_GET_VALUE(const Point, b);
    // Compare by distance from origin
    int dist_a = pa.x * pa.x + pa.y * pa.y;
    int dist_b = pb.x * pb.x + pb.y * pb.y;
    return dist_a - dist_b;
}

int main() {
    JARRAY points;
    jarray.init(&points, sizeof(Point));
    points.user_implementation.print_element_callback = print_point;
    points.user_implementation.compare = compare_points;
    
    // Add points
    Point p1 = {3, 4};
    Point p2 = {1, 1};
    jarray.add(&points, &p1);
    jarray.add(&points, &p2);
    
    jarray.print(&points); // Output: (3,4) (1,1)
    
    // Sort by distance from origin
    jarray.sort(&points, QSORT);
    jarray.print(&points); // Output: (1,1) (3,4)
    
    jarray.free(&points);
    return 0;
}
```

## Error Handling

### Error Checking Macros

- `JARRAY_CHECK_RET(ret)`: Check return value, print error and continue on failure
- `JARRAY_CHECK_RET_FREE(ret)`: Check return value, print error and free array on failure  
- `JARRAY_CHECK_RET_CONTINUE_FREE(ret)`: Check return value, continue on error after freeing

### Custom Error Callback

```c
void print_error_callback(const JARRAY_RETURN_ERROR error) {
    fprintf(stderr, "JARRAY Error: %s\n", error.error_msg);
    free(error.error_msg);
}

// Set custom error handler
array.user_implementation.print_error_callback = print_error_callback;
```

### Error Types

- `JARRAY_INDEX_OUT_OF_BOUND`: Index exceeds array bounds
- `JARRAY_UNINITIALIZED`: Array not properly initialized
- `JARRAY_DATA_NULL`: Memory allocation failed
- `JARRAY_EMPTY`: Operation on empty array not allowed
- `JARRAY_ELEMENT_NOT_FOUND`: Search operation found no matches
- `JARRAY_INVALID_ARGUMENT`: Invalid parameters passed
- And more...

## Building

Include `jarray.h` and link with `jarray.c`:

```bash
gcc -o myprogram main.c jarray.c
```

The library uses standard C library functions: `malloc`, `realloc`, `free`, `memcpy`, `memmove`, `qsort`, `printf`, `fprintf`.

## Best Practices

1. **Always initialize** arrays before use
2. **Set required callbacks** before calling functions that need them
3. **Use error checking macros** for automatic error handling
4. **Free returned arrays and data** to prevent memory leaks
5. **Don't modify array during iteration** - undefined behavior
6. **Use JARRAY_DIRECT_INPUT macro** for cleaner element insertion
7. **Check documentation** for memory ownership of each function

---

JARRAY provides a powerful, flexible foundation for dynamic arrays in C while maintaining type safety and comprehensive error handling. The library is designed to be intuitive for C programmers while providing modern conveniences through its macro system.