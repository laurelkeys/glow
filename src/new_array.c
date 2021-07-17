#include "new_array.h"

void *arr__growf(void *a, size_t elemsize, size_t addlen, size_t min_cap) {
    const size_t min_len = arrlen(a) + addlen;

    // compute the minimum capacity needed
    if (min_len > min_cap) { min_cap = min_len; }

    if (min_cap <= arrcap(a)) { return a; }

    // increase needed capacity to guarantee O(1) amortized
    if (min_cap < 2 * arrcap(a)) {
        min_cap = 2 * arrcap(a);
    } else if (min_cap < 4) {
        min_cap = 4;
    }

    void *b = realloc((a) ? arr__header(a) : 0, elemsize * min_cap + sizeof(new_array_header_t));
    b = (char *) b + sizeof(new_array_header_t);
    if (a == NULL) { arr__header(b)->length = 0; }
    arr__header(b)->capacity = min_cap;

    return b;
}

void arr__freef(void *a) {
    free(arr__header(a));
}
