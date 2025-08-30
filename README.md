# JARRAY - Generic Dynamic Array for C on Linux

**Note**: I'm a beginner programmer, so don't take this code too seriously. This is a learning project!

A dynamic array library for C on **Linux** with filtering, sorting, searching, and more. The library is designed to be generic, allowing storage of any data type by specifying the element size during initialization. 
It provides a variety of utility functions and supports user-defined callbacks for custom behavior (a bit). The functions return a struct containing either the result or an error code/message, which can be checked using provided macros. 
The functions are inspired by higher-level languages like JavaScript and Java...

**Note**: You can store either pointers or values, but if storing pointers you have to implement a certain function `copy_elem_override`. Please lool below for more info. You can also find the file `jarray_string.c` where I implemented the functions need to have a jarray of string (char*) as a preset. You have an example of the use of this present in `jarray_string.c` in folder `Examples`.

`main.c` is complete example of a jarray storing integers. But you can store any structure in a jarray.

## Install

Via Git download:
```bash
mkdir build && cd build
cmake ..
sudo make lib
sudo ldconfig
```

Via dpkg:

```bash
sudo dpkg -i libjarray-dev_1.0.0-1_amd64.deb
sudo apt-get install -f
sudo ldconfig
```
This should install libjarray.so and libjarray.a in /urs/local/lib/ and jarray.h in usr/local/include/

To use in your projet just include with *#include <jarray.h>* and link with *-ljarray* when compiling.
You can find in folder `Examples` some simple c files using jarray. To see result (for jarray_string.c for example): 
```bash
cd Examples
make
./jarray_string hello goodbye
./jarray_points
```

## Quick Start

```c
#include <jarray.h>
#include <stdio.h>

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
    // Or nothing but that creates memory leaks if there is returned value or an error

*
    // Set callbacks
    array.user_implementation.print_element_callback = print_int;
    array.user_implementation.compare = compare_int;
    
    // Add elements
    for (int i = 1; i <= 5; i++) {
        ret = jarray.add(&array, JARRAY_DIRECT_INPUT(int, i));
        if (JARRAY_CHECK_RET(ret)) return EXIT_FAILURE;
    }
    
    ret = jarray.print(&array);
    JARRAY_CHECK_RET(ret);
    ret = jarray.free(&array);
    JARRAY_CHECK_RET(ret);
    return 0;
}
```
### Output

```
JARRAY [size: 5] =>
1 2 3 4 5 
```



## Core Functions

### Basic Operations
```c
jarray.init(&array, sizeof(int));                       // Initialize empty array
jarray.init_reserve(&array, sizeof(int), capacity)      // Initialize array and reserve memory
jarray.init_with_data(&array, data, count, sizeof(int));// Initialize with raw data (don't use if data is stack allocated)
jarray.init_with_data_copy(&array, data, count, sizeof(int));// Initialize with a copy of raw data 
jarray.init_preset(preset);                             // Returns a jarray with all the user functions implemented for a given preset
jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42));       // Append element
jarray.addm(&array, ...);                               // Appends elements to the end of the array.
jarray.add_all(&array, data, count);                    // Append multiple elements
jarray.add_at(&array, index, &value);                   // Insert at index
jarray.at(&array, index);                               // Get element at index
jarray.set(&array, index, &value);                      // Overwrite element
jarray.remove(&array);                                  // Remove last element (and returns copy)
jarray.remove_at(&array, index);                        // Remove at index (and returns copy)
jarray.print(&array);                                   // Print array (needs print_element_callback)
jarray.copy_data(&array);                               // Copy raw buffer
jarray.clear(&array);                                   // Clear contents
jarray.clone(&array);                                   // Deep copy
jarray.concat(&array, &other);                          // Concatenate two arrays
jarray.reserve(&array, capacity)                        // Reserves `capacity * array->_elem_size` bytes for the array, and sets `array->_min_alloc` to `capacity`
jarray.free(&array);                                    // Free memory
```

