#include <stdio.h>
#include <stdlib.h>
#include "my_array.h"

// ----------- Helpers -----------

// Predicate: keep only even numbers
bool is_even(const void *x, const void *ctx) {
    (void)ctx;
    return GET_VALUE(const int, x) % 2 == 0;
}

// Print an int
void print_int(const void *x) {
    printf("%d ", GET_VALUE(const int, x));
}

// Compare two ints
int compare_int(const void *a, const void *b) {
    return GET_VALUE(const int, a) - GET_VALUE(const int, b);
}

bool is_equal_int(const void *a, const void *b) {
    return GET_VALUE(const int, a) == GET_VALUE(const int, b);
}

typedef struct TEST_CTX {
    int sn;
    int hn;
}TEST_CTX;

bool test_ctx_func(const void *x, const void *ctx){
    return (GET_VALUE(const int, x) > GET_VALUE(TEST_CTX, ctx).sn && GET_VALUE(const int, x) < GET_VALUE(TEST_CTX, ctx).hn);
}

int main(void) {
    ARRAY_RETURN ret;
    Array array;

    // --- Init ---
    ret = jarray.init(&array, sizeof(int));
    CHECK_RET_FREE(ret);
    array.user_implementation.print_element_callback = print_int;
    array.user_implementation.compare = compare_int;
    array.user_implementation.is_equal = is_equal_int;

    printf("\n=== DEMO: jarray ===\n");

    // --- Adding elements ---
    printf("\nAdding numbers 1..10:\n");
    for (int i = 1; i <= 10; i++) {
        ret = jarray.add(&array, TO_POINTER(int, i));
        CHECK_RET_FREE(ret);
    }

    printf("Insert 11 at index 0, and 12 at index 5\n");
    ret = jarray.add_at(&array, 0, TO_POINTER(int, 11));
    CHECK_RET_FREE(ret);
    ret = jarray.add_at(&array, 5, TO_POINTER(int, 12));
    CHECK_RET_FREE(ret);

    printf("Full array: ");
    ret = jarray.print(&array);
    CHECK_RET_FREE(ret);


    // --- Filtering ---
    printf("\nFiltering even numbers:\n");
    ret = jarray.filter(&array, is_even, NULL);
    CHECK_RET(ret);
    Array* evens = RET_GET_POINTER(Array, ret);
    ret = jarray.print(evens);
    CHECK_RET_FREE(ret);
    jarray.free(evens); // free filtered array

    printf("\nFiltering numbers between 3 and 9:\n");
    TEST_CTX ctx = {3, 9};
    ret = jarray.filter(&array, test_ctx_func, &ctx);
    CHECK_RET(ret);
    Array* filtered = RET_GET_POINTER(Array, ret);
    ret = jarray.print(filtered);
    CHECK_RET_FREE(ret);
    jarray.free(evens); // free filtered array
    // --- Sorting ---
    printf("\nSorting array:\n");
    ret = jarray.sort(&array, QSORT);
    CHECK_RET(ret);
    Array* sorted = RET_GET_POINTER(Array, ret);
    ret = jarray.print(sorted);
    CHECK_RET_FREE(ret);
    jarray.free(sorted); // free sorted copy

    // --- Accessing ---
    printf("\nAccess element at index 3: ");
    ret = jarray.at(&array, 3);
    CHECK_RET(ret);
    printf("%d\n", RET_GET_VALUE(int, ret));

    printf("Find first even number: ");
    ret = jarray.find_first(&array, is_even, NULL);
    CHECK_RET(ret);
    printf("%d\n", RET_GET_VALUE(int, ret));

    // --- Raw data ---
    printf("\nRaw data pointer:\n");
    ret = jarray.data(&array);
    CHECK_RET(ret);
    int *data = RET_GET_POINTER(int, ret);
    printf("data[0] = %d\n", data[0]);
    free(data); // because jarray.data allocated

    // --- Subarray ---
    printf("\nSubarray [0..3]:\n");
    ret = jarray.subarray(&array, 0, 3);
    CHECK_RET(ret);
    Array* sub = RET_GET_POINTER(Array, ret);
    CHECK_RET_FREE(jarray.print(sub));
    jarray.free(sub);

    // --- Modify ---
    printf("\nSet index 1 to 12:\n");
    ret = jarray.set(&array, 1, TO_POINTER(int, 12));
    CHECK_RET_FREE(ret);
    ret = jarray.print(&array);
    CHECK_RET_FREE(ret);

    ret = jarray.find_indexes(&array, TO_POINTER(int, 20));
    CHECK_RET_FREE(ret);
    size_t *indexes = RET_GET_POINTER(size_t, ret);

    // print matches
    printf("%zu\n", indexes[0]);
    free(indexes);

    // --- Cleanup ---
    printf("\nFreeing main array...\n");
    jarray.free(&array);

    printf("\n=== END DEMO ===\n");
    return 0;
}
