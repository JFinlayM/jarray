/*
 * Basic tests for jarray (no test library, only assertions).
 * Build and run with `make -C tests` (uses AddressSanitizer + UBSan: a leak or an invalid access fails the run).
 */
#include "../inc/jarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { failures++; fprintf(stderr, "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

#define CHECK_NO_ERROR() CHECK(!jarray_last_error.has_error)
#define CHECK_ERROR(code) CHECK(jarray_last_error.has_error && jarray_last_error.error_code == (code))

#define RUN(test) do { \
    int before = failures; \
    test(); \
    printf("%-45s %s\n", #test, failures == before ? "ok" : "FAILED"); \
} while (0)

typedef struct { int x, y; } Point;

static bool is_even(const void *elem, const void *ctx) { (void)ctx; return *(const int*)elem % 2 == 0; }
static bool always(const void *elem, const void *ctx) { (void)elem; (void)ctx; return true; }

static int int_at(JARRAY *a, size_t i) { return *(int*)jarray.at(a, i); }
static const char *str_at(JARRAY *a, size_t i) { return *(char**)jarray.at(a, i); }

/* ---------- int preset ---------- */

static void test_int_basic(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    CHECK_NO_ERROR();
    jarray_add(&a, 3); jarray_add(&a, 1); jarray_add(&a, 2);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 3);
    CHECK(int_at(&a, 0) == 3 && int_at(&a, 2) == 2);
    jarray_add_at(&a, 1, 9);
    CHECK(int_at(&a, 1) == 9 && jarray.length(&a) == 4);
    jarray_set(&a, 0, 7);
    CHECK(int_at(&a, 0) == 7);
    jarray_remove_at(&a, 1);
    CHECK(jarray.length(&a) == 3 && int_at(&a, 1) == 1);
    jarray_remove(&a);
    CHECK(jarray.length(&a) == 2);
    CHECK(jarray_contains(&a, 7));
    CHECK(!jarray_contains(&a, 123));
    jarray_free(&a);
}

static void test_int_sorts(void) {
    JARRAY_SORT_METHOD methods[] = { JARRAY_QSORT, JARRAY_BUBBLE_SORT, JARRAY_INSERTION_SORT, JARRAY_SELECTION_SORT };
    for (size_t m = 0; m < sizeof(methods) / sizeof(methods[0]); m++) {
        JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
        jarray_addm(&a, 5, 2, 9, 1, 5, 0);
        jarray_sort(&a, methods[m], NULL);
        CHECK_NO_ERROR();
        for (size_t i = 1; i < jarray.length(&a); i++) CHECK(int_at(&a, i - 1) <= int_at(&a, i));
        jarray_reverse(&a);
        CHECK(int_at(&a, 0) == 9 && int_at(&a, 5) == 0);
        jarray_free(&a);
    }
}

static void test_int_filter_subarray_concat_clone(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_addm(&a, 1, 2, 3, 4, 5, 6);

    JARRAY evens = jarray_filter(&a, is_even, NULL);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&evens) == 3 && int_at(&evens, 0) == 2 && int_at(&evens, 2) == 6);

    JARRAY sub = jarray_subarray(&a, 1, 3);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&sub) == 3 && int_at(&sub, 0) == 2 && int_at(&sub, 2) == 4);

    JARRAY both = jarray_concat(&a, &evens);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&both) == 9);
    for (size_t i = 0; i < 6; i++) CHECK(int_at(&both, i) == (int)i + 1);   /* concat must copy ALL of the first array */
    CHECK(int_at(&both, 6) == 2 && int_at(&both, 8) == 6);

    JARRAY copy = jarray_clone(&a);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&copy) == 6);
    jarray_set(&copy, 0, 100);
    CHECK(int_at(&a, 0) == 1);   /* independent data */

    jarray_free(&a); jarray_free(&evens); jarray_free(&sub); jarray_free(&both); jarray_free(&copy);
}

