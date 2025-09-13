#include "../../inc/jarray.h"
#include <stdio.h>

static char *my_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}


static void print_element_array_callback(const void *x){
    char **str = (char**)x;
    printf("%s ", *str);
}

static char *element_to_string_array_callback(const void *x){
    char **str = (char**)x;
    return my_strdup(*str);
}

static int compare_array_callback(const void *x, const void *y){
    return strcmp(*(char**)x, *(char**)y);
}

static bool is_equal_array_callback(const void *x, const void *y){
    char **str_x = (char**)x;
    char **str_y = (char**)y;
    int cmp = strcmp(*str_x, *str_y);
    if (cmp != 0) return false;
    return true;
}


static void *copy_elem_override(const void *x){
    char **str = (char**)x;
    char **res = (char**)malloc(sizeof(char**));
    *res = my_strdup(*str);
    return res;
}


JARRAY create_jarray_string(void){

    JARRAY array;
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
    imp.print_element_callback = print_element_array_callback;
    imp.element_to_string_callback = element_to_string_array_callback;
    imp.compare_callback = compare_array_callback;
    imp.is_equal_callback = is_equal_array_callback;
    imp.copy_elem_callback = copy_elem_override;
    jarray.init(&array, sizeof(char*), JARRAY_TYPE_POINTER, imp);
    array._type_preset = JARRAY_STRING_PRESET;
    return array;
}