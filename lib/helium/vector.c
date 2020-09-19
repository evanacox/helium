#include "helium/vector.h"
#include "helium/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void he_vector_resize(he_vector *vec) {
    void *ptr = he_grow_array(vec->array, vec->type_size, &vec->capacity);

    if (!ptr) {
        fprintf(stderr, "he_vector_resize: Unable to expand array!\n");
        return;
    }

    vec->array = ptr;
}

void he_vector_init(he_vector *vec, size_t type_size) {
    vec->array = NULL;
    vec->capacity = 0;
    vec->size = 0;
    vec->type_size = type_size;
}

void he_vector_init_prealloc(he_vector *vec, size_t type_size, size_t capacity) {
    he_vector_init(vec, type_size);

    void *ptr = he_alloc(vec->type_size, capacity);

    if (!ptr) {
        fprintf(stderr, "he_vector_init_prealloc: Unable to allocate array!\n");
        return;
    }

    vec->array = ptr;
    vec->capacity = capacity;
}

void he_vector_destroy(he_vector *vec) {
    he_free_array(vec->array);

    he_vector_init(vec, vec->type_size);
}

void he_vector_pop(he_vector *vec, void *dest) {
    assert(vec->size != 0 && "attempting to pop from empty vector");
    memcpy(dest, vec->array + ((vec->size - 1) * vec->type_size), vec->type_size);
    --vec->size;
}

void he_vector_push(he_vector *vec, void *to_push) {
    if (vec->size == vec->capacity) {
        // size gets expanded by x2
        he_vector_resize(vec);
    }

    memcpy(vec->array + (vec->size * vec->type_size), to_push, vec->type_size);
    ++vec->size;
}

void *he_vector_last(const he_vector *vec) {
    assert(vec->size != 0 && "attempting to get last from empty vector");

    return vec->array + ((vec->size - 1) * vec->type_size);
}

void *he_vector_at(const he_vector *vec, size_t idx) {
    assert(idx < vec->size && "attempting to get element past end of vector");

    return vec->array + (idx * vec->type_size);
}
