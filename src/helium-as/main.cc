#include "helium/helium.h"
#include "helium_as.hh"
#include <iomanip>
#include <iostream>

struct Point {
  double x, y;

  Point(double x, double y) {
    this->x = x;
    this->y = y;
  }

  void print() { std::cout << "Point { x: " << x << ", y: " << y << " }\n"; }
};

void print(he_module &mod) {
  for (auto i = 0; i < mod.ops.size; ++i) {
    uint8_t *bytes = (uint8_t *)he_vector_at(&mod.ops, i);
    std::cout << "byte: ";
    std::printf("%02X", bytes[0]);

    if (*bytes == OP_LOAD_CONST) {
      size_t *addr = reinterpret_cast<size_t *>(bytes + 1);

      std::cout << ", addr: " << *addr;

      i += 8;
    }

    std::cout << "\n";
  }
}

int main() {
  he_module mod;
  he_module_init(&mod);

  he_module_add_constant(&mod, he_val_from_int(12));
  he_module_add_constant(&mod, he_val_from_int(12));
  he_module_write_byte(&mod, OP_ADD);

  // print(mod);

  he_vm vm;
  he_vm_init(&vm);

  he_vm_run(&vm, &mod);

  std::cout << "result: " << he_val_as_int(vm.stack.top) << "\n";
}
