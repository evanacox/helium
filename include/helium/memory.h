#ifndef HE_MEMORY_H
#define HE_MEMORY_H

#include <stddef.h>

/**
 * @brief Allocates an array of @p sizeof_type bytes * @p length
 * @param sizeof_type The size of the type being allocated
 * @param length The number of elements in the array
 * @return A pointer to the space
 */
void *he_alloc(size_t sizeof_type, size_t length);

/**
 * @brief Doubles an array's size, or if the array is 0 long makes it 8 long
 * @param array_ptr Pointer to realloc
 * @param type_size Size of the type of the array
 * @param current_length Current capacity of the array, modified by the function
 * @return Pointer to the new array
 */
void *he_grow_array(void *array_ptr, size_t type_size, size_t *current_length);

/**
 * @brief Frees a dynamic array
 * @param array The array to free
 */
void he_free_array(void *array);

#endif
