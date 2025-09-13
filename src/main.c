#include <stdio.h>
#include <stdlib.h>
#include "../inc/jarray.h"

// ----------- Helpers -----------

// Predicate: keep only even numbers
bool is_even(const void *x, const void *ctx) {
    (void)ctx;
    return JARRAY_GET_VALUE(const int, x) % 2 == 0;
}

void modulo3(void *x, void *ctx) {
    (void)ctx;
    JARRAY_GET_VALUE(int, x) %= 3;
}

// Print an int
void print_int(const void *x) {
    printf("%d ", JARRAY_GET_VALUE(const int, x));
}

char *int_to_string(const void *x) {
    int value = JARRAY_GET_VALUE(const int, x);
    // Allocate enough space for the string representation
    char *str = (char*)malloc(12*sizeof(char)); // Enough for 32-bit int
    if (str) {
        snprintf(str, 12, "%d", value);
    }
    return str;
}

// Compare two ints
int compare_int(const void *a, const void *b) {
    return JARRAY_GET_VALUE(const int, a) - JARRAY_GET_VALUE(const int, b);
}

bool is_equal_int(const void *a, const void *b) {
    return JARRAY_GET_VALUE(const int, a) == JARRAY_GET_VALUE(const int, b);
}

void print_array_override(const JARRAY *array) {
    printf("Custom print of JARRAY [size: %zu]: ", array->_length);
    for (size_t i = 0; i < array->_length; i++) {
        void *elem = (char*)array->_data + i * array->_elem_size;
        printf("%d ", JARRAY_GET_VALUE(const int, elem));
    }
    printf("\n");
}

bool sup_8(const void *x, const void *ctx) {
    (void)ctx;
    return JARRAY_GET_VALUE(const int, x) > 8;
}

typedef struct TEST_CTX {
    int sn;
    int hn;
}TEST_CTX;

bool test_ctx_func(const void *x, const void *ctx){
    return (JARRAY_GET_VALUE(const int, x) < JARRAY_GET_VALUE(TEST_CTX, ctx).sn || JARRAY_GET_VALUE(const int, x) > JARRAY_GET_VALUE(TEST_CTX, ctx).hn);
}

void *sum(const void *accumulator, const void *elem, const void *ctx) {
    (void)ctx;
    int *result = malloc(sizeof(int));
    *result = JARRAY_GET_VALUE(const int, accumulator) + JARRAY_GET_VALUE(const int, elem);
    return result;
}

