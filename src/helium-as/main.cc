#include "helium/cxx_bindings.hh"
#include "helium_as.hh"
#include "logger.hh"

using result = helium::vm::result;

int main() {
  helium::mod mod;

  mod.add_constant(helium::value::from_int(12));
  mod.add_constant(helium::value::from_int(12));
  mod.write_byte(OP_ADD);
  mod.add_constant(helium::value::from_int(24));
  mod.write_byte(OP_EQ);

  helium_as::print(mod);

  helium::vm vm;
  helium_as::print_state(vm);

  vm.use(mod);

  auto result = result::success;

  do {
    result = vm.execute_instruction();
    helium_as::print_state(vm);
  } while (vm.pc() != mod.ops_size());

  helium_as::print_result(vm);
}
