#include "../../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>

static void print_element_array_callback(const void *x){
    printf("%hu ", JARRAY_GET_VALUE(const unsigned short, x));
}

static char *element_to_string_array_callback(const void *x){
    unsigned short value = JARRAY_GET_VALUE(const unsigned short, x);
    // Allocate enough space for the string representation
    char *str = (char*)malloc(6*sizeof(char));
    if (str) {
        snprintf(str, 6, "%hu", value);
    }
    return str;
}

static int compare_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const unsigned short, x) - JARRAY_GET_VALUE(const unsigned short, y);
}

static bool is_equal_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const unsigned short, x) == JARRAY_GET_VALUE(const unsigned short, y);
}


JARRAY create_jarray_ushort(void){
    JARRAY array;
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
    imp.print_element_callback = print_element_array_callback;
    imp.element_to_string = element_to_string_array_callback;
    imp.compare = compare_array_callback;
    imp.is_equal = is_equal_array_callback;
    jarray.init(&array, sizeof(unsigned short), imp);
    return array;
}