#ifndef HELIUM_CXX_BINDINGS_HH
#define HELIUM_CXX_BINDINGS_HH

#include "helium.h"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

// clang format is disabled because the main .clang-format is made for C.
// I use a different one for C++, and this was formatted with that.

// clang-format off
namespace helium {
  /** @brief Wraps a he_value with RAII and some cast operators */
  class value {
    he_value m_val;

  public:
    enum class type { boolean = 0, integer, fp, string, object };

    static value from_bool(bool b) { return value(he_val_from_bool(b)); }

    static value from_int(std::int64_t integer) { return value(he_val_from_int(integer)); }

    static value from_double(double fl) { return value(he_val_from_float(fl)); }

    static value from_string(const char *str) { return value(he_val_from_string(str)); }

    static value from_object(void *obj) { return value(he_val_from_object(obj)); }

    value(he_value from) : m_val(from) {}

    bool is(type kind) const {
      static_assert(static_cast<std::underlying_type_t<type>>(type::boolean) == TYPE_BOOL, 
        "cannot compare enum class to he_value_type: differing values!");

      // I know it kind of ruins the point of enum classes, but in this case, I am
      // the one deciding when the values of each one changes
      return static_cast<std::underlying_type_t<type>>(kind) == m_val.type;
    }

    bool is_bool() const { return he_val_is_bool(&m_val); }

    bool is_int() const { return he_val_is_int(&m_val); }

    bool is_float() const { return he_val_is_float(&m_val); }

    bool is_string() const { return he_val_is_string(&m_val); }

    bool is_object() const { return he_val_is_object(&m_val); }

    type kind() const {
      static_assert(static_cast<std::underlying_type_t<type>>(type::boolean) == TYPE_BOOL, 
        "cannot convert enum class to he_value_type: differing values!");

      // I know it kind of ruins the point of enum classes, but in this case, I am
      // the one deciding when the values of each one changes
      return static_cast<type>(m_val.type);
    }

    operator bool() const { return he_val_as_bool(&m_val); }

    operator std::int64_t() const { return he_val_as_int(&m_val); }

    operator double() const { return he_val_as_float(&m_val); }

    operator const char *() const { return he_val_as_string(&m_val); }

    operator void *() const { return he_val_as_object(&m_val); }

    operator he_value() const { return m_val; }
  };

  /** @brief Wraps a he_module with RAII */
  class mod {
    he_module m_mod;

  public:
    template <class T> class bytecode_iterator {
    public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = T;
      using difference_type = std::ptrdiff_t;
      using pointer = T *;
      using reference = T &;

    protected:
      pointer m_ptr;

    public:
      bytecode_iterator(T *ptr = nullptr) : m_ptr(ptr) {}

      bytecode_iterator(const bytecode_iterator<T> &other) = default;

      ~bytecode_iterator() {}

      bytecode_iterator<T> &operator=(const bytecode_iterator<T> &other) = default;

      bytecode_iterator<T> &operator=(T *ptr) {
        m_ptr = ptr;
        return (*this);
      }

      operator bool() const { return m_ptr; }

      bool operator==(const bytecode_iterator<T> &other) const { return m_ptr == other.m_ptr; }

      bool operator!=(const bytecode_iterator<T> &other) const { return !(*this == other); }

      bytecode_iterator<T> &operator+=(const difference_type &distance) {
        m_ptr += distance;
        return *this;
      }

      bytecode_iterator<T> &operator-=(const difference_type &distance) {
        m_ptr -= distance;
        return *this;
      }

      bytecode_iterator<T> &operator++() {
        ++m_ptr;
        return *this;
      }

      bytecode_iterator<T> &operator--() {
        --m_ptr;
        return *this;
      }

      bytecode_iterator<T> operator+(const difference_type &movement) {
        auto oldPtr = m_ptr;
        m_ptr += movement;
        auto temp(*this);
        m_ptr = oldPtr;
        return temp;
      }

      bytecode_iterator<T> operator-(const difference_type &movement) {
        auto oldPtr = m_ptr;
        m_ptr -= movement;
        auto temp(*this);
        m_ptr = oldPtr;
        return temp;
      }

      difference_type operator-(const bytecode_iterator<T> &rawIterator) {
        return std::distance(rawIterator.getPtr(), this->getPtr());
      }

      T &operator*() { return *m_ptr; }

      const T &operator*() const { return *m_ptr; }

      T *operator->() { return m_ptr; }

      T *operator&() { return m_ptr; }
    };

    using iterator = bytecode_iterator<uint8_t>;
    using const_iterator = bytecode_iterator<const uint8_t>;

    mod() : m_mod() { he_module_init(&m_mod); }

    ~mod() { he_module_destroy(&m_mod); }

    void write_byte(std::uint8_t byte) { he_module_write_byte(&m_mod, byte); }

    void write_int(std::size_t num) { he_module_write_int(&m_mod, num); }

    void add_constant(value val) { he_module_add_constant(&m_mod, val); }

    [[nodiscard]] std::size_t ops_size() const { return m_mod.ops.size; }

    operator const he_module *() const { return &m_mod; }

    iterator begin() { return reinterpret_cast<uint8_t *>(he_vector_at(&m_mod.ops, 0)); }

    const_iterator cbegin() const {
      return reinterpret_cast<uint8_t *>(he_vector_at(&m_mod.ops, 0));
    }

    const_iterator begin() const {
      return reinterpret_cast<uint8_t *>(he_vector_at(&m_mod.ops, 0));
    }

    iterator end() { return 1 + reinterpret_cast<uint8_t *>(he_vector_at(&m_mod.ops, m_mod.ops.size - 1)); }

    const_iterator cend() const { return 1 + reinterpret_cast<uint8_t *>(he_vector_at(&m_mod.ops, m_mod.ops.size - 1)); }

    const_iterator end() const { return 1 + reinterpret_cast<uint8_t *>(he_vector_at(&m_mod.ops, m_mod.ops.size - 1)); }

    he_module *raw() { return &m_mod; }

    const he_module *raw() const { return &m_mod; }
  };

  /** @brief Wraps a he_vm with RAII */
  class vm {
    he_vm m_vm;

  public:
    enum class result { success, failure };

    vm() : m_vm() { he_vm_init(&m_vm); }

    ~vm() { he_vm_destroy(&m_vm); }

    void use(const mod &mod) { he_vm_use(&m_vm, mod.raw()); }

    result run(const mod &mod) {
      auto result = he_vm_run(&m_vm, mod);

      if (result == INTERPRET_SUCCESS) {
        return result::success;
      }

      return result::failure;
    }

    result execute_instruction() {
      auto result = he_vm_execute_instruction(&m_vm, false);

      if (result == INTERPRET_SUCCESS) {
        return result::success;
      }

      return result::failure;
    }

    operator const he_vm *() const { return &m_vm; }

    value top() const { return value(*m_vm.stack.top); }

    he_vm *raw() { return &m_vm; }

    const he_vm *raw() const { return &m_vm; }

    [[nodiscard]] std::size_t pc() const { return m_vm.pc; }
  };
} // namespace helium
// clang-format on

#endif
