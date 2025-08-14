#include <stdio.h>
#include <stdlib.h>
#include "my_array.h"

// Prédicat pour filtrer les nombres pairs
bool is_even(const void *x) {
    return (*(const int*)x) % 2 == 0;
}

// Fonction pour afficher un int
void print_int(const void *x) {
    printf("%d ", *(const int*)x);
}

int main(void) {
    ARRAY_RETURN ret;

    // Initialisation de l'Array
    Array array;
    ret = jarray.init(&array, sizeof(int));
    if (!ret.has_value) {
        jarray.print_array_err(ret);
        return 1;
    }

    // Ajouter des valeurs
    for (int i = 1; i <= 10; i++) {
        ret = jarray.add(&array, &i);
        if (!ret.has_value) {
            jarray.print_array_err(ret);
            free(array.data);
            return 1;
        }
    }

    // Afficher tous les éléments
    printf("Array complet : ");
    ret = jarray.print(&array, print_int);
    if (!ret.has_value) jarray.print_array_err(ret);

    // Filtrer les éléments pairs
    ret = jarray.filter(&array, is_even);
    if (!ret.has_value) {
        jarray.print_array_err(ret);
        free(array.data);
        return 1;
    }

    Array evens = GET_VALUE(Array, ret);

    // Afficher les éléments filtrés
    printf("Éléments pairs : ");
    ret = jarray.print(&evens, print_int);
    if (!ret.has_value) jarray.print_array_err(ret);
    
    size_t idx = 10;
    ret = jarray.at(&array, idx);        // <-- passer la valeur, pas l'adresse
    if (!ret.has_value) {
        jarray.print_array_err(ret);
    } else {
        printf("ret = %d\n", GET_VALUE_SAFE(int, ret, 0));  // <-- cast + deref
    }

    return 0;
}
