#include "helium/module.h"
#include "helium/instruction.h"
#include "helium/memory.h"
#include <stdio.h>

void he_module_init(he_module *mod) {
    he_vector_init(&mod->ops, sizeof(uint8_t));
    he_vector_init(&mod->pool, sizeof(he_value));
}

void he_module_destroy(he_module *mod) {
    he_vector_destroy(&mod->ops);
    he_vector_destroy(&mod->pool);

    he_module_init(mod);
}

void he_module_write_byte(he_module *mod, uint8_t byte) {
    he_vector_push_val(&mod->ops, byte);
}

void he_module_write_int(he_module *mod, size_t bytes) {
    // type punning in C is great. this is completely UB of course,
    // but as the consumer will be reading the same number of bytes
    // as another size_t elsewhere, it works
    uint8_t *as_bytes = (uint8_t *)(&bytes);

    for (unsigned int i = 0; i < sizeof(size_t); ++i) {
        he_module_write_byte(mod, as_bytes[i]);
    }
}

void he_module_add_constant(he_module *mod, he_value val) {
    uint64_t addr = mod->pool.size;
    he_vector_push_val(&mod->pool, val);

    he_module_write_byte(mod, OP_LOAD_CONST);
    he_module_write_int(mod, addr);
}
