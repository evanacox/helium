#include "helium/module.h"
#include "helium/memory.h"
#include <stdio.h>

void he_const_pool_init(he_const_pool *pool) {
    pool->pool = NULL;
    pool->pool_capacity = 0;
    pool->pool_size = 0;
}

void he_const_pool_destroy(he_const_pool *pool) {
    he_free_array(pool->pool);

    he_const_pool_init(pool);
}

void he_module_init(he_module *mod) {
    mod->ops = NULL;
    mod->ops_capacity = 0;
    mod->ops_size = 0;

    he_const_pool_init(&mod->pool);
}

void he_module_destroy(he_module *mod) {
    he_free_array(mod->ops);
    he_const_pool_init(&mod->pool);

    he_module_init(mod);
}

void he_module_write_byte(he_module *mod, uint8_t byte) {
    if (mod->ops_size == mod->ops_capacity) {
        void *ptr = he_grow_array(mod->ops, sizeof(size_t), &mod->ops_capacity);

        if (!ptr) { fprintf(stderr, "he_module_write_byte: Unable to expand stack!\n"); }

        mod->ops = ptr;
    }

    mod->ops[mod->ops_size++] = byte;
}

void he_module_write_int(he_module *mod, size_t byte) {
    if (mod->ops_size + 8 >= mod->ops_capacity) {
        void *ptr = he_grow_array(mod->ops, sizeof(size_t), &mod->ops_capacity);

        if (!ptr) { fprintf(stderr, "he_module_write_byte: Unable to expand stack!\n"); }

        mod->ops = ptr;
    }

    size_t *bytes = (void *)mod->ops;

    *bytes = byte;
    mod->ops_size += 8;
}
