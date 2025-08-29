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
    jarray.init(&array, sizeof(int));
    array.user_implementation.print_element_callback = print_element_array_callback;
    array.user_implementation.element_to_string = element_to_string_array_callback;
    array.user_implementation.compare = compare_array_callback;
    array.user_implementation.is_equal = is_equal_array_callback;
    return array;
}