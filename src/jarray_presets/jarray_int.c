#include "../../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>

static void print_element_array_callback(const void *x){
    printf("%d ", JARRAY_GET_VALUE(const int, x));
}

static char *element_to_string_array_callback(const void *x){
    int value = JARRAY_GET_VALUE(const int, x);
    // Allocate enough space for the string representation
    char *str = (char*)malloc(12*sizeof(char)); // Enough for 32-bit int
    if (str) {
        snprintf(str, 12, "%d", value);
    }
    return str;
}

static int compare_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const int, x) - JARRAY_GET_VALUE(const int, y);
}

static bool is_equal_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const int, x) == JARRAY_GET_VALUE(const int, y);
}


JARRAY create_jarray_int(void){
    JARRAY array;
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
    imp.print_element_callback = print_element_array_callback;
    imp.element_to_string = element_to_string_array_callback;
    imp.compare = compare_array_callback;
    imp.is_equal = is_equal_array_callback;
    jarray.init(&array, sizeof(int), imp);
    return array;
}