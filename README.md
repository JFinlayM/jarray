# JARRAY - Generic Dynamic Array for C on Linux

**Note**: This is a personal learning project. It is tested with AddressSanitizer/UBSan on the examples, but expect rough edges and breaking changes between versions.

A dynamic array library for C on **Linux** with filtering, sorting, searching, and more. The library is designed to be generic, allowing storage of any data type by specifying the element size during initialization. 
It provides a variety of utility functions and supports user-defined callbacks for custom behavior (a bit). Functions return their result directly. Errors are not returned: they are stored in a per-thread error trace (`jarray_last_error`) that you check after each call with the provided macros (`JARRAY_CHECK_RET`). 
The functions are inspired by higher-level languages like JavaScript and Java...

**Note**: You can store either pointers or values, but if storing pointers you have to implement a certain function `copy_elem_callback`. Please look below for more info. You can also find the file `jarray_string.c` where I implemented the functions need to have a jarray of string (char*) as a preset. You have an example of the use of this present in `jarray_string.c` in folder `Examples`.

`main.c` is complete example of a jarray storing integers. But you can store any structure in a jarray.

## Requirements & portability

- A **GCC or Clang** compiler (the convenience macros use GNU extensions: statement expressions, `__typeof__`, `__VA_OPT__`). MSVC is not supported.
- **C11** (`-std=c11` or newer) to use the generic `jarray_*` macros. The `jarray.xxx(...)` function interface works with C99.
- With the `jarray_*` macros, the value is boxed with its own C type. Make sure it matches the array's element type (for example `jarray_add(&floats, 1.5f)` for a float preset, because `1.5` is a `double` and `'c'` is an `int` in C). Strings (`"text"`, `char[]`, `char*`) work directly with the string preset.

## Install

Via Git download:
```bash
mkdir build && cd build
cmake ..            # add -DJARRAY_WERROR=ON to treat warnings as errors
sudo make lib
sudo ldconfig
```

Via dpkg:

```bash
sudo dpkg -i libjarray-dev_<version>_amd64.deb
sudo apt-get install -f
sudo ldconfig
```
This should install libjarray.so and libjarray.a in /usr/local/lib/ and jarray.h in /usr/local/include/

To use in your projet just include with *#include <jarray.h>* and link with *-ljarray* when compiling.
You can find in folder `Examples` some simple c files using jarray. These examples uses presets so there should be not user implementation function but if you want to store custom types, you will need to implement functions. You can see examples of how the functions are implemented in `src/jarray_presets` folder.
To see result (for jarray_string.c and jarray_points.c for example): 
```bash
cd Examples
make
./jarray_string hello goodbye
./jarray_points
```

## Two ways to call the library

Every operation exists twice: as `jarray.function(...)` and as `jarray_function(...)`. They do the same thing (the second one calls the first one), so you can mix them in the same program. The only difference is **how you give an element**.

### `jarray.function(...)`: the real API
`jarray` is a global structure of function pointers. Every function that takes an element takes a **pointer** to it, because the array does not know the type (it only knows the element size). So the element must live in memory:

```c
int value = 42;
jarray.add(&array, &value);                          // pointer to a variable
jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42));    // pointer to a temporary (compound literal)
jarray.splice(&array, 2, 1, JARRAY_DIRECT_INPUT(int, 25), NULL);   // variadic: NULL must end the list
```
Works with any type (including your own structures), any array (preset or not), C99, GCC and Clang.

### `jarray_function(...)`: the macros
The macros are wrappers around the same functions. For the functions that take an element (`add`, `add_at`, `addm`, `set`, `fill`, `shift_right`, `contains`, `indexes_of`, `splice`), the macro **wraps your value in a temporary for you**, using the C type of the expression you wrote. For the other functions, it only forwards its arguments.

```c
jarray_add(&array, 42);                  // same as jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42))
jarray_add(&strings, "text");            // string preset: char* / char[] / string literal work directly
jarray_add(&points, p);                  // any type works if the expression has the array's element type (here a Point variable)
jarray_add(&points, ((Point){1, 2}));    // a compound literal needs extra parentheses, otherwise its comma splits the macro arguments
jarray_splice(&array, 2, 1, 25, 27);     // no NULL needed at the end
```
Needs C11 and GCC/Clang.

### Which one to use?
| | `jarray.function` | `jarray_function` |
|---|---|---|
| Element argument | pointer (`&value`, `JARRAY_DIRECT_INPUT(type, v)`) | the value itself |
| Variadic functions (`addm`, `splice`) | end the list with `NULL` | no `NULL` needed |
| Needs | C99 | C11 |
| Type safety | none (`void*`), you must pass the right type | none either: the macro uses the type of **your expression** |

Use the macros for the common case (presets, literals, values). Use `jarray.function` when you already have a pointer to the element (a variable, an item of another array), or when you build for C99.

