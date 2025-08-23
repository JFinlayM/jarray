# JARRAY - Generic Dynamic Array for C

**Note**: I'm a beginner programmer, so don't take this code too seriously. This is a learning project!

A dynamic array library for C with filtering, sorting, searching, and more. The library is designed to be generic, allowing storage of any data type by specifying the element size during initialization. 
It provides a variety of utility functions and supports user-defined callbacks for custom behavior (a bit). The functions return a struct containing either the result or an error code/message, which can be checked using provided macros. 
The functions are inspired by higher-level languages like JavaScript and Java but hopefully quicker...

## Building

```bash
sudo dpkg -i libjarray-dev_1.0.0-1_amd64.deb
sudo apt-get install -f
gcc main.c -o main -ljarray
```

## Quick Start

```c
#include <jarray.h>

void print_int(const void *x) {
    printf("%d ", JARRAY_GET_VALUE(const int, x));
}

int compare_int(const void *a, const void *b) {
    return JARRAY_GET_VALUE(const int, a) - JARRAY_GET_VALUE(const int, b);
}

int main() {
    JARRAY array;
    JARRAY_RETURN ret;
    
    // Initialize
    ret = jarray.init(&array, sizeof(int));
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE; // Check for errors and return
    // Or just JARRAY_CHECK_RET_FREE(ret) after function calls if you don't need to return
    // Or JARRAY_CHECK_RET(ret) if you need the return value later
    // Or nothing but that creates memory leaks

    // Set callbacks
    array.user_implementation.print_element_callback = print_int;
    array.user_implementation.compare = compare_int;
    
    // Add elements
    for (int i = 1; i <= 5; i++) {
        ret = jarray.add(&array, JARRAY_DIRECT_INPUT(int, i));
        if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    }
    
    jarray.print(&array); // Output: 1 2 3 4 5
    jarray.free(&array);
    return 0;
}
```

## Core Functions

### Basic Operations
```c
jarray.init(&array, sizeof(int));                   // Initialize empty array
jarray.init_with_data(&array, data, count, sizeof(int)); // Initialize with raw data
jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42));   // Append element
jarray.add_all(&array, data, count);                // Append multiple elements
jarray.add_at(&array, index, &value);               // Insert at index
jarray.at(&array, index);                           // Get element at index
jarray.set(&array, index, &value);                  // Overwrite element
jarray.length(&array);                              // Get array length
jarray.remove(&array);                              // Remove last element (and returns copy)
jarray.remove_at(&array, index);                    // Remove at index (and returns copy)
jarray.remove_all(&array, data, count);             // Remove all occurrences of given data
jarray.print(&array);                               // Print array (needs print_element_callback)
jarray.data(&array);                                // Copy raw buffer
jarray.clear(&array);                               // Clear contents
jarray.clone(&array);                               // Deep copy
jarray.concat(&array, &other);                      // Concatenate two arrays
jarray.free(&array);                                // Free memory

```

### Advanced Operations
```c
jarray.sort(&array, QSORT, compare);                // Sort (either compare callback in arg or in user_implementation)
jarray.filter(&array, predicate, ctx);              // Filter by condition (predicate callback and optional context)
jarray.subarray(&array, start, end);                // Extract subarray
jarray.find_first(&array, predicate, ctx);          // First match by predicate
jarray.find_indexes(&array, &value);                // All indexes of a value
jarray.contains(&array, &value);                    // True/false if value exists
jarray.for_each(&array, callback, ctx);             // Apply function to each element
jarray.reduce(&array, reducer, &initial, ctx);      // Reduce to single value
jarray.join(&array, separator);                     // Join as string (requires element_to_string callback)
```

## Memory Management Rules

| Function                            | Must Free Result?                  | How to Free?           |
| ----------------------------------- | ---------------------------------  | ---------------------- |
| `at()`, `find_first()`              | ❌ No (points inside array)        | —                      |
| `filter()`, `clone()`, `subarray()` | ✅ Yes                             | `jarray.free(&result)` |
| `data()`, `find_indexes()`          | ✅ Yes                             | `free(result)`         |
| `remove()`, `remove_at()`           | ✅ Yes (copy returned)             | `free(result)`         |
| `reduce()`                          | ✅ Yes (if it allocates new value) | `free(result)`         |
| `join()`                            | ✅ Yes                             | `free(result)`         |
| `contains()`, `length()`, `clear()` | ❌ No                              | —                      |


## Examples

### Filtering and Searching
```c
bool is_even(const void *x, const void *ctx) {
    (void)ctx; // Unused here
    return JARRAY_GET_VALUE(const int, x) % 2 == 0;
}

// Filter
ret = jarray.filter(&array, is_even, NULL); // No context needed here -> NULL
JARRAY *evens = JARRAY_RET_GET_POINTER(JARRAY, ret);
jarray.print(evens);
jarray.free(evens);

// Find first
ret = jarray.find_first(&array, is_even, NULL); // No context needed here -> NULL
if (!ret.has_error) {
    printf("First even: %d\n", JARRAY_RET_GET_VALUE(int, ret));
}
```

### Custom Types
```c
typedef struct { int x, y; } Point;

void print_point(const void *p) {
    Point pt = JARRAY_GET_VALUE(const Point, p);
    printf("(%d,%d) ", pt.x, pt.y);
}

JARRAY points;
jarray.init(&points, sizeof(Point));
points.user_implementation.print_element_callback = print_point;

Point p = {3, 4};
jarray.add(&points, &p);
```

### Sorting
```c
// Sort methods: QSORT, BUBBLE_SORT, INSERTION_SORT, SELECTION_SORT
array.user_implementation.compare = compare_func; // Set comparison function
jarray.sort(&array, QSORT, NULL);
```
Or
```c
jarray.sort(&array, QSORT, compare_func);
```

## Required Callbacks

Set these before using related functions:
```c
array.user_implementation.print_element_callback = print_func;  // For print()
array.user_implementation.element_to_string = to_string_func; // For join()
array.user_implementation.compare = compare_func;              // For sort()
array.user_implementation.is_equal = equal_func;               // For contains(), find_indexes()
```

## MACROS

Use these macros for automatic error checking and value extraction:
```c
JARRAY_CHECK_RET(ret);           // Print error, free error, return true if error -> if you ret value later
JARRAY_CHECK_RET_FREE(ret);      // Print error, free return value and error, return true if error -> if you don't ret value later
JARRAY_RET_GET_POINTER(type, ret); // Get pointer from return value (after checking no error)
JARRAY_RET_GET_VALUE(type, ret);   // Get value from return value (after checking no error)
```