static void test_int_reduce_join_find(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_addm(&a, 1, 2, 3, 4);

    char *joined = jarray_join(&a, ", ");
    CHECK_NO_ERROR();
    CHECK(joined && strcmp(joined, "1, 2, 3, 4") == 0);
    free(joined);

    CHECK(jarray_any(&a, is_even, NULL));
    int *first = jarray_find_first(&a, is_even, NULL);
    int *last = jarray_find_last(&a, is_even, NULL);
    CHECK(first && *first == 2 && last && *last == 4);
    CHECK(jarray_find_first_index(&a, is_even, NULL) == 1);
    CHECK(jarray_find_last_index(&a, is_even, NULL) == 3);

    int *copy = jarray_copy_data(&a);
    CHECK(copy && copy[0] == 1 && copy[3] == 4);
    free(copy);
    jarray_free(&a);
}

static void test_indexes_of_all_match(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_addm(&a, 4, 4, 4, 4);
    JARRAY_INDEXES r = jarray_indexes_of(&a, 4);    /* used to overflow the buffer when every element matches */
    CHECK_NO_ERROR();
    CHECK(r.indexes && r.count == 4 && r.indexes[0] == 0 && r.indexes[3] == 3);
    free(r.indexes);
    r = jarray_indexes_of(&a, 5);
    CHECK_ERROR(JARRAY_ELEMENT_NOT_FOUND);
    CHECK(r.indexes == NULL && r.count == 0);
    jarray_free(&a);
}

static void test_add_after_clear_and_reserve(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_add(&a, 1);
    jarray_clear(&a);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 0);
    jarray_add(&a, 2);                           /* used to write to a NULL buffer */
    CHECK_NO_ERROR();
    CHECK(int_at(&a, 0) == 2);

    jarray_reserve(&a, 100);
    CHECK_NO_ERROR();
    CHECK(a._capacity >= 100);
    for (int i = 0; i < 1000; i++) jarray_add(&a, i);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 1001);
    while (jarray.length(&a) > 1) jarray_remove(&a);
    CHECK(jarray.length(&a) == 1);
    jarray_free(&a);
}

static void test_remove_all(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_addm(&a, 1, 2, 2, 3, 2, 4);
    int to_remove[] = { 2, 4, 99 };
    jarray_remove_all(&a, to_remove, 3);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 2 && int_at(&a, 0) == 1 && int_at(&a, 1) == 3);
    jarray_free(&a);

    JARRAY s = jarray_init_preset(JARRAY_STRING_PRESET);
    jarray_addm(&s, "a", "b", "a", "c");
    char *rm[] = { "a" };
    jarray_remove_all(&s, rm, 1);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&s) == 2 && strcmp(str_at(&s, 0), "b") == 0 && strcmp(str_at(&s, 1), "c") == 0);
    jarray_free(&s);
}

static void test_fill_and_shift_right(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_addm(&a, 1, 2, 3);
    jarray_fill(&a, 1, 5, 8);                    /* extends the array */
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 6 && int_at(&a, 0) == 1 && int_at(&a, 1) == 8 && int_at(&a, 5) == 8);
    jarray_shift_right(&a, 0);
    CHECK(jarray.length(&a) == 7 && int_at(&a, 0) == 0 && int_at(&a, 1) == 1);
    jarray_shift(&a);
    CHECK(jarray.length(&a) == 6 && int_at(&a, 0) == 1);
    jarray_splice(&a, 1, 2, 40, 41, 42);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 7 && int_at(&a, 1) == 40 && int_at(&a, 3) == 42);
    jarray_free(&a);
}

/* ---------- string preset (pointer array: ownership matters, run under ASan) ---------- */