**Warning**: with the macros, the type of the value must be the array's element type. `jarray_add(&floats, 1.5)` adds a `double` (8 bytes) to a float array. Write `1.5f`. Likewise `'c'` is an `int` in C, so cast it for a char array: `(char)'c'`. The compiler cannot warn you.

## Quick Start

```c
#include <jarray.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s n1 n2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    JARRAY arr_preset = jarray.init_preset(JARRAY_INT_PRESET);
    jarray.reserve(&arr_preset, 5);
    JARRAY_CHECK_RET;

    for (int i = 1; i < argc; i++) {
        jarray.add(&arr_preset, JARRAY_DIRECT_INPUT(int, atoi(argv[i])));
        JARRAY_CHECK_RET;
    }
    jarray.print(&arr_preset);

    char *joined = jarray.join(&arr_preset, ", ");
    JARRAY_CHECK_RET;
    printf("Joined string: %s\n", joined);
    free(joined);

    jarray.sort(&arr_preset, JARRAY_QSORT, NULL);
    JARRAY_CHECK_RET;
    jarray.print(&arr_preset);
    JARRAY_CHECK_RET;

    jarray.add(&arr_preset, JARRAY_DIRECT_INPUT(int, 9));
    JARRAY_CHECK_RET;
    jarray.print(&arr_preset);
    JARRAY_CHECK_RET;

    bool contains = jarray.contains(&arr_preset, JARRAY_DIRECT_INPUT(int, -3));
    JARRAY_CHECK_RET;
    printf("Contains -3 ? %s\n", contains ? "true" : "false");

    jarray.splice(&arr_preset, 2, 1, JARRAY_DIRECT_INPUT(int, 25), NULL);
    JARRAY_CHECK_RET;
    jarray.print(&arr_preset);
    JARRAY_CHECK_RET;

    // --- Cleanup ---
    jarray.free(&arr_preset);

    return EXIT_SUCCESS;
}
```
### Output

```
JARRAY [size: 5] =>
1 2 3 4 5 
```

In the previous example, the preset `JARRAY_INT_PRESET` is used, so you can use macros and not directly functions so you don't need to use `JARRAY_DIRECT_INPUT`. See [Two ways to call the library](#two-ways-to-call-the-library) for the difference. Example with the macros : 

```c
#include "../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s n1 n2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    JARRAY arr_preset = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_reserve(&arr_preset, 5);
    JARRAY_CHECK_RET;

    for (int i = 1; i < argc; i++) {
        jarray_add(&arr_preset, atoi(argv[i]));
        JARRAY_CHECK_RET;
    }
    jarray_print(&arr_preset);

    char *joined = jarray_join(&arr_preset, ", ");
    JARRAY_CHECK_RET;
    printf("Joined string: %s\n", joined);
    free(joined);

    jarray_sort(&arr_preset, JARRAY_QSORT, NULL);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;

    jarray_add(&arr_preset, 9);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;

    bool contains = jarray_contains(&arr_preset, -3);
    JARRAY_CHECK_RET;
    printf("Contains -3 ? %s\n", contains ? "true" : "false");

    // For splice and addm, if using macro, you don't need to add NULL as last argument.
    jarray_splice(&arr_preset, 2, 1, 25, 27);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;
    jarray_addm(&arr_preset, 26, 27);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;
    jarray_splice(&arr_preset, 2, 2);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;

    // --- Cleanup ---
    jarray_free(&arr_preset);

    return EXIT_SUCCESS;
}
```


## Core Functions

### Basic Operations
```c
jarray.init(&array, sizeof(int), JARRAY_TYPE_VALUE, imp);    // Initialize empty array (JARRAY_TYPE_VALUE or JARRAY_TYPE_POINTER)
jarray.init_reserve(&array, sizeof(int), capacity, JARRAY_TYPE_VALUE, imp) // Initialize array and reserve memory
jarray.init_with_data(&array, data, count, sizeof(int), JARRAY_TYPE_VALUE, imp);// Initialize with raw data (don't use if data is stack allocated)
jarray.init_with_data_copy(&array, data, count, sizeof(int), JARRAY_TYPE_VALUE, imp);// Initialize with a copy of raw data 
jarray.init_preset(preset);                             // Returns a jarray with all the user functions implemented for a given preset
jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42));       // Append element
jarray.addm(&array, ...);                               // Appends elements to the end of the array.
jarray.add_all(&array, data, count);                    // Append multiple elements
jarray.add_at(&array, index, &value);                   // Insert at index
jarray.at(&array, index);                               // Get element at index
jarray.set(&array, index, &value);                      // Overwrite element
jarray.remove(&array);                                  // Remove last element (and frees it)
jarray.remove_at(&array, index);                        // Remove at index (and frees it)
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
jarray.sort(&array, JARRAY_QSORT, compare);                    // Sort (either compare callback in arg or in user_implementation)
jarray.filter(&array, predicate, ctx);                  // Filter by condition (predicate callback and optional context)
jarray.subarray(&array, start, end);                    // Extract subarray
jarray.find_first(&array, predicate, ctx);              // First match by predicate
jarray.find_first_index(&array, predicate, ctx);        // Index of first match by predicate
jarray.find_last(&array, predicate, ctx);               // Last match by predicate
jarray.find_last_index(&array, predicate, ctx);         // Index of last match by predicate
jarray.indexes_of(&array, &value);                    // All indexes of a value
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
JARRAY evens = jarray.filter(&array, is_even, NULL); // No context needed here -> NULL
JARRAY_CHECK_RET_RETURN;
jarray.print(&evens);
JARRAY_CHECK_RET;
jarray.free(&evens);
JARRAY_CHECK_RET;

// Find first
int even = *(int*)jarray.find_first(&array, is_even, NULL); // No context needed here -> NULL
JARRAY_CHECK_RET_RETURN;
printf("First even: %d\n", even);
```

