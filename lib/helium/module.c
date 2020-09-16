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

void he_module_write_int(he_module *mod, uint64_t bytes) {
    uint8_t *as_bytes = (uint8_t *)(&bytes);

    for (int i = 0; i < 8; ++i) {
        he_module_write_byte(mod, as_bytes[i]);
    }
}

void he_module_add_constant(he_module *mod, he_value val) {
    uint64_t addr = mod->pool.size;
    he_vector_push_val(&mod->pool, val);

    he_module_write_byte(mod, OP_LOAD_CONST);
    he_module_write_int(mod, addr);
}
