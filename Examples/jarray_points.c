#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/jarray.h"

typedef struct {
    int x, y;
} Point;

// --- Helpers ---

// Print a Point
void print_point(const void *p) {
    Point pt = JARRAY_GET_VALUE(const Point, p);
    printf("(%d,%d) ", pt.x, pt.y);
}

// Convert Point -> string "(x,y)"
char *point_to_string(const void *p) {
    Point pt = JARRAY_GET_VALUE(const Point, p);
    char *str = malloc(32);
    if (str) snprintf(str, 32, "(%d,%d)", pt.x, pt.y);
    return str;
}

// Compare points lexicographically by x then y
int compare_point(const void *a, const void *b) {
    Point pa = JARRAY_GET_VALUE(const Point, a);
    Point pb = JARRAY_GET_VALUE(const Point, b);
    if (pa.x != pb.x) return pa.x - pb.x;
    return pa.y - pb.y;
}

// Equality check
bool is_equal_point(const void *a, const void *b) {
    Point pa = JARRAY_GET_VALUE(const Point, a);
    Point pb = JARRAY_GET_VALUE(const Point, b);
    return pa.x == pb.x && pa.y == pb.y;
}

// Predicate: keep only points with both coords even
bool is_even_point(const void *p, const void *ctx) {
    (void)ctx;
    Point pt = JARRAY_GET_VALUE(const Point, p);
    return (pt.x % 2 == 0) && (pt.y % 2 == 0);
}

// Transformation: modulo 3 coords
void modulo3_point(void *p, void *ctx) {
    (void)ctx;
    Point *pt = (Point*)p;
    pt->x %= 3;
    pt->y %= 3;
}

// Predicate: x > threshold
bool x_greater_than(const void *p, const void *ctx) {
    int threshold = JARRAY_GET_VALUE(const int, ctx);
    Point pt = JARRAY_GET_VALUE(const Point, p);
    return pt.x > threshold;
}

// Reduce: sum of coords
void *sum_points(const void *acc, const void *elem, const void *ctx) {
    (void)ctx;
    Point a = JARRAY_GET_VALUE(const Point, acc);
    Point b = JARRAY_GET_VALUE(const Point, elem);
    Point *res = malloc(sizeof(Point));
    res->x = a.x + b.x;
    res->y = a.y + b.y;
    return res;
}

// --- MAIN DEMO ---
int main(void) {
    JARRAY points;

    printf("\n=== DEMO: jarray<Point> ===\n");

    // --- Init with data ---
    Point data_start[5] = {{2,4}, {5,10}, {3,6}, {1,2}, {4,8}};

    JARRAY_USER_CALLBACK_IMPLEMENTATION imp;

    imp.print_element_callback = print_point;
    imp.element_to_string = point_to_string;
    imp.compare = compare_point;
    imp.is_equal = is_equal_point;

    jarray.init_with_data_copy(&points, data_start, 5, sizeof(Point), imp);
    if (JARRAY_CHECK_RET) return EXIT_FAILURE;


    printf("Initial array:\n");
    jarray.print(&points);
    JARRAY_CHECK_RET;

    // --- Insert ---
    Point p0 = {0,0}, pX = {9,9};
    jarray.add_at(&points, 0, &p0);
    JARRAY_CHECK_RET;
    jarray.add(&points, &pX);
    JARRAY_CHECK_RET;

    printf("\nAfter insertions:\n");
    jarray.print(&points);
    JARRAY_CHECK_RET;

    // --- Filter ---
    printf("\nFiltering only even points:\n");
    JARRAY evens = jarray.filter(&points, is_even_point, NULL);
    JARRAY_CHECK_RET;
    jarray.print(&evens);
    JARRAY_CHECK_RET;
    jarray.free(&evens);

    // --- Sort ---
    printf("\nSorting points:\n");
    jarray.sort(&points, QSORT, NULL);
    jarray.print(&points);

    // --- Access ---
    printf("\nPoint at index 2: ");
    Point p = *(Point*)jarray.at(&points, 2);
    JARRAY_CHECK_RET;
    printf("(%d,%d)\n", p.x, p.y);

    // --- Find ---
    printf("\nFind first with x > 3:\n");
    int threshold = 3;
    Point found = *(Point*)jarray.find_first(&points, x_greater_than, &threshold);
    JARRAY_CHECK_RET;
    printf("(%d,%d)\n", found.x, found.y);

    // --- Subarray ---
    printf("\nSubarray [1..3]:\n");
    JARRAY sub = jarray.subarray(&points, 1, 3);
    JARRAY_CHECK_RET;
    jarray.print(&sub);
    jarray.free(&sub);

    // --- Modify ---
    printf("\nSet index 1 to (7,7):\n");
    Point p7 = {7,7};
    jarray.set(&points, 1, &p7);
    JARRAY_CHECK_RET;
    jarray.print(&points);

    // --- For each ---
    printf("\nModulo 3 on each point:\n");
    jarray.for_each(&points, modulo3_point, NULL);
    jarray.print(&points);

    // --- Clone ---
    printf("\nCloning array:\n");
    JARRAY clone = jarray.clone(&points);
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Join ---
    printf("\nJoining points with ', ':\n");
    char *joined = jarray.join(&clone, ", ");
    JARRAY_CHECK_RET;
    printf("%s\n", joined);

    // --- Reduce ---
    printf("\nReducing (sum of coords):\n");
    Point total = *(Point*)jarray.reduce(&clone, sum_points, NULL, NULL);
    JARRAY_CHECK_RET;
    printf("Sum = (%d,%d)\n", total.x, total.y);

    // --- Splice ---
    printf("\nSplicing clone at index 1 (replace 2 elems by (10,10)):\n");
    Point p10 = {10,10};
    jarray.splice(&clone, 1, 2, &p10, NULL);
    JARRAY_CHECK_RET;
    jarray.print(&clone);

    // --- Cleanup ---
    jarray.free(&points);
    jarray.free(&clone);

    printf("\n=== END DEMO ===\n");
    return EXIT_SUCCESS;
}
