#ifndef HELIUM_VECTOR_H
#define HELIUM_VECTOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Represents a generic "vector" */
typedef struct he_vector {
    /** @brief The number of elements */
    size_t size;

    /** @brief The capacity of the array */
    size_t capacity;

    /** @brief The size of the type contained */
    size_t type_size;

    /** @brief Array pointer */
    uint8_t *array;

    /** @brief Pointer to the last element of the array */
    void *top;
} he_vector;

/**
 * @brief Initializes a vector with a NULL array
 * @param vec Pointer to the vec
 * @param type_size sizeof(T) where he_vector is he_vector<T>
 */
void he_vector_init(he_vector *vec, size_t type_size);

/**
 * @brief Initializes a vector with an initially allocated array
 * @param vec Pointer to the vec
 * @param type_size sizeof(T) where he_vector is he_vector<T>
 * @param capacity The number of T elements to have
 */
void he_vector_init_prealloc(he_vector *vec, size_t type_size, size_t capacity);

/**
 * @brief Resizes the vector's array
 * @param vec Pointer to the vec
 * @param new_capacity The new capacity to have
 */
void he_vector_resize(he_vector *vec);

/**
 * @brief Pushes (copies) an element into the vector
 * @param vec The vector to push into
 * @param to_push Pointer to the object thats being pushed in
 */
void he_vector_push(he_vector *vec, void *to_push);

/**
 * @brief Writes the last element into @p dest, then pops it
 * @param vec The vector to pop from
 * @param dest Pointer to the location to put the popped element
 */
void he_vector_pop(he_vector *vec, void *dest);

/**
 * @brief Returns a pointer to the very last element
 * @param vec The vector to get the last element from
 * @return A pointer to the last element
 */
void *he_vector_last(const he_vector *vec);

/**
 * @brief Effectively an operator[] for he_vector
 * @param vec The vector to get an element from
 * @param idx The index to get a pointer to
 * @return A pointer to vec.array + idx for the type
 */
void *he_vector_at(const he_vector *vec, size_t idx);

/**
 * @brief Frees the vector's array and sets it to a pre-defined state
 * @param vec The vec to destroy
 */
void he_vector_destroy(he_vector *vec);

#define he_vector_push_val(vec, val) he_vector_push(vec, &val);

#ifdef __cplusplus
#define he_vector_push_rval(vec, rval)                                                             \
    do {                                                                                           \
        auto temp = rval;                                                                          \
        he_vector_push(vec, &temp);                                                                \
    } while (false)
#else
#define he_vector_push_rval(vec, rval)                                                             \
    do {                                                                                           \
        __auto_type temp = rval;                                                                   \
        he_vector_push(vec, &temp);                                                                \
    } while (false)
#endif

#ifdef __cplusplus
}
#endif

#endif
