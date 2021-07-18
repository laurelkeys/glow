#include "dynarray.h"

#include <stdlib.h> // realloc, free

void *arr__growf(void *a, size_t elemsize, size_t addlen, size_t min_cap) {
    if (a != NULL) {
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

        void *b = realloc(arr__header(a), elemsize * min_cap + sizeof(dyn_array_header_t));
        b = (char *) b + sizeof(dyn_array_header_t);
        arr__header(b)->capacity = min_cap;
        // keeps the same length

        return b;
    }

    const size_t min_len = addlen < 4 ? 4 : addlen; // max(addlen, 4)

    if (min_len > min_cap) { min_cap = min_len; }

    void *b = realloc(NULL, elemsize * min_cap + sizeof(dyn_array_header_t));
    b = (char *) b + sizeof(dyn_array_header_t);
    arr__header(b)->capacity = min_cap;
    arr__header(b)->length = 0;

    return b;
}

void arr__freef(void *a) {
    free(arr__header(a));
}
