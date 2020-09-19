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
    /** @brief Vector representing the stack */
    he_vector vec;

    /** @brief Pointer to the top of the stack */
    he_value *top;
} he_stack;

/** @brief Represents the return address stack of the VM */
typedef struct he_return_stack {
    /** @brief Vector representing the stack */
    he_vector vec;

    /** @brief Pointer to the top of the stack */
    size_t *top;
} he_return_stack;

/** @brief Represents the VM */
typedef struct he_vm {
    /** @brief The data stack */
    he_stack stack;

    /** @brief Return address stack */
    he_return_stack ret_addrs;

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
 * @brief Sets up a VM instance to use a certain mod
 * @param vm The VM to give the module to
 * @param mod The module to give to a VM
 */
void he_vm_use(he_vm *vm, const he_module *mod);

/**
 * @brief Executes a single instruction on the VM using the vm's module
 * @param vm The vm to execute with
 * @param has_setjmp_env Whether or not a jmp_buf env already exists for the function to longjmp to
 */
he_interpret_flag he_vm_execute_instruction(he_vm *vm, bool has_setjmp_env);

/**
 * @brief Runs a module with a VM instance
 * @param vm The VM instance to use
 * @param mod The module to run
 */
he_interpret_flag he_vm_run(he_vm *vm, const he_module *mod);

#ifdef __cplusplus
}
#endif

#endif
