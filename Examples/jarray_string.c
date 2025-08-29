#include "../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s word1 word2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    JARRAY arr_preset = jarray.init_preset(STRING_PRESET);
    JARRAY_RETURN ret = jarray.reserve(&arr_preset, 5);
    JARRAY_CHECK_RET(ret);

    for (int i = 1; i < argc; i++) {
        ret = jarray.add(&arr_preset, argv[i]);
        JARRAY_CHECK_RET(ret);
    }
    jarray.print(&arr_preset);

    ret = jarray.join(&arr_preset, ", ");
    JARRAY_CHECK_RET(ret);
    printf("Joined string: %s\n", JARRAY_RET_GET_POINTER(char, ret));

    ret = jarray.sort(&arr_preset, QSORT, NULL);
    JARRAY_CHECK_RET(ret);
    ret = jarray.print(&arr_preset);
    JARRAY_CHECK_RET(ret);

    ret = jarray.add(&arr_preset, "thanks");
    JARRAY_CHECK_RET(ret);
    ret = jarray.print(&arr_preset);
    JARRAY_CHECK_RET(ret);

    ret = jarray.contains(&arr_preset, "hello");
    JARRAY_CHECK_RET(ret);
    printf("Contains 'hello' ? %s\n", JARRAY_RET_GET_VALUE_FREE(bool, ret) ? "true" : "false");

    ret = jarray.splice(&arr_preset, 2, 1, "great", NULL);
    JARRAY_CHECK_RET(ret);
    ret = jarray.print(&arr_preset);
    JARRAY_CHECK_RET(ret);

    // --- Cleanup ---
    jarray.free(&arr_preset);

    return EXIT_SUCCESS;
}