### Custom Types
```c
typedef struct { int x, y; } Point;

void print_point(const void *p) {
    Point pt = JARRAY_GET_VALUE(const Point, p);
    printf("(%d,%d) ", pt.x, pt.y);
}

JARRAY points;

JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
imp.print_element_callback = print_point;

jarray.init(&points, sizeof(Point), JARRAY_TYPE_VALUE, imp);
JARRAY_CHECK_RET_RETURN;

Point p = {3, 4};
jarray.add(&points, &p);
JARRAY_CHECK_RET;

jarray.print(&points);
JARRAY_CHECK_RET;
```

### Sorting
```c
// Sort methods: JARRAY_QSORT, JARRAY_BUBBLE_SORT, JARRAY_INSERTION_SORT, JARRAY_SELECTION_SORT
array.user_callbacks.compare = compare_func; // Set comparison function
jarray.sort(&array, JARRAY_QSORT, NULL);
```
Or
```c
jarray.sort(&array, JARRAY_QSORT, compare_func);
```

## Required Callbacks

Set these before using related functions:
```c
JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
imp.print_element_callback = print_element_array_callback;  // For print()
imp.element_to_string_callback = element_to_string_array_callback;   // For join()
imp.compare_callback = compare_array_callback;                       // For sort()
imp.is_equal_callback = is_equal_array_callback;                     // For contains(), indexes_of()
imp.copy_elem_callback = copy_elem_func;                    // For copy override. MANDATORY when storing pointers (Example : strdup for char*)
```

## Override callbacks

There is some functions that can be overriden. Maybe more will be added later:
```c
JARRAY_USER_OVERRIDE_IMPLEMENTATION imp;
imp.print_error_override = error_func;        // For error printing
imp.print_jarray_override = print_array_func;  // For print() override
```

## Good practices
- you **should** implement every function of `JARRAY_USER_CALLBACK_IMPLEMENTATION`.
- always check return value with macros below to be noticed if the last jarray function call produced an error.
- if you know rougly how many element there should be in your jarray, you should use `reserve` function to allocate memory beforehand (to reduce realloc calls).
- if you need to store pointers, you **must** implement the `copy_elem_callback` function and set it in the user implementation structure of your array. Please look at file `jarray_string.c` in folder `Examples` where I implemented an array of string (char*) as an example. 

## Memory ownership

Who frees what (`T` is the element type; "pointer array" means `JARRAY_TYPE_POINTER`, like the string preset):

| Function(s) | Ownership |
|---|---|
| `add`, `add_at`, `addm`, `add_all`, `set`, `fill`, `shift_right`, `splice` | The element is **copied** (with `copy_elem_callback` for a pointer array). You keep ownership of what you pass. |
| `remove`, `remove_at`, `remove_all`, `shift`, `set`, `clear` | The removed or overwritten element is **freed** by the library (pointer arrays: the pointed data too). |
| `at`, `find_first`, `find_last` | Returns a pointer **inside** the array. Do not free it. It is invalid after the array is modified or freed. |
| `copy_data`, `reduce`, `reduce_right` | You must `free` the result. For `copy_data` on a pointer array, also free each element. |
| `indexes_of` | You must `free` the result (`result[0]` is the count, the indexes follow). |
| `join` | You must `free` the returned string. |
| `clone`, `filter`, `subarray`, `concat` | Returns a new, independent JARRAY (deep copy for pointer arrays). Call `jarray.free` on it. |
| `sort`, `reverse` | In place. Elements are only moved, nothing is allocated or freed. |
| `free` | Frees the data (and the pointed data for a pointer array). Does not free the JARRAY struct itself. |

A JARRAY must not be used from several threads at the same time without a lock, but the error trace (`jarray_last_error`) is per thread.

## Macros

Use these macros for automatic error checking and value extraction:
```c
JARRAY_CHECK_RET;                   // Print error, free error, return true if error
JARRAY_CHECK_RET_RETURN;            // Print error, free error, return EXIT_FAILURE if error
JARRAY_GET_VALUE(type, val);        // Extract value from pointer (doesn't free)
JARRAY_DIRECT_INPUT(type, val);     // Create pointer for input value if using custom type or STRING preset
```