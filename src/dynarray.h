#pragma once

// NOTE: this is simply a stripped-down version of http://nothings.org/stb_ds (v0.67)

/* clang-format off */

#include <string.h> // size_t, memmove

typedef struct {
    size_t length;
    size_t capacity;
} dyn_array_header_t;

//
// PUBLIC INTERFACE
//

/////// arrfree(T*) -> void;            Frees the array.
#define arrfree(a)                      ((void)((a) ? arr__freef(a) : (void) 0), (a) = NULL)

/////// arrlast(T*) -> T;               Returns the last element in the array.
#define arrlast(a)                      ((a)[arr__header(a)->length - 1])

/////// arrclear(T*) -> void;           Sets the length of the array to 0. Does not change allocated storage.
#define arrclear(a)                     ((a) ? arr__header(a)->length = 0 : 0)

/////// arrlen(T*) -> size_t;           Returns the number of elements in the array.
#define arrlen(a)                       ((a) ? arr__header(a)->length : 0)

/////// arrsetlen(T*, int) -> void;     Changes the length of the array to n. Allocates uninitialized slots at the end if necessary.
#define arrsetlen(a, n)                 (((ptrdiff_t) arrcap(a) < (ptrdiff_t) (n) ? arrsetcap((a), (size_t) (n)), 0 : 0), (a) ? arr__header(a)->length = (size_t) (n) : 0)

/////// arrcap(T*) -> size_t;           Returns the number of total elements the array can contain without needing to be reallocated.
#define arrcap(a)                       ((a) ? arr__header(a)->capacity : 0)

/////// arrsetcap(T*, int) -> void;     Sets the length of allocated storage to at least n. It will not change the length of the array.
#define arrsetcap(a, n)                 (arr__grow(a, 0, n))

/////// arrpush(T*, T) -> T;            Appends the item v to the end of array a. Returns v.
#define arrpush(a, v)                   (arr__maybegrow(a, 1), (a)[arr__header(a)->length++] = (v))

/////// arrpop(T*) -> T;                Removes the final element of the array and returns it.
#define arrpop(a)                       (arr__header(a)->length--, (a)[arr__header(a)->length])

/////// arrdelswap(T*, int) -> void;    Deletes the element at a[i], replacing it with the element from the end of the array. O(1) performance.
#define arrdelswap(a, i)                ((a)[i] = arrlast(a), arr__header(a)->length -= 1)

/*
/////// arrdel(T*, int) -> void;        Deletes the element at a[i], moving the rest of the array over.
#define arrdel(a, i)                    (arrdeln(a, i, 1))

/////// arrdeln(T*, int, int) -> void;  Deletes n elements starting at a[i], moving the rest of the array over.
#define arrdeln(a, i, n)                (memmove(&(a)[i], &(a)[(i) + (n)], sizeof *(a) * (arr__header(a)->length - (n) - (i))), arr__header(a)->length -= (n))

/////// arrins(T*, int, T) -> T;        Inserts the item v into the middle of array a, into a[i], moving the rest of the array over. Returns v.
#define arrins(a, i, v)                 (arrinsn((a), (i), 1), (a)[i] = (v))

/////// arrinsn(T*, int, int) -> void;  Inserts n uninitialized items into array a starting at a[i], moving the rest of the array over.
#define arrinsn(a, i, n)                ((void) (arraddnindex(a, n)), memmove(&(a)[(i) + (n)], &(a)[i], sizeof *(a) * (arr__header(a)->length - (n) - (i))))

/////// arraddnptr(T*, int) -> T*;      Appends n uninitialized items onto array at the end. Returns a pointer to the first uninitialized item added.
#define arraddnptr(a, n)                (arr__maybegrow(a, n), (n) ? (arr__header(a)->length += (n), &(a)[arr__header(a)->length - (n)]) : (a))

/////// arraddnindex(T*, int) -> size_t;Appends n uninitialized items onto array at the end. Returns the index of the first uninitialized item added.
#define arraddnindex(a, n)              (arr__maybegrow(a, n), (n) ? (arr__header(a)->length += (n), arr__header(a)->length - (n)) : arrlen(a))
*/

//
// PRIVATE INTERFACE
//

#define arr__header(t) ((dyn_array_header_t *) (t) -1)
#define arr__grow(a, b, c) ((a) = arr__growf_wrapper((a), sizeof *(a), (b), (c)))
#define arr__maybegrow(a, n) ((!(a) || arr__header(a)->length + (n) > arr__header(a)->capacity) ? (arr__grow(a, n, 0), 0) : 0)

#ifndef __cplusplus

void *arr__growf(void *a, size_t elemsize, size_t addlen, size_t min_cap);
void arr__freef(void *a);

// in C we use implicit assignment from void*-returning functions to T*
#define arr__growf_wrapper arr__growf

#else

extern "C" {
    void *arr__growf(void *a, size_t elemsize, size_t addlen, size_t min_cap);
    void arr__freef(void *a);
}

// in C++ this template makes the same code work
template <typename T>
static T *arr__growf_wrapper(T *a, size_t elemsize, size_t addlen, size_t min_cap) {
    return static_cast<T *>(arr__growf((void *) a, elemsize, addlen, min_cap));
}

#endif