### Advanced Operations
```c
jarray.remove_all(&array, data, count);                 // Remove all occurrences of given data
jarray.sort(&array, QSORT, compare);                    // Sort (either compare callback in arg or in user_implementation)
jarray.filter(&array, predicate, ctx);                  // Filter by condition (predicate callback and optional context)
jarray.subarray(&array, start, end);                    // Extract subarray
jarray.find_first(&array, predicate, ctx);              // First match by predicate
jarray.find_first_index(&array, predicate, ctx);        // Index of first match by predicate
jarray.find_last(&array, predicate, ctx);               // Last match by predicate
jarray.find_last_index(&array, predicate, ctx);         // Index of last match by predicate
jarray.find_indexes(&array, &value);                    // All indexes of a value
jarray.contains(&array, &value);                        // True/false if value exists
jarray.reverse(&array);                                 // Reverse array
jarray.any(&array, predicate, ctx);                     // True/false if any element matches predicate
jarray.for_each(&array, callback, ctx);                 // Apply function to each element
jarray.reduce(&array, reducer, &initial, ctx);          // Reduce to single value
jarray.reduce_right(&array, reducer, &initial, ctx);    // Reduce from the right to single value
jarray.join(&array, separator);                         // Join as string (requires element_to_string callback)
jarray.fill(&array, elem, start, end);                  // Fills of elem the jarray from start to end index
jarray.shift(&array);                                   // Shifts the array to the left and discards the first element.
jarray.shift_right(&array, elem);                       // Shifts the array to the right and adds elem at index 0.
jarray.splice(&array, index, count, ...);               // Adds and/or removes array elements.
```

## Examples

There is an example for every function in file `main.c`. To see result:
```bash
make test
./jarray_test
```
But you can also find a few examples below. **Note**: you cannot just copy/paste the code below, it's just function call examples.

### Filtering and Searching
```c
bool is_even(const void *x, const void *ctx) {
    (void)ctx;  // Unused here. But you can use ctx to personnalize this function
                // without having to declare global variables
    return JARRAY_GET_VALUE(const int, x) % 2 == 0;
}

// Filter
ret = jarray.filter(&array, is_even, NULL); // No context needed here -> NULL
if (JARRAY_CHECK_RET(ret)) return EXIT_FAILURE;
JARRAY *evens = JARRAY_RET_GET_POINTER(JARRAY, ret);
ret = jarray.print(evens);
JARRAY_CHECK_RET(ret);
ret = jarray.free(evens);
JARRAY_CHECK_RET(ret);

// Find first
ret = jarray.find_first(&array, is_even, NULL); // No context needed here -> NULL
if (JARRAY_CHECK_RET(ret)) return EXIT_FAILURE;
printf("First even: %d\n", JARRAY_RET_GET_VALUE(int, ret));
```

### Custom Types
```c
typedef struct { int x, y; } Point;

void print_point(const void *p) {
    Point pt = JARRAY_GET_VALUE(const Point, p);
    printf("(%d,%d) ", pt.x, pt.y);
}

JARRAY points;
JARRAY_RETURN ret;
ret = jarray.init(&points, sizeof(Point));
if (JARRAY_CHECK_RET(ret)) return EXIT_FAILURE;
points.user_implementation.print_element_callback = print_point;

Point p = {3, 4};
ret = jarray.add(&points, &p);
JARRAY_CHECK_RET(ret);

ret = jarray.print(&points);
JARRAY_CHECK_RET(ret);
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
array.user_implementation.element_to_string = to_string_func;   // For join()
array.user_implementation.compare = compare_func;               // For sort()
array.user_implementation.is_equal = equal_func;                // For contains(), find_indexes()
```

## Override callbacks

There is some functions that can be overriden. Maybe more will be added later:
```c
array.user_implementation.print_error_override = error_func;        // For error printing
array.user_implementation.print_array_override = print_array_func;  // For print() override
array.user_implementation.copy_elem_override = copy_elem_func;      // For copy override. MANDATORY when storing pointers (Example : strdup for char*)
```

## Good practices

- always check return value with macros below to be notices of an error and/free memory.
- if you know rougly how many element there should be in your jarray, you should use `reserve` function to allocate memory beforehand (to reduce realloc calls).
- if you need to store pointers, you **must** implement the `copy_elem_override` function and set it in the user implementation structure of your array. Please look at file `jarray_string.c` in folder `Examples` where I implemented an array of string (char*) as an example. 

## Macros

Use these macros for automatic error checking and value extraction:
```c
JARRAY_CHECK_RET(ret);             // Print error, free error, return true if error -> if you need ret value later
JARRAY_CHECK_RET_FREE(ret);        // Print error, free ret value and error, return true if error -> if you don't need ret value later
JARRAY_GET_VALUE(type, val);       // Extract value from pointer (doesn't free)
JARRAY_DIRECT_INPUT(type, val);    // Create pointer for input value
JARRAY_RET_GET_VALUE(type, ret);   // Get value from ret (doesn't free)
JARRAY_RET_GET_VALUE_FREE(type, ret); // Get value and free ret
JARRAY_RET_GET_POINTER(type, ret); // Get pointer from ret
JARRAY_FREE_RET(ret);              // Free both value and error
JARRAY_FREE_RET_VALUE(ret);        // Free only value
JARRAY_FREE_RET_ERROR(ret);        // Free only error
```