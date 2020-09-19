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

typedef struct he_module {
    /** @brief Vector of opcodes */
    he_vector ops;

    /** @brief Pool of constant values */
    he_vector pool;
} he_module;

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
 * @brief Adds a native-width integer to the byte array
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
