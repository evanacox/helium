#include "logger.hh"
#include <iomanip>
#include <iostream>
#include <sstream>

constexpr auto NUM_PRECISION = 8;

#define SIMPLE_OP(name)                                                                            \
  case name:                                                                                       \
    std::cout << #name << ')';                                                                     \
    break

void helium_as::print(const helium::mod &mod) {
  std::cout << "== module disassembly ==\n";

  for (auto it = mod.begin(); it != mod.end(); ++it) {
    auto byte = *it;
    auto offset = &it - mod.raw()->ops.array;

    std::cout << std::setfill('0') << std::setw(NUM_PRECISION) << offset << ": "
              << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(byte) << " (";

    switch (byte) {
      SIMPLE_OP(OP_RET);
      SIMPLE_OP(OP_CALL);
      SIMPLE_OP(OP_ADD);
      SIMPLE_OP(OP_SUB);
      SIMPLE_OP(OP_MUL);
      SIMPLE_OP(OP_DIV);
      SIMPLE_OP(OP_MOD);
      SIMPLE_OP(OP_GT);
      SIMPLE_OP(OP_LT);
      SIMPLE_OP(OP_GTEQ);
      SIMPLE_OP(OP_LTEQ);
      SIMPLE_OP(OP_EQ);
      SIMPLE_OP(OP_NOT);
      SIMPLE_OP(OP_NEGATE);
      SIMPLE_OP(OP_POP);
      case OP_LOAD_CONST: {
        auto *addr = reinterpret_cast<const std::size_t *>(&it + 1);
        std::cout << "OP_LOAD_CONST) arg: " << std::setfill('0') << std::setw(NUM_PRECISION)
                  << *addr;
        it += sizeof(size_t);
        break;
      }
      case OP_JMP: {
        auto *addr = reinterpret_cast<const std::size_t *>(&it + 1);
        std::cout << "OP_JMP) arg: " << std::setfill('0') << std::setw(NUM_PRECISION) << *addr;
        it += sizeof(size_t);
        break;
      }
      case OP_JNZ: {
        auto *addr = reinterpret_cast<const std::size_t *>(&it + 1);
        std::cout << "OP_JNZ) arg: " << std::setfill('0') << std::setw(NUM_PRECISION) << *addr;
        it += sizeof(size_t);
        break;
      }
      case OP_JZ: {
        auto *addr = reinterpret_cast<const std::size_t *>(&it + 1);
        std::cout << "OP_JZ) arg: " << std::setfill('0') << std::setw(NUM_PRECISION) << *addr;
        it += sizeof(size_t);
        break;
      }
    }

    std::cout << "\n";
  }
  std::cout << "== end module disassembly ==\n";
}

#undef SIMPLE_OP

static std::string stringify(const helium::value &val) {
  using type = helium::value::type;

  std::stringstream ss;

  switch (val.kind()) {
    case type::boolean:
      ss << "bool: " << std::boolalpha << static_cast<bool>(val);
      break;
    case type::integer:
      ss << "integer: " << static_cast<std::int64_t>(val);
      break;
    case type::fp:
      ss << "fp: " << static_cast<double>(val);
      break;
    case type::string:
      ss << "string: " << static_cast<const char *>(val);
      break;
    case type::object:
      ss << "object: " << std::hex << static_cast<void *>(val);
      break;
  }

  return ss.str();
}

void helium_as::print_result(const helium::vm &vm) {
  std::cout << "result: " << stringify(vm.top()) << "\n";
}

void helium_as::print_state(const helium::vm &vm) {
  std::cout << "== VM State ==\n";
  std::cout << "pc: " << vm.raw()->pc << "\n";
  std::cout << "return addresses: [";

  if (vm.raw()->ret_addrs.vec.size != 0) {
    std::cout << "\n";

    // i is unsigned, so I'm using the size starting at 1 and just `vec[i - 1]`
    // it lets me check if it's valid without underflowing and without a bunch of `if`s
    for (auto i = vm.raw()->ret_addrs.vec.size; i > 0; --i) {
      auto addr = reinterpret_cast<int64_t *>(he_vector_at(&vm.raw()->ret_addrs.vec, i - 1));

      std::cout << "   [" << i << "]: " << std::setfill('0') << std::setw(NUM_PRECISION) << addr
                << "\n";
    }
  }

  std::cout << "]\n";
  std::cout << "stack: [";

  if (vm.raw()->stack.vec.size != 0) {
    std::cout << "\n";

    for (auto i = vm.raw()->stack.vec.size; i > 0; --i) {
      auto *val = reinterpret_cast<he_value *>(he_vector_at(&vm.raw()->stack.vec, i - 1));

      std::cout << "  [" << i << "]: " << stringify(helium::value(*val)) << "\n";
    }
  }

  std::cout << "]\n";
}
