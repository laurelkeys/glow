#pragma once

// NOTE: this is simply a stripped-down version of http://nothings.org/stb_ds (v0.67)

// clang-format off
#include <stdlib.h> // realloc, free
#include <string.h> // memmove

typedef struct {
    size_t length;
    size_t capacity;
} new_array_header_t;

//
// PUBLIC INTERFACE
//

#define arrfree(a)          ((void)((a) ? arr__freef(a) : (void) 0), (a) = NULL)

#define arrlast(a)          ((a)[arr__header(a)->length - 1])

#define arrlen(a)           ((a) ? (ptrdiff_t) arr__header(a)->length : 0)
#define arrlenu(a)          ((a) ? arr__header(a)->length : 0)
#define arrsetlen(a, n)     ((arrcap(a) < (size_t) (n) ? arrsetcap((a), (size_t) (n)), 0 : 0), (a) ? arr__header(a)->length = (size_t) (n) : 0)

#define arrcap(a)           ((a) ? arr__header(a)->capacity : 0)
#define arrsetcap(a, n)     (arr__grow(a, 0, n))

#define arrpush(a, v)       (arr__maybegrow(a, 1), (a)[arr__header(a)->length++] = (v))
#define arrpop(a)           (arr__header(a)->length--, (a)[arr__header(a)->length])

/*
#define arraddnptr(a, n)    (arr__maybegrow(a, n), (n) ? (arr__header(a)->length += (n), &(a)[arr__header(a)->length - (n)]) : (a))
#define arraddnindex(a, n)  (arr__maybegrow(a, n), (n) ? (arr__header(a)->length += (n), arr__header(a)->length - (n)) : arrlen(a))

#define arrins(a, i, v)     (arrinsn((a), (i), 1), (a)[i] = (v))
#define arrinsn(a, i, n)    ((void) (arraddnindex(a, n)), memmove(&(a)[(i) + (n)], &(a)[i], sizeof *(a) * (arr__header(a)->length - (n) - (i))))

#define arrdel(a, i)        (arrdeln(a, i, 1))
#define arrdeln(a, i, n)    (memmove(&(a)[i], &(a)[(i) + (n)], sizeof *(a) * (arr__header(a)->length - (n) - (i))), arr__header(a)->length -= (n))
#define arrdelswap(a, i)    ((a)[i] = arrlast(a), arr__header(a)->length -= 1)
*/

//
// PRIVATE INTERFACE
//

#define arr__header(t) ((new_array_header_t *) (t) -1)
#define arr__grow(a, b, c) ((a) = arr__growf_wrapper((a), sizeof *(a), (b), (c)))
#define arr__maybegrow(a, n) ((!(a) || arr__header(a)->length + (n) > arr__header(a)->capacity) ? (arr__grow(a, n, 0), 0) : 0)

#ifndef __cplusplus

void *arr__growf(void *a, size_t elemsize, size_t addlen, size_t min_cap);
void arr__freef(void *a);

// in C we use implicit assignment from void*-returning functions to T*
#define arr__growf_wrapper arr__growf

#else

extern "C"
{
    void *arr__growf(void *a, size_t elemsize, size_t addlen, size_t min_cap);
    void arr__freef(void *a);
}

// in C++ this template makes the same code work
template <typename T>
static T *arr__growf_wrapper(T *a, size_t elemsize, size_t addlen, size_t min_cap)
{
    return static_cast<T *>(arr__growf((void *) a, elemsize, addlen, min_cap));
}

#endif

//
// DOCUMENTATION
//

/* Non-function interface:

    Declare an empty dynamic array of type T
      T* foo = NULL;

    Access the i'th item of a dynamic array 'foo' of type T, T* foo:
      foo[i]
*/

/* Functions (actually macros):

    void arrfree(T* a);
        Frees the array.

    T arrlast(T* a);
        Returns the last element in the array.

    ptrdiff_t arrlen(T* a);
        Returns the number of elements in the array.

    size_t arrlenu(T* a);
        Returns the number of elements in the array as an unsigned type.

    void arrsetlen(T* a, int n);
        Changes the length of the array to n. Allocates uninitialized slots at the end if necessary.

    size_t arrcap(T* a);
        Returns the number of total elements the array can contain without needing to be reallocated.

    size_t arrsetcap(T* a, int n);
        Sets the length of allocated storage to at least n. It will not change the length of the array.

    T arrpush(T* a, T v);
        Appends the item v to the end of array a. Returns v.

    T arrpop(T* a);
        Removes the final element of the array and returns it.

    T* arraddnptr(T* a, int n);
        Appends n uninitialized items onto array at the end. Returns a pointer to the first uninitialized item added.

    size_t arraddnindex(T* a, int n);
        Appends n uninitialized items onto array at the end. Returns the index of the first uninitialized item added.

    T arrins(T* a, int i, T v);
        Inserts the item v into the middle of array a, into a[i], moving the rest of the array over. Returns v.

    void arrinsn(T* a, int i, int n);
        Inserts n uninitialized items into array a starting at a[i], moving the rest of the array over.

    void arrdel(T* a, int i);
        Deletes the element at a[i], moving the rest of the array over.

    void arrdeln(T* a, int i, int n);
        Deletes n elements starting at a[i], moving the rest of the array over.

    void arrdelswap(T* a, int i);
        Deletes the element at a[i], replacing it with the element from the end of the array. O(1) performance.
*/
