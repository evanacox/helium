#ifndef HE_VM_H
#define HE_VM_H

#include "module.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Simply a failure code if the VM had any issues */
typedef enum he_interpret_flag { INTERPRET_SUCCESS, INTERPRET_FAILURE } he_interpret_flag;

/** @brief Represents a result from running a module */
typedef struct he_interpret_result {
    /** @brief The value on the top of the stack when the module exited */
    he_value *result;

    /** @brief The result code */
    he_interpret_flag flag;
} he_interpret_result;

/** @brief Represents the data stack of the VM */
typedef struct he_stack {
    /** @brief Pointer to the array */
    he_value *stack;

    /** @brief Pointer to the top of the stack */
    he_value *stack_top;

    /** @brief The size of the stack */
    size_t stack_size;

    /** @brief Capacity of the stack */
    size_t stack_capacity;
} he_stack;

/** @brief Represents the return address stack of the VM */
typedef struct he_return_stack {
    /** @brief Pointer to the beginning of the array */
    size_t *stack;

    /** @brief Pointer to the top of the stack */
    size_t *stack_top;

    /** @brief The size of the stack */
    size_t stack_size;

    /** @brief Capacity of the stack */
    size_t stack_capacity;
} he_return_stack;

/** @brief Represents the VM */
typedef struct he_vm {
    /** @brief The data stack */
    he_stack stack;

    /** @brief Return address stack */
    he_return_stack return_addresses;

    /** @brief Index of the current instruction */
    size_t pc;

    /** @brief Pointer to the module being interpreted */
    const he_module *mod;
} he_vm;

/**
 * @brief Initializes a VM instance
 * @param vm The VM to initialize
 */
void he_vm_init(he_vm *vm);

/**
 * @brief Destroys a VM's members
 * @param vm The VM to de-initialize
 */
void he_vm_destroy(he_vm *vm);

/**
 * @brief Runs a module with a VM instance
 * @param vm The VM instance to use
 * @param mod The module to run
 */
void he_vm_run(he_vm *vm, he_module *mod);

#ifdef __cplusplus
}
#endif

#endif
