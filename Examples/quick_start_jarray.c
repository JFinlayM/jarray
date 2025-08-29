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