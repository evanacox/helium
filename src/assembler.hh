#ifndef HELIUM_AS_ASSEMBLER_HH
#define HELIUM_AS_ASSEMBLER_HH

#include "helium/module.h"
#include <string_view>

namespace helium {
  class assembler {
    std::string_view m_source;

  public:
    explicit assembler(std::string_view source);

    he_module assemble();
  };
} // namespace helium

#endif