static void test_string_ownership(void) {
    JARRAY a = jarray_init_preset(JARRAY_STRING_PRESET);
    char buf[8] = "buf";
    const char *cs = "cst";
    char *heap = malloc(5); memcpy(heap, "heap", 5);

    jarray_add(&a, "lit");                       /* string literal, char[], const char*, char* */
    jarray_add(&a, buf);
    jarray_add(&a, cs);
    jarray_add(&a, heap);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&a) == 4);
    CHECK(strcmp(str_at(&a, 0), "lit") == 0 && strcmp(str_at(&a, 1), "buf") == 0);
    CHECK(strcmp(str_at(&a, 3), "heap") == 0);
    CHECK(str_at(&a, 3) != heap);                /* the array owns a copy */
    free(heap);

    jarray_addm(&a, "m1", "m2");
    jarray_add_at(&a, 1, "ins");
    jarray_set(&a, 0, "set");
    jarray_splice(&a, 2, 1);                     /* remove only */
    jarray_splice(&a, 0, 1, "s1", "s2");         /* remove and insert */
    jarray_fill(&a, 1, 9, "fill");
    jarray_shift_right(&a, "sr");
    jarray_shift(&a);
    jarray_remove(&a);
    jarray_remove_at(&a, 0);
    CHECK_NO_ERROR();
    CHECK(jarray_contains(&a, "fill"));

    jarray_sort(&a, JARRAY_QSORT, NULL);
    jarray_sort(&a, JARRAY_BUBBLE_SORT, NULL);
    jarray_sort(&a, JARRAY_INSERTION_SORT, NULL);
    jarray_sort(&a, JARRAY_SELECTION_SORT, NULL);
    jarray_reverse(&a);
    CHECK_NO_ERROR();

    char *joined = jarray_join(&a, "|");
    CHECK(joined != NULL);
    free(joined);

    jarray_clear(&a);
    jarray_add(&a, "x");
    CHECK(strcmp(str_at(&a, 0), "x") == 0);
    jarray_free(&a);
}

static void test_string_derived_arrays(void) {
    JARRAY a = jarray_init_preset(JARRAY_STRING_PRESET);
    jarray_reserve(&a, 10);                      /* clone used to read the reserved (uninitialised) slots */
    jarray_addm(&a, "a", "bb", "ccc");

    JARRAY copy = jarray_clone(&a);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&copy) == 3 && strcmp(str_at(&copy, 2), "ccc") == 0);
    CHECK(str_at(&copy, 0) != str_at(&a, 0));    /* deep copy */

    JARRAY both = jarray_concat(&a, &copy);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&both) == 6);
    for (size_t i = 0; i < 3; i++) CHECK(strcmp(str_at(&both, i), str_at(&a, i)) == 0);

    JARRAY sub = jarray_subarray(&both, 1, 4);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&sub) == 4);

    JARRAY all = jarray_filter(&a, always, NULL);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&all) == 3);

    char **data = jarray_copy_data(&a);          /* raw copy: the caller frees each string and the buffer */
    CHECK(data && strcmp(data[1], "bb") == 0);
    for (size_t i = 0; i < jarray.length(&a); i++) free(data[i]);
    free(data);

    jarray_free(&a); jarray_free(&copy); jarray_free(&both); jarray_free(&sub); jarray_free(&all);
}

/* ---------- custom type (value array without preset) ---------- */

static void test_custom_struct(void) {
    JARRAY a;
    JARRAY_USER_CALLBACK_IMPLEMENTATION imp = {0};
    jarray_init(&a, sizeof(Point), JARRAY_TYPE_VALUE, imp);
    CHECK_NO_ERROR();
    Point p = {1, 2};
    jarray_add(&a, p);
    jarray_add(&a, ((Point){3, 4}));             /* a compound literal needs extra parentheses in a macro */
    CHECK(jarray.length(&a) == 2);
    CHECK(((Point*)jarray.at(&a, 1))->y == 4);
    jarray_free(&a);
}

/* ---------- errors, NULL arguments ---------- */

static void test_null_arguments(void) {
    JARRAY c = jarray.clone(NULL);               /* used to dereference NULL */
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(c._data == NULL && c._length == 0);
    jarray.free(&c);

    JARRAY d = jarray.concat(NULL, NULL);
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(d._data == NULL);

    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_add(&a, 1);
    JARRAY e = jarray.concat(&a, NULL);
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(e._data == NULL);                      /* must not share a's data */

    JARRAY f = jarray.subarray(NULL, 0, 1);
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(f._data == NULL);
    JARRAY g = jarray.filter(NULL, NULL, NULL);
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(g._data == NULL);

    jarray.add(NULL, NULL);
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    jarray.print_jarray_err(__FILE__, __LINE__); /* printing an error raised on NULL must not crash */
    jarray_free(&a);
}

