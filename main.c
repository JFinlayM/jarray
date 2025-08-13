#include <stdio.h>
#include <stdlib.h>
#include "my_array.h"

bool is_even(const void *x) {
    return (*(const int*)x) % 2 == 0;
}

void print_int(const void *x) {
    printf("%d ", *(const int*)x);
}

int main(void) {
    Array array;
    array_init(&array, sizeof(int));

    // ajouter des valeurs
    for (int i = 1; i <= 10; i++) {
        jarray.add(&array, &i);
    }

    printf("Array complet : ");
    jarray.print(&array, print_int);

    // filtrer les éléments pairs
    Array evens = jarray.filter(&array, is_even);

    printf("Éléments pairs : ");
    jarray.print(&evens, print_int);

    // libération mémoire
    free(array.data);
    free(evens.data);

    return 0;
}
