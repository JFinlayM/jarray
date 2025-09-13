#include "../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s n1 n2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    JARRAY arr_preset = jarray_init_preset(JARRAY_DOUBLE_PRESET);
    jarray_reserve(&arr_preset, 5);
    JARRAY_CHECK_RET;

    for (int i = 1; i < argc; i++) {
        jarray_add(&arr_preset, atof(argv[i]));
        JARRAY_CHECK_RET;
    }
    jarray_print(&arr_preset);

    char* joined = jarray_join(&arr_preset, ", ");
    JARRAY_CHECK_RET_RETURN;
    printf("Joined string: %s\n", joined);

    jarray_sort(&arr_preset, QSORT, NULL);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;

    jarray_add(&arr_preset, 9.5);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;

    bool contains = jarray_contains(&arr_preset, 5.00);
    JARRAY_CHECK_RET;
    printf("Contains 5.0 ? %s\n", contains ? "true" : "false");

    jarray_splice(&arr_preset, 2, 1, 25.94);
    JARRAY_CHECK_RET;
    jarray_print(&arr_preset);
    JARRAY_CHECK_RET;

    // --- Cleanup ---
    jarray_free(&arr_preset);

    return EXIT_SUCCESS;
}