#ifndef MY_ARRAY_H
#define MY_ARRAY_H

#include <stdbool.h>
#include <stddef.h>

// Codes d'erreur
typedef enum {
    INDEX_OUT_OF_BOUND = 0,
    ARRAY_UNINITIALIZED,
    DATA_NULL
} ARRAY_ERROR;

// État de l'Array
typedef enum {
    UNINITIALIZED = 0,
    INITIALIZED
} Array_state_t;

// Structure pour stocker une erreur
typedef struct{
    ARRAY_ERROR error_code;
    char* error_msg;
} ARRAY_RETURN_ERROR;

// Structure de retour générique
typedef struct {
    union {
        void* value;                // pointeur vers valeur (Array ou élément)
        ARRAY_RETURN_ERROR error;   // message d'erreur
    };
    bool has_value;                 // vrai si value valide
} ARRAY_RETURN;

// Structure principale Array
typedef struct Array {
    void *data;
    size_t length;
    size_t elem_size;
    Array_state_t state;
} Array;

// Interface "statique" pour méthodes
typedef struct Jarray {
    ARRAY_RETURN (*filter)(struct Array *self, bool (*predicate)(const void *));
    ARRAY_RETURN (*at)(struct Array *self, size_t index);
    ARRAY_RETURN (*add)(struct Array *self, const void * elem);
    ARRAY_RETURN (*init)(struct Array *array, size_t elem_size);
    ARRAY_RETURN (*init_with_data)(struct Array *array, void *data, size_t length, size_t elem_size);
    ARRAY_RETURN (*print)(struct Array *self, void (*print_element_callback)(const void *));
    void (*print_array_err)(ARRAY_RETURN ret);
} Jarray;

// Instance globale
extern Jarray jarray;

// Prototypes des fonctions
ARRAY_RETURN array_filter(struct Array *self, bool (*predicate)(const void *));
ARRAY_RETURN array_at(struct Array *self, size_t index);
ARRAY_RETURN array_add(struct Array *self, const void * elem);
ARRAY_RETURN array_init(Array *array, size_t elem_size);
ARRAY_RETURN array_init_with_data(Array *array, void *data, size_t length, size_t elem_size);
ARRAY_RETURN array_print(struct Array *array, void (*print_element_callback)(const void *));
void print_array_err(ARRAY_RETURN ret);

#endif
