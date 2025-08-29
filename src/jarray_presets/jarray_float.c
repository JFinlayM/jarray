#include "../../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>

static void print_element_array_callback(const void *x){
    printf("%.2f ", JARRAY_GET_VALUE(const float, x));
}

static char *element_to_string_array_callback(const void *x){
    float value = JARRAY_GET_VALUE(const float, x);
    // Allocate enough space for the string representation
    char *str = (char*)malloc(12*sizeof(char));
    if (str) {
        snprintf(str, 12, "%.2f", value);
    }
    return str;
}

static int compare_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const float, x) - JARRAY_GET_VALUE(const float, y);
}

static bool is_equal_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const float, x) == JARRAY_GET_VALUE(const float, y);
}


JARRAY create_jarray_float(void){
    JARRAY array;
    jarray.init(&array, sizeof(float));
    array.user_implementation.print_element_callback = print_element_array_callback;
    array.user_implementation.element_to_string = element_to_string_array_callback;
    array.user_implementation.compare = compare_array_callback;
    array.user_implementation.is_equal = is_equal_array_callback;
    return array;
}