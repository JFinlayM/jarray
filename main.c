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

int compare_int(const void *a, const void *b) {
    return (*(const int*)a) - (*(const int*)b);
}

int main(void) {
    ARRAY_RETURN ret;

    // Initialisation de l'Array
    Array array;
    ret = jarray.init(&array, sizeof(int));
    CHECK_RET(ret);
    array.user_implementation.print_element_callback = print_int;
    array.user_implementation.compare = compare_int;

    // Ajouter des valeurs
    for (int i = 1; i <= 10; i++) {
        ret = jarray.add(&array, TO_POINTER(int, i));
        CHECK_RET(ret);
    }
    ret = jarray.add_at(&array, 0, TO_POINTER(int, 11)); // Ajouter 0 au début
    CHECK_RET(ret);
    ret = jarray.add_at(&array, 5, TO_POINTER(int, 12)); // Ajouter 12 à l'index 5
    CHECK_RET(ret);

    // Afficher tous les éléments
    printf("Array complet : ");
    ret = jarray.print(&array);
    CHECK_RET(ret);

    // Filtrer les éléments pairs
    ret = jarray.filter(&array, is_even);
    CHECK_RET(ret);

    Array evens = GET_VALUE(Array, ret);

    // Afficher les éléments filtrés
    printf("Éléments pairs : ");
    ret = jarray.print(&evens);
    CHECK_RET(ret);

    // Trier l'Array
    ret = jarray.sort(&array, QSORT);
    CHECK_RET(ret);
    
    Array* sorted_array = GET_POINTER(Array, ret);
    printf("Array trié : ");
    ret = jarray.print(sorted_array);
    CHECK_RET(ret);

    
    size_t idx = 15;
    ret = jarray.at(&array, idx);        // <-- passer la valeur, pas l'adresse
    CHECK_RET(ret);
    printf("ret = %d\n", GET_VALUE_SAFE(int, ret, 0));  // <-- cast + deref
    

    return 0;
}
