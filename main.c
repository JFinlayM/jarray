#include <stdio.h>
#include <stdlib.h>
#include "jarray.h"

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
    char *str = malloc(12); // Enough for 32-bit int
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

void print_error_callback(const JARRAY_RETURN_ERROR error) {
    fprintf(stderr, "[\033[31mError: %s\033[0m]\n", error.error_msg);
    free(error.error_msg); // Free the error message after printing
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
    JARRAY_RETURN ret;
    JARRAY array;

    // --- Init ---
    ret = jarray.init(&array, sizeof(int));
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;


    printf("\n=== DEMO: jarray ===\n");

    // --- Adding elements ---
    printf("\nAdding numbers 1..10:\n");
    int *data_start = malloc(10 * sizeof(int));
    for (int i = 1; i <= 10; i++) {
        //ret = jarray.add(&array, JARRAY_DIRECT_INPUT(int, i));
        //if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
        data_start[i-1] = i;
   }
   ret = jarray.init_with_data(&array, data_start, 10, sizeof(int));
   if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

   free(data_start); // data copied into array, can free original

   array.user_implementation.print_element_callback = print_int;
    array.user_implementation.element_to_string = int_to_string;
   array.user_implementation.compare = compare_int;
   array.user_implementation.is_equal = is_equal_int;

    printf("Insert 11 at index 0, and 12 at index 5\n");
    ret = jarray.add_at(&array, 0, JARRAY_DIRECT_INPUT(int, 11));
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.add_at(&array, 5, JARRAY_DIRECT_INPUT(int, 12));
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    printf("Full array: ");
    ret = jarray.print(&array);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;


    // --- Filtering ---
    printf("\nFiltering even numbers:\n");
    ret = jarray.filter(&array, is_even, NULL);
    JARRAY_CHECK_RET(ret);
    JARRAY* evens = JARRAY_RET_GET_POINTER(JARRAY, ret);
    ret = jarray.print(evens);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    jarray.free(evens); // free filtered array

    printf("\nFiltering numbers between 3 and 9:\n");
    TEST_CTX ctx = {3, 9};
    ret = jarray.filter(&array, test_ctx_func, &ctx);
    JARRAY_CHECK_RET(ret);
    JARRAY* filtered = JARRAY_RET_GET_POINTER(JARRAY, ret);
    ret = jarray.print(filtered);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    jarray.free(filtered); // free filtered array
    // --- Sorting ---
    printf("\nSorting array:\n");
    ret = jarray.sort(&array, QSORT, NULL);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.print(&array);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    // --- Accessing ---
    printf("\nAccess element at index 3: ");
    ret = jarray.at(&array, 3);
    JARRAY_CHECK_RET(ret);
    printf("%d\n", JARRAY_RET_GET_VALUE(int, ret));

    printf("Find first even number: ");
    ret = jarray.find_first(&array, is_even, NULL);
    JARRAY_CHECK_RET(ret);
    printf("%d\n", JARRAY_RET_GET_VALUE(int, ret));

    // --- Raw data ---
    printf("\nRaw data pointer:\n");
    ret = jarray.data(&array);
    JARRAY_CHECK_RET(ret);
    int *data = JARRAY_RET_GET_POINTER(int, ret);
    printf("data[0] = %d\n", data[0]);
    free(data); // because jarray.data allocated

    // --- Subarray ---
    printf("\nSubarray [0..3]:\n");
    ret = jarray.subarray(&array, 0, 3);
    JARRAY_CHECK_RET(ret);
    JARRAY* sub = JARRAY_RET_GET_POINTER(JARRAY, ret);
    ret = jarray.print(sub);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    jarray.free(sub);

    // --- Modify ---
    printf("\nSet index 1 to 12:\n");
    ret = jarray.set(&array, 1, JARRAY_DIRECT_INPUT(int, 12));
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.print(&array);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    // --- Find indexes ---
    printf("\nFinding indexes of 12:\n");
    ret = jarray.find_indexes(&array, JARRAY_DIRECT_INPUT(int, 12));
    JARRAY_CHECK_RET(ret);
    size_t *indexes = JARRAY_RET_GET_POINTER(size_t, ret);

    // print matches
    printf("%zu\n", indexes[0]);
    free(indexes);

    // --- For each ---
    printf("\nFor each element, modulo 3:\n");
    ret = jarray.for_each(&array, modulo3, NULL);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.print(&array);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    // --- Clone ---
    printf("\nCloning array:\n");
    ret = jarray.clone(&array);
    JARRAY_CHECK_RET(ret);
    JARRAY* clone = JARRAY_RET_GET_POINTER(JARRAY, ret);
    ret = jarray.print(clone);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    // --- Clear ---
    printf("\nClearing clone array:\n");
    ret = jarray.clear(clone);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.print(clone);
    JARRAY_CHECK_RET_FREE(ret); // should print empty array

    // --- Add all ---
    printf("\nAdding all elements from original array to clone:\n");
    ret = jarray.add_all(clone, array._data, array._length);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.print(clone);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    // --- Join ---
    printf("\nJoining elements of clone array with ', ' separator:\n");
    ret = jarray.join(clone, "-");
    JARRAY_CHECK_RET(ret);
    char *joined_str = JARRAY_RET_GET_POINTER(char, ret);
    printf("Joined string: %s\n", joined_str);
    joined_str = NULL;
    JARRAY_FREE_RET(ret);

    // --- Reduce ---
    printf("\nReducing clone array (sum of elements): ");
    ret = jarray.reduce(clone, sum, NULL, NULL);
    JARRAY_CHECK_RET(ret);
    printf("Sum = %d\n", JARRAY_RET_GET_VALUE_FREE(int, ret));

    // --- Contains ---
    printf("\nChecking if clone contains 5: ");
    ret = jarray.contains(clone, JARRAY_DIRECT_INPUT(int, 5));
    JARRAY_CHECK_RET(ret);
    printf("%s\n", JARRAY_RET_GET_VALUE_FREE(bool, ret) ? "Yes" : "No");

    // --- Remove all ---
    printf("\nRemoving all elements that are in clone from original array:\n");
    jarray.add(&array, JARRAY_DIRECT_INPUT(int, 17)); // add 17 to original array for testing
    ret = jarray.data(clone);
    if (JARRAY_CHECK_RET(ret)) return EXIT_FAILURE;
    void *data_clone = JARRAY_RET_GET_POINTER(void*, ret);
    ret = jarray.length(clone);
    if (JARRAY_CHECK_RET(ret)) {
        free(data_clone);
        return EXIT_FAILURE;
    }
    size_t count = JARRAY_RET_GET_VALUE(size_t, ret);
    ret = jarray.remove_all(&array, data_clone, count);
    free(data_clone);
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;
    ret = jarray.print(&array); // Should only display 17
    if (JARRAY_CHECK_RET_FREE(ret)) return EXIT_FAILURE;

    // --- Concat ---
    printf("Concat array and clone array:\n");
    ret = jarray.concat(&array, clone);
    if (JARRAY_CHECK_RET(ret)) return EXIT_FAILURE;
    JARRAY *conc = JARRAY_RET_GET_POINTER(JARRAY, ret);
    jarray.print(conc);
    jarray.free(conc);


    // --- Cleanup ---
    printf("\nFreeing array...\n");
    jarray.free(&array);
    jarray.free(clone);

    printf("\n=== END DEMO ===\n");
    return EXIT_SUCCESS;
}
