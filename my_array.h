#ifndef MY_ARRAY_H
#define MY_ARRAY_H

#include <stdbool.h>
#include <stddef.h>

// Codes d'erreur
typedef enum {
    INDEX_OUT_OF_BOUND = 0,
    ARRAY_UNINITIALIZED,
    DATA_NULL,
    PRINT_ELEMENT_CALLBACK_UNINTIALIZED,
    EMPTY_ARRAY
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


typedef struct USER_FUNCTION_IMPLEMENTATION {
    void (*print_element_callback)(const void*);
    int (*compare)(const void*, const void*);
}USER_FUNCTION_IMPLEMENTATION;

// Structure principale Array
typedef struct Array {
    void *data;
    size_t length;
    size_t elem_size;
    Array_state_t state;
    USER_FUNCTION_IMPLEMENTATION user_implementation;
} Array;


typedef enum SORT_METHOD {
    QSORT = 0,
    BUBBLE_SORT,
    INSERTION_SORT,
    SELECTION_SORT,
} SORT_METHOD;


// Interface "statique" pour méthodes
typedef struct Jarray {
    ARRAY_RETURN (*filter)(struct Array *self, bool (*predicate)(const void *));
    ARRAY_RETURN (*at)(struct Array *self, size_t index);
    ARRAY_RETURN (*add)(struct Array *self, const void * elem);
    ARRAY_RETURN (*remove)(struct Array *self);
    ARRAY_RETURN (*remove_at)(struct Array *self, size_t index);
    ARRAY_RETURN (*add_at)(struct Array *self, size_t index, const void * elem);
    ARRAY_RETURN (*init)(struct Array *array, size_t elem_size);
    ARRAY_RETURN (*init_with_data)(struct Array *array, void *data, size_t length, size_t elem_size);
    ARRAY_RETURN (*print)(struct Array *self);
    ARRAY_RETURN (*sort)(struct Array *self, SORT_METHOD method);
    void (*print_array_err)(ARRAY_RETURN ret);
} Jarray;

// Instance globale
extern Jarray jarray;

// Prototypes des fonctions
ARRAY_RETURN array_filter(struct Array *self, bool (*predicate)(const void *));
ARRAY_RETURN array_at(struct Array *self, size_t index);
ARRAY_RETURN array_add(struct Array *self, const void * elem);
ARRAY_RETURN array_remove(struct Array *self);
ARRAY_RETURN array_remove_at(struct Array *self, size_t index);
ARRAY_RETURN array_add_at(struct Array *self, size_t index, const void * elem);
ARRAY_RETURN array_init(Array *array, size_t elem_size);
ARRAY_RETURN array_init_with_data(Array *array, void *data, size_t length, size_t elem_size);
ARRAY_RETURN array_print(struct Array *array);
ARRAY_RETURN array_sort(struct Array *self, SORT_METHOD method);
void print_array_err(ARRAY_RETURN ret);

#define GET_VALUE(type, array_return) (*(type*)(array_return).value)
#define GET_POINTER(type, array_return) (type*)(array_return.value)
#define TO_POINTER(type, value) (&(type){value})
#define GET_VALUE_SAFE(type, array_return, default_value) \
    ((array_return).has_value == true ? *(type*)((array_return).value) : (default_value))
#define CHECK_RET(ret)      if (!ret.has_value) {                       \
                                jarray.print_array_err(ret);            \
                                return;                                 \
                            }


#endif
