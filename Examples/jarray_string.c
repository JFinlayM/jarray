#include "../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s word1 word2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    JARRAY arr_preset = jarray.init_preset(JARRAY_STRING_PRESET);
    jarray.reserve(&arr_preset, 5);
    JARRAY_CHECK_RET;

    for (int i = 1; i < argc; i++) {
        jarray.add(&arr_preset, &argv[i]);
        JARRAY_CHECK_RET;
    }
    jarray.print(&arr_preset);

    char *joined = jarray.join(&arr_preset, ", ");
    JARRAY_CHECK_RET;
    printf("Joined string: %s\n", joined);
    free(joined);

    jarray.sort(&arr_preset, QSORT, NULL);
    JARRAY_CHECK_RET;
    jarray.print(&arr_preset);
    JARRAY_CHECK_RET;

    jarray.add(&arr_preset, JARRAY_DIRECT_INPUT(char*, "thanks"));
    JARRAY_CHECK_RET;
    jarray.print(&arr_preset);
    JARRAY_CHECK_RET;

    bool contains = jarray.contains(&arr_preset, JARRAY_DIRECT_INPUT(char*, "hello"));
    JARRAY_CHECK_RET;
    printf("Contains 'hello' ? %s\n", contains ? "true" : "false");

    jarray.splice(&arr_preset, 2, 1, JARRAY_DIRECT_INPUT(char*, "great"), NULL);
    JARRAY_CHECK_RET;
    jarray.print(&arr_preset);
    JARRAY_CHECK_RET;

    char ** val = (char**)jarray.at(&arr_preset, 2);
    JARRAY_CHECK_RET_RETURN;
    printf("val at index 2 : %s\n", *val);

    char **values = (char**)jarray.copy_data(&arr_preset);
    for (size_t i = 0; i < arr_preset._length; i++){
        printf("%s\n", values[i]);
        free(values[i]);
    }
    free(values);

    // --- Cleanup ---
    jarray.free(&arr_preset);

    return EXIT_SUCCESS;
}