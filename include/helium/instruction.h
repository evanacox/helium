#ifndef HE_INSTRUCTION_H
#define HE_INSTRUCTION_H

#include "value.h"
#include <assert.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The opcodes for various instructions */
enum he_opcode {
    /** @brief Gets the front return address and jumps to it */
    OP_RET,

    /** @brief Pushes the next instruction address onto the return addr stack and jumps to a symbol
     */
    OP_CALL,

    /**
     * @brief Reads the next 4 bytes as an index for the constant pool, and
     * pushes the item at that index onto the stack
     */
    OP_LOAD_CONST,

    /** @brief Pops 2 values, adds them and pushes */
    OP_ADD,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_SUB,

    /** @brief Pops 2 values, multiplies them and pushes */
    OP_MUL,

    /** @brief Pops 2 values, divides them and pushes */
    OP_DIV,

    /** @brief Pops 2 values, modulus-es them and pushes */
    OP_MOD,

    /** @brief Pops 2 values, checks if first is greater than second then pushes */
    OP_GT,

    /** @brief Pops 2 values, checks if first is less than second then pushes */
    OP_LT,

    /** @brief Pops 2 values, checks if first is gt/equal to the second then pushes */
    OP_GTEQ,

    /** @brief Pops 2 values, checks if first is lt/equal to second then pushes */
    OP_LTEQ,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_JMP,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_JZ,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_JNZ,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_PUSH,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_POP,
} __attribute__((packed));

_Static_assert(sizeof(enum he_opcode) == sizeof(uint8_t), "op_code should be same size as byte");

/** @brief Represents a call op */
struct he_op_call {
    /** @brief The address / PC of where to return to */
    size_t return_address;
};

/** @brief Represents one of the jump ops (jmp/jz/jnz) */
struct he_op_jmp {
    /** @brief The address to jump to if the condition is met */
    size_t address;
};

/** @brief Represents a push instruction */
struct he_op_push {
    /** @brief The value to push */
    struct he_value val;
};

/** @brief Represents a single instruction */
struct he_op {
    enum he_opcode op;

    union he_op_as {
        struct he_op_call call;
        struct he_op_jmp jmp;
        struct he_op_push push;
    } op_object;
};

#ifdef __cplusplus
}
#endif

#endif