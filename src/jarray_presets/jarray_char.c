#include "../../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>

static void print_element_array_callback(const void *x){
    printf("%c ", JARRAY_GET_VALUE(char, x));
}

static char *element_to_string_array_callback(const void *x){
    char val = JARRAY_GET_VALUE(char, x);
    char *buf = (char*)malloc(2*sizeof(char)); 
    snprintf(buf, 2, "%c", val);
    return buf;
}

static int compare_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(char, x) - JARRAY_GET_VALUE(char, y);
}

static bool is_equal_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(char, x) == JARRAY_GET_VALUE(char, y);
}


JARRAY create_jarray_char(void){
    JARRAY array;
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
    imp.print_element_callback = print_element_array_callback;
    imp.element_to_string = element_to_string_array_callback;
    imp.compare = compare_array_callback;
    imp.is_equal = is_equal_array_callback;
    jarray.init(&array, sizeof(char), imp);
    return array;
}