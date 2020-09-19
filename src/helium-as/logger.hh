#ifndef HELIUM_AS_LOGGER_HH
#define HELIUM_AS_LOGGER_HH

#include "helium/cxx_bindings.hh"

namespace helium_as {
  /**
   * @brief Prettyprints a he_module
   * @param mod The mod to pretty-print
   */
  void print(const helium::mod &mod);

  /**
   * @brief Prints the state of a VM
   * @param vm The VM that owns the state to look at
   */
  void print_state(const helium::vm &vm);

  /**
   * @brief Prints the top of the stack of a VM
   * @param vm The VM that owns the stack to look at
   */
  void print_result(const helium::vm &vm);
} // namespace helium_as

#endif
