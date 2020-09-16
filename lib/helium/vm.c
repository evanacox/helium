#include "helium/vm.h"
#include "helium/memory.h"
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
    // reading the next 8 bytes as a size_t
    size_t *next_8_bytes = he_vector_at(&mod->ops, vm->pc);

    // so the 8 bytes won't get read as bytecode
    vm->pc += 8;

    return *next_8_bytes;
}

static he_value *he_stack_peek(he_stack *stack) {
    return stack->top;
}

static void he_val_add(he_value *top, he_value second) {
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);

    if (top->type != second.type) {
        // can't add two mismatched types, at least not at this level
        fprintf(stderr, "he_val_add: types mismatched!\n");
    }

    static void *dispatch_table[] = {
        &&none,     // bools cant be added
        &&integer,  // ints can be added
        &&floating, // floats can be added
        &&string,   // strings can be concatted
        &&none,     // objects cant be added
    };

    goto *dispatch_table[top->type];

integer:
    top->as.integer = he_val_as_int(top) + he_val_as_int(&second);
    return;

floating:
    top->as.floating = he_val_as_float(top) + he_val_as_float(&second);
    return;

string:
    fprintf(stderr, "he_val_add: strcat unimplemented!\n");
    longjmp(jump_buffer, -1);

none:
    fprintf(stderr, "he_val_add: unable to + type!\n");
    longjmp(jump_buffer, -1);
}

#define ARITHMETIC_OP(op_name, op)                                                                 \
    do {                                                                                           \
        assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||       \
               top->type == TYPE_STRING || top->type == TYPE_OBJECT);                              \
                                                                                                   \
        if (top->type != second.type) {                                                            \
            fprintf(stderr, "he_val_" #op_name ": types mismatched!\n");                           \
        }                                                                                          \
                                                                                                   \
        switch (top->type) {                                                                       \
            case TYPE_INT:                                                                         \
                /* ... */                                                                          \
                top->as.integer = he_val_as_int(top) op he_val_as_int(&second);                    \
                break;                                                                             \
            case TYPE_FLOAT:                                                                       \
                top->as.floating = he_val_as_float(top) op he_val_as_float(&second);               \
                break;                                                                             \
            default:                                                                               \
                fprintf(stderr, "he_val_" #op_name ": unable to " #op " type!\n");                 \
                longjmp(jump_buffer, -1);                                                          \
                break;                                                                             \
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
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);

    if (top->type != second.type) {
        // can't add two mismatched types, at least not at this level
        fprintf(stderr, "he_val_mod: types mismatched!\n");
    }

    if (top->type == TYPE_INT) {
        top->as.integer = he_val_as_int(top) % he_val_as_int(&second);
        return;
    }

    fprintf(stderr, "he_val_mod: unable to %% type!\n");
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
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);

    assert(second.type == TYPE_BOOL || second.type == TYPE_INT || second.type == TYPE_FLOAT ||
           second.type == TYPE_STRING || second.type == TYPE_OBJECT);

    if (top->type != second.type) {
        top->type = TYPE_BOOL;
        top->as.boolean = false;
    }

    static void *dispatch_table[] = {
        &&boolean,
        &&integer,
        &&floating,
        &&string,
        &&none,
    };

    goto *dispatch_table[top->type];

boolean:
    top->as.boolean = he_val_as_bool(top) == he_val_as_bool(&second);
    return;

integer:
    top->type = TYPE_BOOL;
    top->as.boolean = he_val_as_int(top) == he_val_as_int(&second);
    return;

floating:
    top->type = TYPE_BOOL;
    top->as.boolean = he_val_as_float(top) == he_val_as_float(&second);
    return;

string:
    top->type = TYPE_BOOL;
    top->as.boolean = (strcmp(he_val_as_string(top), he_val_as_string(&second))) == 0;
    return;

none:
    fprintf(stderr, "he_val_eq: unable to == type!\n");
    longjmp(jump_buffer, -1);
}

