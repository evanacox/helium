#ifndef HE_MODULE_H
#define HE_MODULE_H

#include "value.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Represents the "constant" section of a module, holding all the known constants */
typedef struct he_const_pool {
    /** @brief Array of he_value objects */
    he_value *pool;

    /** @brief The size of the array */
    size_t pool_size;

    /** @brief The capacity of the array */
    size_t pool_capacity;
} he_const_pool;

typedef struct he_module {
    /** @brief Pointer to the bytecode */
    uint8_t *ops;

    /** @brief Number of bytes in the byte array */
    size_t ops_size;

    /** @brief Max capacity of the byte array */
    size_t ops_capacity;

    /** @brief Pool of constant values */
    he_const_pool pool;
} he_module;

/**
 * @brief Initializes a const_pool
 * @param pool The pool to initialize
 */
void he_const_pool_init(he_const_pool *pool);

/**
 * @brief Destroys a pool's members
 * @param pool The pool to destroy
 */
void he_const_pool_destroy(he_const_pool *pool);

/**
 * @brief Initializes a module
 * @param mod The module to initialize
 */
void he_module_init(he_module *mod);

/**
 * @brief Destroys a module's members
 * @param mod The module to destroy
 */
void he_module_destroy(he_module *mod);

/**
 * @brief Adds a byte to the module
 * @param mod The module to add to
 * @param byte The byte to add
 */
void he_module_write_byte(he_module *mod, uint8_t byte);

/**
 * @brief Adds an 8 byte integer to the byte array
 * @param mod The module to add to
 * @param num The number to write
 */
void he_module_write_int(he_module *mod, size_t num);

/**
 * @brief Adds a constant to the const_pool, and writes an OP_LOAD_CONST
 * @param mod The module to add t o
 * @param val The constant value to add
 */
void he_module_add_constant(he_module *mod, he_value val);

#ifdef __cplusplus
}
#endif

#endif
