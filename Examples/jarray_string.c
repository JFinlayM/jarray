#include <jarray.h>
#include <stdio.h>
#include <stdlib.h>

static char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}


void print_element_array_callback(const void *x){
    char *str = (char*)x;
    printf("%s ", str);
}

char *element_to_string_array_callback(const void *x){
    char *str = (char*)x;
    return strdup(str);
}

int compare_array_callback(const void *x, const void *y){
    return strcmp((char*)x, (char*)y);
}

bool is_equal_array_callback(const void *x, const void *y){
    char *str_x = (char*)x;
    char *str_y = (char*)y;
    int cmp = strcmp(str_x, str_y);
    if (cmp != 0) return false;
    return true;
}


void *copy_elem_override(const void *x){
    char *str = (char*)x;
    return strdup(str);
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s word1 word2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }
    JARRAY array;
    JARRAY_RETURN ret = jarray.init(&array, sizeof(char*));
    JARRAY_CHECK_RET(ret);
    ret = jarray.reserve(&array, 5);
    array.user_implementation.print_element_callback = print_element_array_callback;
    array.user_implementation.element_to_string = element_to_string_array_callback;
    array.user_implementation.compare = compare_array_callback;
    array.user_implementation.is_equal = is_equal_array_callback;
    array.user_implementation.copy_elem_override = copy_elem_override;

    for (int i = 1; i < argc; i++) {
        ret = jarray.add(&array, argv[i]);
        JARRAY_CHECK_RET(ret);
    }

    ret = jarray.print(&array);
    JARRAY_CHECK_RET(ret);

    ret = jarray.join(&array, ", ");
    JARRAY_CHECK_RET(ret);
    printf("Joined string: %s\n", JARRAY_RET_GET_POINTER(char, ret));

    ret = jarray.sort(&array, QSORT, NULL);
    JARRAY_CHECK_RET(ret);
    ret = jarray.print(&array);
    JARRAY_CHECK_RET(ret);

    ret = jarray.add(&array, "thanks");
    JARRAY_CHECK_RET(ret);
    ret = jarray.print(&array);
    JARRAY_CHECK_RET(ret);

    ret = jarray.contains(&array, "hello");
    JARRAY_CHECK_RET(ret);
    printf("Contains 'hello' ? %s\n", JARRAY_RET_GET_VALUE_FREE(bool, ret) ? "true" : "false");

    ret = jarray.splice(&array, 2, 1, "great", NULL);
    JARRAY_CHECK_RET(ret);
    ret = jarray.print(&array);
    JARRAY_CHECK_RET(ret);
    
    // --- Cleanup ---
    jarray.free(&array);

    return EXIT_SUCCESS;
}