static void he_val_not(he_value *top) {
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);

    if (!he_val_is_bool(top)) {
        fprintf(stderr, "he_val_not: unable to 'not' type!\n");
        longjmp(jump_buffer, -1);
    }

    top->as.boolean = !top->as.boolean;
}

static void he_val_negate(he_value *top) {
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);

    static void *dispatch_table[] = {
        &&none,     // bools cant be negated
        &&integer,  // ints can be negated
        &&floating, // floats can be negated
        &&none,     // strings can be negated
        &&none,     // objects cant be negated
    };

    goto *dispatch_table[top->type];

integer:
    top->as.integer = -he_val_as_int(top);
    return;

floating:
    top->as.floating = -he_val_as_float(top);
    return;

none:
    fprintf(stderr, "he_val_add: unable to + type!\n");
    longjmp(jump_buffer, -1);
}

static bool he_jmp_result(he_value *top) {
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);

    if (!he_val_is_bool(top)) {
        fprintf(stderr, "he_jmp_result: unable to jmp based on non-bool!\n");
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

he_interpret_flag he_vm_run(he_vm *vm, he_module *mod) {
    vm->mod = mod;

    // computed GOTO table
    static void *dispatch_table[] = {
        &&op_ret,    &&op_call, &&op_load_const, &&op_add,  &&op_sub,  &&op_mul, &&op_div,
        &&op_mod,    &&op_gt,   &&op_lt,         &&op_gteq, &&op_lteq, &&op_eq,  &&op_not,
        &&op_negate, &&op_jmp,  &&op_jz,         &&op_jnz,  &&op_pop,
    };

#define DISPATCH() goto *dispatch_table[*((uint8_t *)he_vector_at(&mod->ops, vm->pc++))];

#define BINARY(op_name)                                                                            \
    do {                                                                                           \
        he_value second = he_stack_pop(&vm->stack);                                                \
        he_val_##op_name(he_stack_peek(&vm->stack), second);                                       \
        DISPATCH()                                                                                 \
    } while (false)

    if (setjmp(jump_buffer) == -1) {
        fprintf(stderr, "helium: exiting with critical error\n");
        return INTERPRET_FAILURE;
    }

    while (vm->pc != vm->mod->ops.size) {
        DISPATCH();

    op_ret:
        vm->pc = he_return_stack_pop(&vm->ret_addrs);
        DISPATCH();

    op_call:;
        size_t next_addr = read_address(vm);
        he_return_stack_push(&vm->ret_addrs, vm->pc);
        vm->pc = next_addr;
        DISPATCH();

    op_load_const:;
        size_t const_addr = read_address(vm);
        he_stack_push(&vm->stack, *(he_value *)he_vector_at(&mod->pool, const_addr));
        DISPATCH();

    op_add:;
        BINARY(add);

    op_sub:;
        BINARY(sub);

    op_mul:;
        BINARY(mul);

    op_div:;
        BINARY(div);

    op_mod:;
        BINARY(mod);

    op_gt:;
        BINARY(gt);

    op_lt:;
        BINARY(lt);

    op_gteq:;
        BINARY(gteq);

    op_lteq:;
        BINARY(lteq);

    op_eq:;
        BINARY(eq);

    op_not:;
        he_val_not(he_stack_peek(&vm->stack));
        DISPATCH();

    op_negate:;
        he_val_negate(he_stack_peek(&vm->stack));
        DISPATCH();

    op_jmp:;
        size_t jmp_addr = read_address(vm);
        vm->pc = jmp_addr;
        DISPATCH();

    op_jz:;
        size_t jz_addr = read_address(vm);

        if (!he_jmp_result(he_stack_peek(&vm->stack))) {
            // jz jumps on zero, aka false
            vm->pc = jz_addr;
        }

        DISPATCH();

    op_jnz:;
        size_t jnz_addr = read_address(vm);

        if (he_jmp_result(he_stack_peek(&vm->stack))) {
            // jnz jumps on non-zero, aka true
            vm->pc = jnz_addr;
        }

        DISPATCH();

    op_pop:
        // discard the popped value
        he_stack_pop(&vm->stack);
        DISPATCH();
    }

    return INTERPRET_SUCCESS;
}
