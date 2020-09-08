#ifndef HE_INSTRUCTION_H
#define HE_INSTRUCTION_H

#include "value.h"
#include <assert.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The opcodes for various instructions */
typedef enum he_opcode {
    /** @brief Gets the front return address and jumps to it */
    OP_RET = 0,

    /**
     * @brief Pushes the next address onto the return addr stack and
     * reads next 8 bytes as an address and jumps to it
     */
    OP_CALL,

    /**
     * @brief Reads the next 8 bytes as an index for the constant pool, and
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

    /** @brief Pops 2 values, returns if they're equal */
    OP_EQ,

    /** @brief Pops 1 value, returns !value */
    OP_NOT,

    /** @brief Pops 1 value, negates it. Returns -value */
    OP_NEGATE,

    /** @brief Reads the next 8 bytes as an address */
    OP_JMP,

    /** @brief Checks if the stack value is 0, if it does, pops and jumps to */
    OP_JZ,

    /** @brief Pops 2 values, subtracts them and pushes */
    OP_JNZ,

    /** @brief Pops a value off of the stack */
    OP_POP,
} __attribute__((packed)) he_opcode;

_Static_assert(sizeof(he_opcode) == sizeof(uint8_t), "op_code should be same size as byte");

/** @brief Represents a call op */
typedef struct he_op_call {
    /** @brief The address / PC of where to return to */
    size_t return_address;
} he_op_call;

/** @brief Represents one of the jump ops (jmp/jz/jnz) */
typedef struct he_op_jmp {
    /** @brief The address to jump to if the condition is met */
    size_t address;
} he_op_jmp;

/** @brief Represents a push instruction */
typedef struct he_op_push {
    /** @brief The value to push */
    struct he_value val;
} he_op_push;

/** @brief Represents a single instruction */
typedef struct he_op {
    he_opcode op;

    union he_op_as {
        he_op_call call;
        he_op_jmp jmp;
        he_op_push push;
    } op_object;
} he_op;

#ifdef __cplusplus
}
#endif

#endif
