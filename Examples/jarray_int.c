#include "../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s n1 n2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    JARRAY arr_preset = jarray.init_preset(INT_PRESET);
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

    jarray.sort(&arr_preset, QSORT, NULL);
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