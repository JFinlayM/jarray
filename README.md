# JARRAY - Generic Dynamic Array for C

**Note**: I'm a beginner programmer, so don't take this code too seriously. This is a learning project!

A flexible dynamic array library for C with filtering, sorting, searching, and more.

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
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    
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
jarray.init(&array, sizeof(int))           // Initialize array
jarray.init_with_data(&array, data, 10, sizeof(int)); // Initialize array with data
jarray.add(&array, JARRAY_DIRECT_INPUT(int, 42))  // Add element
jarray.at(&array, index)                   // Get element at index
jarray.set(&array, index, &value)          // Set element
jarray.remove(&array)                      // Remove last element
jarray.remove_at(&array, index)            // Remove at index
jarray.length(&array)                      // Get length
jarray.print(&array)                       // Print array
jarray.free(&array)                        // Free memory
```

### Advanced Operations
```c
jarray.filter(&array, predicate, ctx)      // Filter elements
jarray.sort(&array, QSORT)                 // Sort array
jarray.find_first(&array, predicate, ctx)  // Find first match
jarray.contains(&array, &value)            // Check if contains
jarray.clone(&array)                       // Clone array
jarray.subarray(&array, start, end)        // Get subarray
jarray.for_each(&array, callback, ctx)     // Apply function to all
```

## Memory Management Rules

| Function | Must Free Result? |
|----------|------------------|
| `at()`, `find_first()` | No (points into array) |
| `filter()`, `clone()`, `subarray()` | Yes (use `jarray.free()`) |
| `data()`, `find_indexes()` | Yes (use `free()`) |
| `remove()`, `remove_at()` | Yes (use `free()`) |

## Examples

### Filtering and Searching
```c
bool is_even(const void *x, const void *ctx) {
    return JARRAY_GET_VALUE(const int, x) % 2 == 0;
}

// Filter
ret = jarray.filter(&array, is_even, NULL);
JARRAY *evens = JARRAY_RET_GET_POINTER(JARRAY, ret);
jarray.print(evens);
jarray.free(evens); // Important!

// Find first
ret = jarray.find_first(&array, is_even, NULL);
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
jarray.sort(&array, QSORT);  // Fastest for large arrays
```

## Required Callbacks

Set these before using related functions:
```c
array.user_implementation.print_element_callback = print_func;  // For print()
array.user_implementation.compare = compare_func;              // For sort()
array.user_implementation.is_equal = equal_func;               // For contains(), find_indexes()
```

## Error Handling

Use these macros for automatic error checking:
```c
JARRAY_CHECK_RET(ret);           // Print error, return true if error
JARRAY_CHECK_RET_FREE(ret);      // Print error, free array, return true if error
```

## Best Practices

1. **Always set callbacks** before using functions that need them
2. **Use error checking macros** consistently
3. **Free returned arrays/data** to prevent memory leaks
4. **Use JARRAY_DIRECT_INPUT** for cleaner code
5. **Don't modify array during iteration**