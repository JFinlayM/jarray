#include "../../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void print_element_array_callback(const void *x){
    printf("%.2lf ", JARRAY_GET_VALUE(const double, x));
}

static char *element_to_string_array_callback(const void *x){
    double value = JARRAY_GET_VALUE(const double, x);
    // Allocate enough space for the string representation
    char *str = (char*)malloc(32*sizeof(char));
    if (str) {
        snprintf(str, 32, "%.2lf", value);
    }
    return str;
}

static int compare_array_callback(const void *x, const void *y){
    return JARRAY_GET_VALUE(const double, x) - JARRAY_GET_VALUE(const double, y);
}

static bool is_equal_array_callback(const void *x, const void *y){
    const double x_d = JARRAY_GET_VALUE(const double, x);
    const double y_d = JARRAY_GET_VALUE(const double, y);
    return x_d == y_d;
}


JARRAY create_jarray_double(void){
    JARRAY array;
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;
    imp.print_element_callback = print_element_array_callback;
    imp.element_to_string_callback = element_to_string_array_callback;
    imp.compare_callback = compare_array_callback;
    imp.is_equal_callback = is_equal_array_callback;
    jarray.init(&array, sizeof(double), JARRAY_TYPE_VALUE, imp);
    array._type_preset = JARRAY_DOUBLE_PRESET;
    return array;
}