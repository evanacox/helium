#include "helium/vm.h"
#include "helium/instruction.h"
#include "helium/memory.h"
#include "helium/value.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

jmp_buf jump_buffer;

static void he_stack_init(he_stack *stack) {
    he_vector_init(&stack->vec, sizeof(he_value));

    stack->top = NULL;
}

static void he_stack_destroy(he_stack *stack) {
    he_vector_destroy(&stack->vec);

    // return back to "no capacity" state
    he_stack_init(stack);
}

static void he_return_stack_init(he_return_stack *stack) {
    he_vector_init(&stack->vec, sizeof(size_t));

    stack->top = NULL;
}

static void he_return_stack_destroy(he_return_stack *stack) {
    he_vector_destroy(&stack->vec);

    // set it back to the "no capacity" state
    he_return_stack_init(stack);
}

static void he_stack_push(he_stack *stack, he_value val) {
    he_vector_push_val(&stack->vec, val);

    stack->top = he_vector_last(&stack->vec);
}

static void he_return_stack_push(he_return_stack *stack, size_t pc) {
    he_vector_push_val(&stack->vec, pc);

    stack->top = he_vector_last(&stack->vec);
}

static he_value he_stack_pop(he_stack *stack) {
    he_value val;

    he_vector_pop(&stack->vec, &val);

    return val;
}

static size_t he_return_stack_pop(he_return_stack *stack) {
    size_t val;

    he_vector_pop(&stack->vec, &val);

    return val;
}

static size_t read_address(he_vm *vm) {
    const he_module *mod = vm->mod;

    // reads the byte array as a size_t array
    // and returns the first, effectively
    // reading the next 8 bytes (or 4 or however many it is) as a size_t
    size_t *next_nativewidth_int = he_vector_at(&mod->ops, vm->pc);

    // so the bytes won't get read as bytecode
    vm->pc += sizeof(size_t);

    return *next_nativewidth_int;
}

static he_value *he_stack_peek(he_stack *stack) {
    return stack->top;
}

#define IS_TYPE(expr)                                                                              \
    assert(expr == TYPE_BOOL || expr == TYPE_INT || expr == TYPE_FLOAT || expr == TYPE_STRING ||   \
           expr == TYPE_OBJECT && "type is not a valid value!")

static void he_val_add(he_value *top, he_value second) {
    IS_TYPE(top->type);

    if (top->type != second.type) {
        // can't add two mismatched types, at least not at this level
        fputs("he_val_add: types mismatched!", stderr);
        longjmp(jump_buffer, -1);
    }

    switch (top->type) {
        case TYPE_INT:
            top->as.integer = he_val_as_int(top) + he_val_as_int(&second);
            break;
        case TYPE_FLOAT:
            top->as.floating = he_val_as_float(top) + he_val_as_float(&second);
            break;
        default:
            fputs("he_val_add: unable to + type!", stderr);
            longjmp(jump_buffer, -1);
    }
}