static void test_macro_size_mismatch(void) {
    JARRAY f = jarray_init_preset(JARRAY_FLOAT_PRESET);
    jarray_add(&f, 1.5);                         /* a double (8 bytes) in a float (4 bytes) array */
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(jarray.length(&f) == 0);               /* nothing was added */
    jarray_add(&f, 1.5f);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&f) == 1);
    jarray_addm(&f, 2.5f, 3.5);                  /* the second value is a double: nothing is added */
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    CHECK(jarray.length(&f) == 1);
    jarray_addm(&f, 2.5f, 3.5f);
    CHECK_NO_ERROR();
    CHECK(jarray.length(&f) == 3);
    CHECK(!jarray_contains(&f, 9.0));            /* mismatch: reports an error, returns false */
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    jarray_free(&f);

    JARRAY c = jarray_init_preset(JARRAY_CHAR_PRESET);
    jarray_add(&c, 'x');                         /* 'x' is an int in C */
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    jarray_add(&c, (char)'x');
    CHECK_NO_ERROR();
    jarray_free(&c);

    JARRAY s = jarray_init_preset(JARRAY_STRING_PRESET);
    jarray_add(&s, "literal");                   /* sizeof the pointer, not of the array */
    CHECK_NO_ERROR();
    jarray_add(&s, 5);
    CHECK_ERROR(JARRAY_INVALID_ARGUMENT);
    jarray_free(&s);
}

static void test_errors_and_reset(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_at(&a, 0);
    CHECK(jarray_last_error.has_error);           /* empty array or out of bound */
    jarray_add(&a, 1);
    CHECK_NO_ERROR();                            /* a successful call resets the trace */
    jarray_at(&a, 5);
    CHECK_ERROR(JARRAY_INDEX_OUT_OF_BOUND);
    jarray_remove_at(&a, 5);
    CHECK_ERROR(JARRAY_INDEX_OUT_OF_BOUND);
    jarray_free(&a);
}

static void test_huge_sizes(void) {
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    jarray_add(&a, 1);
    jarray.reserve(&a, (size_t)-1 / 2);
    CHECK(jarray_last_error.has_error);
    int x = 1;
    jarray.add_all(&a, &x, (size_t)-1);
    CHECK(jarray_last_error.has_error);
    jarray.fill(&a, &x, 0, (size_t)-1);
    CHECK(jarray_last_error.has_error);
    CHECK(jarray.length(&a) == 1 && int_at(&a, 0) == 1);   /* untouched after the failures */
    jarray_free(&a);
}

/* ---------- threads: the error trace is per thread ---------- */

static void *worker(void *arg) {
    int *bad = arg;
    JARRAY a = jarray_init_preset(JARRAY_INT_PRESET);
    for (int i = 0; i < 20000; i++) {
        jarray_add(&a, i);
        if (jarray_last_error.has_error) (*bad)++;       /* another thread's error leaked here */
        jarray_at(&a, 5000000);
        if (!jarray_last_error.has_error) (*bad)++;      /* our own error was lost */
    }
    jarray_free(&a);
    return NULL;
}

static void test_error_trace_is_thread_local(void) {
    pthread_t t[4];
    int bad[4] = {0};
    for (int i = 0; i < 4; i++) pthread_create(&t[i], NULL, worker, &bad[i]);
    for (int i = 0; i < 4; i++) pthread_join(t[i], NULL);
    for (int i = 0; i < 4; i++) CHECK(bad[i] == 0);
}

int main(void) {
    RUN(test_int_basic);
    RUN(test_int_sorts);
    RUN(test_int_filter_subarray_concat_clone);
    RUN(test_int_reduce_join_find);
    RUN(test_indexes_of_all_match);
    RUN(test_add_after_clear_and_reserve);
    RUN(test_remove_all);
    RUN(test_fill_and_shift_right);
    RUN(test_string_ownership);
    RUN(test_string_derived_arrays);
    RUN(test_custom_struct);
    RUN(test_null_arguments);
    RUN(test_macro_size_mismatch);
    RUN(test_errors_and_reset);
    RUN(test_huge_sizes);
    RUN(test_error_trace_is_thread_local);

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
