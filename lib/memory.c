#include "helium/memory.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static size_t expand_capacity(size_t current) {
    if (current == 0) { return 8; }

    return current * 2;
}

void *he_alloc(size_t sizeof_type, size_t length) {
    assert(length != 0 && "attempting to allocate array of 0 length");

    return malloc(sizeof_type * length);
}

void *he_grow_array(void *array_ptr, size_t type_size, size_t *current_length) {
    *current_length = expand_capacity(*current_length);

    void *ptr = realloc(array_ptr, type_size * (*current_length));

    if (!ptr) {
        fprintf(stderr, "he_grow_array: unable to allocate memory!\n");
        exit(-1);
    }

    return ptr;
}

void he_free_array(void *array) {
    free(array);
}
