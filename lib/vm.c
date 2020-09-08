#include "helium/vm.h"
#include "helium/memory.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

jmp_buf jump_buffer;

static void he_stack_init(he_stack *stack) {
    stack->stack = NULL;
    stack->stack_size = 0;
    stack->stack_capacity = 0;
}

static void he_stack_destroy(he_stack *stack) {
    he_free_array(stack->stack);

    // return back to "no capacity" state
    he_stack_init(stack);
}

static void he_return_stack_init(he_return_stack *stack) {
    stack->stack = NULL;
    stack->stack_size = 0;
    stack->stack_capacity = 0;
}

static void he_return_stack_destroy(he_return_stack *stack) {
    he_free_array(stack->stack);

    // set it back to the "no capacity" state
    he_return_stack_init(stack);
}

static void he_stack_push(he_stack *stack, he_value val) {
    if (stack->stack_size == stack->stack_capacity) {
        void *ptr = he_grow_array(stack->stack, sizeof(he_value), &stack->stack_capacity);

        if (!ptr) {
            fprintf(stderr, "he_stack_push: Unable to expand stack!\n");
            longjmp(jump_buffer, -1);
        }

        stack->stack = ptr;
    }

    stack->stack[stack->stack_size++] = val;
}

static void he_return_stack_push(he_return_stack *stack, size_t pc) {
    if (stack->stack_size == stack->stack_capacity) {
        void *ptr = he_grow_array(stack->stack, sizeof(size_t), &stack->stack_capacity);

        if (!ptr) {
            fprintf(stderr, "he_return_stack_push: Unable to expand stack!\n");
            longjmp(jump_buffer, -1);
        }

        stack->stack = ptr;
    }

    stack->stack[stack->stack_size++] = pc;
}

static he_value he_stack_pop(he_stack *stack) {
    return stack->stack[stack->stack_size--];
}

static size_t he_return_stack_pop(he_return_stack *stack) {
    return stack->stack[stack->stack_size--];
}

static size_t read_address(he_vm *vm) {
    // reads the byte array as a size_t array
    // and returns the first, effectively
    // reading the next 8 bytes as a size_t
    size_t *next_8_bytes = (void *)(vm->mod->ops + vm->pc);

    // so the 8 bytes won't get read as bytecode
    vm->pc += 8;

    return *next_8_bytes;
}

static he_value *he_stack_peek(he_stack *stack) {
    return stack->stack + stack->stack_size;
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
    assert(top->type == TYPE_BOOL || top->type == TYPE_INT || top->type == TYPE_FLOAT ||           \
           top->type == TYPE_STRING || top->type == TYPE_OBJECT);                                  \
                                                                                                   \
    if (top->type != second.type) { fprintf(stderr, "he_val_" #op_name ": types mismatched!\n"); } \
                                                                                                   \
    static void *dispatch_table[] = {&&none, &&integer, &&floating, &&none, &&none};               \
                                                                                                   \
    goto *dispatch_table[top->type];                                                               \
                                                                                                   \
    integer:                                                                                       \
    top->as.integer = he_val_as_int(top) op he_val_as_int(&second);                                \
    return;                                                                                        \
                                                                                                   \
    floating:                                                                                      \
    top->as.floating = he_val_as_float(top) op he_val_as_float(&second);                           \
    return;                                                                                        \
                                                                                                   \
    none:                                                                                          \
    fprintf(stderr, "he_val_" #op_name ": unable to " #op " type!\n");                             \
    longjmp(jump_buffer, -1);

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
        &&none,     // bools cant be added
        &&integer,  // ints can be added
        &&floating, // floats can be added
        &&none,     // strings can be concatted
        &&none,     // objects cant be added
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
    he_return_stack_init(&vm->return_addresses);

    vm->pc = 0;
    vm->mod = NULL;
}

void he_vm_destroy(he_vm *vm) {
    he_stack_destroy(&vm->stack);
    he_return_stack_destroy(&vm->return_addresses);

    vm->pc = 0;
    vm->mod = NULL;
}

void he_vm_run(he_vm *vm, he_module *mod) {
    vm->mod = mod;

    // computed GOTO table
    static void *dispatch_table[] = {
        &&op_ret,    &&op_call, &&op_load_const, &&op_add,  &&op_sub,  &&op_mul, &&op_div,
        &&op_mod,    &&op_gt,   &&op_lt,         &&op_gteq, &&op_lteq, &&op_eq,  &&op_not,
        &&op_negate, &&op_jmp,  &&op_jz,         &&op_jnz,  &&op_pop,
    };

#define DISPATCH() goto *dispatch_table[mod->ops[vm->pc++]]

#define BINARY(op_name)                                                                            \
    he_value op_name##_second = he_stack_pop(&vm->stack);                                          \
    he_val_##op_name(he_stack_peek(&vm->stack), op_name##_second);                                 \
    DISPATCH()

    if (setjmp(jump_buffer) == -1) {
        fprintf(stderr, "helium: exiting with critical error\n");
        exit(-1);
    }

    while (vm->pc != vm->mod->ops_size) {
    op_ret:
        vm->pc = he_return_stack_pop(&vm->return_addresses);
        DISPATCH();

    op_call:;
        size_t next_addr = read_address(vm);
        he_return_stack_push(&vm->return_addresses, vm->pc);
        vm->pc = next_addr;
        DISPATCH();

    op_load_const:;
        size_t const_addr = read_address(vm);
        he_stack_push(&vm->stack, mod->pool.pool[const_addr]);
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
}
