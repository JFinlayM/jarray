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

    // --- Initialize ---

    // Set callbacks
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp = {
        .print_element_callback = print_int,
        .compare = compare_int,
        .element_to_string = NULL,
        .is_equal = NULL
    };

    jarray.init(&array, sizeof(int), imp);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE; // Check for errors and return. This will print error that specifies this line and file.
    // Or nothing if your sure there is no error
    
    // Add elements
    for (int i = 1; i <= 5; i++) {
        jarray.add(&array, JARRAY_DIRECT_INPUT(int, i));
        if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    }
    
    jarray.print(&array); // Output: 1 2 3 4 5
    jarray.free(&array);
    return 0;
}