#define ARITHMETIC_OP(op_name, op)                                                                 \
    do {                                                                                           \
        IS_TYPE(top->type);                                                                        \
                                                                                                   \
        if (top->type != second.type) {                                                            \
            fputs("he_val_" #op_name ": types mismatched!\n", stderr);                             \
            longjmp(jump_buffer, -1);                                                              \
        }                                                                                          \
                                                                                                   \
        switch (top->type) {                                                                       \
            case TYPE_INT:                                                                         \
                top->as.integer = he_val_as_int(top) op he_val_as_int(&second);                    \
                break;                                                                             \
            case TYPE_FLOAT:                                                                       \
                top->as.floating = he_val_as_float(top) op he_val_as_float(&second);               \
                break;                                                                             \
            default:                                                                               \
                fputs("he_val_" #op_name ": unable to " #op " type!", stderr);                     \
                longjmp(jump_buffer, -1);                                                          \
        }                                                                                          \
    } while (false)

static void he_val_sub(he_value *top, he_value second) {
    ARITHMETIC_OP(sub, -);
}

static void he_val_mul(he_value *top, he_value second) {
    ARITHMETIC_OP(mul, *);
}

static void he_val_div(he_value *top, he_value second) {
    ARITHMETIC_OP(div, /);
}

static void he_val_mod(he_value *top, he_value second) {
    IS_TYPE(top->type);

    if (top->type != second.type) {
        // can't add two mismatched types, at least not at this level
        fputs("he_val_mod: types mismatched!", stderr);
        longjmp(jump_buffer, -1);
    }

    if (top->type == TYPE_INT) {
        top->as.integer = he_val_as_int(top) % he_val_as_int(&second);
        return;
    }

    fputs("he_val_mod: unable to %% type!", stderr);
    longjmp(jump_buffer, -1);
}

static void he_val_gt(he_value *top, he_value second) {
    ARITHMETIC_OP(gt, >);
}

static void he_val_lt(he_value *top, he_value second) {
    ARITHMETIC_OP(lt, <);
}

static void he_val_gteq(he_value *top, he_value second) {
    ARITHMETIC_OP(gteq, >=);
}

static void he_val_lteq(he_value *top, he_value second) {
    ARITHMETIC_OP(lteq, <=);
}

static void he_val_eq(he_value *top, he_value second) {
    IS_TYPE(top->type);
    IS_TYPE(second.type);

    if (top->type != second.type) {
        top->type = TYPE_BOOL;
        top->as.boolean = false;
        return;
    }

    switch (top->type) {
        case TYPE_BOOL:
            top->as.boolean = he_val_as_bool(top) == he_val_as_bool(&second);
            break;
        case TYPE_INT:
            top->as.boolean = he_val_as_int(top) == he_val_as_int(&second);
            break;
        case TYPE_FLOAT:
            top->as.boolean = he_val_as_float(top) == he_val_as_float(&second);
            break;
        case TYPE_STRING:
            top->as.boolean = (strcmp(he_val_as_string(top), he_val_as_string(&second))) == 0;
            break;
        default:
            fputs("he_val_eq: unable to == type!", stderr);
            longjmp(jump_buffer, -1);
    }

    top->type = TYPE_BOOL;
}

static void he_val_not(he_value *top) {
    IS_TYPE(top->type);

    if (!he_val_is_bool(top)) {
        fputs("he_val_not: unable to 'not' type!", stderr);
        longjmp(jump_buffer, -1);
    }

    top->as.boolean = !top->as.boolean;
}

static void he_val_negate(he_value *top) {
    IS_TYPE(top->type);

    switch (top->type) {
        case TYPE_INT:
            top->as.integer = -he_val_as_int(top);
            break;
        case TYPE_FLOAT:
            top->as.floating = -he_val_as_float(top);
            break;
        default:
            fputs("he_val_add: unable to + type!", stderr);
            longjmp(jump_buffer, -1);
    }
}

static bool he_jmp_result(he_value *top) {
    IS_TYPE(top->type);

    if (!he_val_is_bool(top)) {
        fputs("he_jmp_result: unable to jmp based on non-bool!", stderr);
        longjmp(jump_buffer, -1);
    }

    return top->as.boolean;
}

void he_vm_init(he_vm *vm) {
    he_stack_init(&vm->stack);
    he_return_stack_init(&vm->ret_addrs);

    vm->pc = 0;
    vm->mod = NULL;
}

void he_vm_destroy(he_vm *vm) {
    he_stack_destroy(&vm->stack);
    he_return_stack_destroy(&vm->ret_addrs);

    vm->pc = 0;
    vm->mod = NULL;
}

#define BINARY(op_name)                                                                            \
    do {                                                                                           \
        he_value second = he_stack_pop(&vm->stack);                                                \
        he_val_##op_name(he_stack_peek(&vm->stack), second);                                       \
    } while (false)

#define UNARY(op_name)                                                                             \
    do {                                                                                           \
        he_val_##op_name(he_stack_peek(&vm->stack));                                               \
    } while (false)

void he_vm_use(he_vm *vm, const he_module *mod) {
    // may have more logic later
    vm->mod = mod;
}

he_interpret_flag he_vm_execute_instruction(he_vm *vm, bool has_setjmp_env) {
    assert(vm->mod && "cannot execute instruction on null module");

    // if a jmp_buf environment doesnt exist, one is created
    if (!has_setjmp_env) {
        if (setjmp(jump_buffer) == -1) {
            fputs("helium: exiting with critical error", stderr);
            return INTERPRET_FAILURE;
        }
    }

    uint8_t *instruction = he_vector_at(&vm->mod->ops, vm->pc++);

    switch (*instruction) {
        case OP_RET:
            vm->pc = he_return_stack_pop(&vm->ret_addrs);
            break;
        case OP_CALL: {
            size_t next_addr = read_address(vm);
            he_return_stack_push(&vm->ret_addrs, vm->pc);
            vm->pc = next_addr;
            break;
        }
        case OP_LOAD_CONST: {
            size_t const_addr = read_address(vm);
            he_stack_push(&vm->stack, *(he_value *)he_vector_at(&vm->mod->pool, const_addr));
            break;
        }
        case OP_ADD:
            BINARY(add);
            break;
        case OP_SUB:
            BINARY(sub);
            break;
        case OP_MUL:
            BINARY(mul);
            break;
        case OP_DIV:
            BINARY(div);
            break;
        case OP_MOD:
            BINARY(mod);
            break;
        case OP_GT:
            BINARY(gt);
            break;
        case OP_LT:
            BINARY(lt);
            break;
        case OP_GTEQ:
            BINARY(gteq);
            break;
        case OP_LTEQ:
            BINARY(lteq);
            break;
        case OP_EQ:
            BINARY(eq);
            break;
        case OP_NOT:
            UNARY(not );
            break;
        case OP_NEGATE:
            UNARY(negate);
            break;
        case OP_JMP:
            vm->pc = read_address(vm);
            break;
        case OP_JZ: {
            if (he_jmp_result(he_stack_peek(&vm->stack))) {
                // next instructrion offset is a size_t integer
                vm->pc = read_address(vm);
            }
            break;
        }
        case OP_JNZ: {
            if (!he_jmp_result(he_stack_peek(&vm->stack))) {
                // next instructrion offset is a size_t integer
                vm->pc = read_address(vm);
            }
            break;
        }
        case OP_POP:
            he_stack_pop(&vm->stack);
            break;
        default:
            fprintf(stderr, "he_vm_run: got unknown instruction! value: %hhx\n", *instruction);
            longjmp(jump_buffer, -1);
    }

    return INTERPRET_SUCCESS;
}

he_interpret_flag he_vm_run(he_vm *vm, const he_module *module) {
    vm->mod = module;

    if (setjmp(jump_buffer) == -1) {
        fputs("helium: exiting with critical error", stderr);
        return INTERPRET_FAILURE;
    }

    while (vm->pc != module->ops.size) {
        he_interpret_flag res = he_vm_execute_instruction(vm, true);

        if (res == INTERPRET_FAILURE) {
            // if an instruction fails, we can't exactly run anymore now can we?
            return res;
        }
    }

    return INTERPRET_SUCCESS;
}
