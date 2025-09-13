#include "../../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>

static void print_element_array_callback(const void *x){
    printf("%.2f ", JARRAY_GET_VALUE(const float, x));
}

static char *element_to_string_array_callback(const void *x){
    float value = JARRAY_GET_VALUE(const float, x);
    // Allocate enough space for the string representation
    char *str = (char*)malloc(16*sizeof(char));
    if (str) {
        snprintf(str, 16, "%.2f", value);
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
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
    imp.print_element_callback = print_element_array_callback;
    imp.element_to_string_callback = element_to_string_array_callback;
    imp.compare_callback = compare_array_callback;
    imp.is_equal_callback = is_equal_array_callback;
    jarray.init(&array, sizeof(float), JARRAY_TYPE_VALUE, imp);
    array._type_preset = JARRAY_FLOAT_PRESET;
    return array;
}