int main(void) {
    JARRAY array;

    printf("\n=== DEMO: jarray ===\n");

    // --- Adding elements ---
    printf("\nAdding numbers 1..10:\n");
    int *data_start = malloc(10 * sizeof(int));
    for (int i = 1; i <= 10; i++) {
        data_start[i-1] = i;
    }

    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;

    imp.print_element_callback = print_int;
    imp.element_to_string_callback = int_to_string;
    imp.compare_callback = compare_int;
    imp.is_equal_callback = is_equal_int;

    jarray.init_with_data(&array, data_start, 10, sizeof(int), JARRAY_TYPE_VALUE, imp);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    data_start = NULL;
    jarray.reserve(&array, 10);
    JARRAY_CHECK_RET;

    printf("Insert 11 at index 0, and 12 at index 50 (should indicate error for index 50)\n");
    jarray.add_at(&array, 0, JARRAY_DIRECT_INPUT(int, 11));
    JARRAY_CHECK_RET;
    jarray.add_at(&array, 50, JARRAY_DIRECT_INPUT(int, 12));
    JARRAY_CHECK_RET;

    printf("Full array: ");
    jarray.print(&array);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Filtering ---
    printf("\nFiltering even numbers:\n");
    JARRAY evens = jarray.filter(&array, is_even, NULL);
    JARRAY_CHECK_RET;
    jarray.print(&evens);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.free(&evens); // free filtered array

    printf("\nFiltering numbers between 3 and 9:\n");
    TEST_CTX ctx = {3, 9};
    JARRAY filtered = jarray.filter(&array, test_ctx_func, &ctx);
    JARRAY_CHECK_RET;
    jarray.print(&filtered);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.free(&filtered); // free filtered array
    // --- Sorting ---
    printf("\nSorting array:\n");
    jarray.sort(&array, QSORT, NULL);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&array);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Accessing ---
    printf("\nAccess element at index 3: ");
    int el = *(int*)jarray.at(&array, 3);
    JARRAY_CHECK_RET;
    printf("%d\n", el);

    // --- Finding ---

    printf("\nFind first even number: ");
    int first = *(int*)jarray.find_first(&array, is_even, NULL);
    JARRAY_CHECK_RET;
    printf("%d\n", first);

    printf("Find index of first even number: ");
    size_t idx = jarray.find_first_index(&array, is_even, NULL);
    JARRAY_CHECK_RET;
    printf("%zu\n", idx);

    printf("Find last even number: ");
    int last = *(int*)jarray.find_last(&array, is_even, NULL);
    JARRAY_CHECK_RET;
    printf("%d\n", last);

    printf("Find index of last even number: ");
    idx = jarray.find_last_index(&array, is_even, NULL);
    JARRAY_CHECK_RET;
    printf("%zu\n", idx);

    // --- Raw data ---
    printf("\nRaw data pointer:\n");
    int* data = (int*)jarray.copy_data(&array);
    JARRAY_CHECK_RET;
    printf("data[0] = %d\n", data[0]);
    free(data);
    data = NULL;

    // --- Subarray ---
    printf("\nSubarray [0..3]:\n");
    JARRAY sub = jarray.subarray(&array, 0, 3);
    JARRAY_CHECK_RET;
    jarray.print(&sub);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.free(&sub);

    // --- Modify ---
    printf("\nSet index 1 to 12:\n");
    jarray.set(&array, 1, JARRAY_DIRECT_INPUT(int, 12));
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&array);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Find indexes ---
    printf("\nFinding indexes of 12:\n");
    size_t* indexes = jarray.indexes_of(&array, JARRAY_DIRECT_INPUT(int, 12));
    JARRAY_CHECK_RET;
    // print matches
    printf("%zu\n", indexes[0]);
    free(indexes);

    // --- For each ---
    printf("\nFor each element, modulo 3:\n");
    jarray.for_each(&array, modulo3, NULL);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&array);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Clone ---
    printf("\nCloning array:\n");
    JARRAY clone = jarray.clone(&array);
    JARRAY_CHECK_RET;
    jarray.print(&clone);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Clear ---
    printf("\nClearing clone array:\n");
    jarray.clear(&clone);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&clone);
    JARRAY_CHECK_RET; // should print empty array

    // --- Add all ---
    printf("\nAdding all elements from original array to clone:\n");
    jarray.add_all(&clone, array._data, array._length);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&clone);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Join ---
    printf("\nJoining elements of clone array with '-' separator:\n");
    char *joined_str = jarray.join(&clone, "-");
    JARRAY_CHECK_RET;
    printf("Joined string: %s\n", joined_str);
    free(joined_str);
    joined_str = NULL;

    // --- Reduce ---
    printf("\nReducing clone array (sum of elements): ");
    int *reduced = (int*)jarray.reduce(&clone, sum, NULL, NULL);
    JARRAY_CHECK_RET;
    printf("Sum = %d\n", *reduced);
    free(reduced);
    reduced = NULL;

    // --- Reduce right ---
    printf("\nReducing clone array from the right (sum of elements): ");
    reduced = (int*)jarray.reduce_right(&clone, sum, NULL, NULL);
    JARRAY_CHECK_RET;
    printf("Sum = %d\n", *reduced);
    free(reduced);
    reduced = NULL;

    // --- Contains ---
    printf("\nChecking if clone contains 5: ");
    bool contains = jarray.contains(&clone, JARRAY_DIRECT_INPUT(int, 5));
    JARRAY_CHECK_RET;
    printf("%s\n", contains ? "Yes" : "No");

    // --- Remove all ---
    printf("\nRemoving all elements that are in clone from original array:\n");
    jarray.add(&array, JARRAY_DIRECT_INPUT(int, 17)); // add 17 to original array for testing
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.remove_all(&array, clone._data, clone._length);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&array); // Should only display 17
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Concat ---
    printf("Concat array and clone array:\n");
    JARRAY conc = jarray.concat(&array, &clone);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&conc);
    jarray.free(&conc);

    // --- Reverse ---
    printf("\nReversing clone array:\n");
    jarray.reverse(&clone);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;
    jarray.print(&clone);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;

    // --- Any ---
    printf("\nChecking if any element in clone is > 8: ");
    bool any = jarray.any(&clone, sup_8, NULL);
    JARRAY_CHECK_RET;
    printf("%s\n", any ? "Yes" : "No");

    // --- Fill ---
    printf("\nFilling clone array with fives:\n");
    jarray.fill(&clone, JARRAY_DIRECT_INPUT(int, 5), 10, clone._length + 3);
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Shift ---
    printf("\nShifting clone:\n");
    jarray.shift(&clone);
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Shift right ---
    printf("\nShifting clone to the right and add 3:\n");
    jarray.shift_right(&clone, JARRAY_DIRECT_INPUT(int, 3));
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Splice ---
    printf("\nSplicing element 1 and 2 and replaced by 10 and 15\n");
    jarray.splice(&clone, 1, 2, JARRAY_DIRECT_INPUT(int, 10), JARRAY_DIRECT_INPUT(int, 15), NULL);
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Addm (Add multiple) ---
    printf("\nAdding 25 and 30 with addm:\n");
    jarray.addm(&clone, JARRAY_DIRECT_INPUT(int, 25), JARRAY_DIRECT_INPUT(int, 30), NULL);
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Cleanup ---
    jarray.free(&array);
    jarray.free(&clone);

    printf("\n=== END DEMO ===\n");
    return EXIT_SUCCESS;
}
