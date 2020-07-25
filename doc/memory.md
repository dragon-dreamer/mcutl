# MCUTL memory access layer (mcutl/memory)
MCUTL routes all MCU memory accesses to a separate memory access layer. This is a zero-cost abstraction, which allows to unit-test your firmware on a host PC. Moreover, all the MCU-specific type casting is located exclusively in this layer. If you plan to unit-test your firmware on host PC, you need to use the functions described below when accessing the MCU memory. The MCUTL library uses these functions and never accesses MCU memory directly, which makes it testable.

## memory.h header
Contains functions to convert addresses and references to byte pointers.

#### to_bytes
```cpp
template<typename Address>
[[nodiscard]] inline auto to_bytes(Address* address) noexcept;
template<typename Struct>
[[nodiscard]] inline auto to_bytes(Struct& obj) noexcept;
```
Convert either a pointer or a struct reference to the MCU byte pointer.

## volatile_memory.h header
Contains functions to access volatile memory of the MCU. When compiling for the MCU, these functions represent raw volatile memory accesses. When the `MCUTL_TEST` macro is defined, these functions call the special test interface, which is then used for unit-testing. Basically, to test the firmware, you need to check that the code performs correct writes to (and sometime reads from) correct memory locations with the right order. The test mocks and interfaces are described in [mcutl/tests](tests.md) in detail.

For Cortex-M3 microcontrollers, the memory access layer performs automatic memory bit-banding, when possible (i.e., when setting a single bit of a memory word inside the bitband regions). If this behavior is not desired, it can be disabled by defining the `MCUTL_CORTEX_M3_BITBAND_DISABLE` macro.

#### volatile_memory
```cpp
template<typename Type, auto Address>
[[nodiscard]] inline auto volatile_memory() noexcept;
template<typename Type, typename Address>
[[nodiscard]] inline auto volatile_memory(Address address) noexcept;
```
Provides access to the memory location with specified address and type. Basically, casts the `address` to the volatile pointer of the required type.

#### to_address
```cpp
template<typename Pointer>
[[nodiscard]] inline uintptr_t to_address(volatile Pointer* pointer) noexcept;
```
Casts `pointer` to its integer address value.

#### max_bitmask
```cpp
template<typename ValueType>
[[maybe_unused]] constexpr std::enable_if_t<std::is_unsigned_v<ValueType>, ValueType>
	max_bitmask = (std::numeric_limits<ValueType>::max)();
```
Represents a bitmask for the unsigned representation of the `ValueType` type, which have all bits set to `1`.

#### set_register_bits, set_register_value, get_register_bits, get_register_flag
Hardware registers for many microcontrollers are represented as structs. For example, for STM32 Cortex-M microcontrollers, this is the definition of `RCC` (reset and clock control) registers:
```c
#define __IO volatile
typedef struct
{
  __IO uint32_t CR;
  __IO uint32_t CFGR;
  __IO uint32_t CIR;
  __IO uint32_t APB2RSTR;
  __IO uint32_t APB1RSTR;
  __IO uint32_t AHBENR;
  __IO uint32_t APB2ENR;
  __IO uint32_t APB1ENR;
  __IO uint32_t BDCR;
  __IO uint32_t CSR;
} RCC_TypeDef;
#define RCC_BASE ...
#define RCC ((RCC_TypeDef *)RCC_BASE)
```
The `set_register_bits`, `set_register_value`, `get_register_bits`, `get_register_flag` groups of overridden functions are designed to work with such registers. You can write to the registers and read them directly, but it's much better to use these MCUTL functions, which enables unit-testing of your firmware on a host PC. MCUTL itself never writes or reads any registers directly to allow unit-testing.

---

```cpp
template<auto BitMask, std::make_unsigned_t<decltype(BitMask)> BitValues, auto Reg, typename RegStruct>
void set_register_bits([[maybe_unused]] volatile RegStruct* ptr) noexcept;
```
Sets `BitValues` bits of a bitmask `BitMask` to `1` in a register `Reg` of a struct `RegStruct`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW, RCC_CFGR_SW_PLL, &RCC_TypeDef::CFGR>(RCC);
```

---

```cpp
template<auto BitMask, auto Reg, typename RegStruct>
void set_register_bits([[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::make_unsigned_t<decltype(BitMask)> value) noexcept;
```
Sets `value` bits of a bitmask `BitMask` to `1` in a register `Reg` of a struct `RegStruct`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW_PLL, &RCC_TypeDef::CFGR>(RCC, RCC_CFGR_SW);
```

---

```cpp
template<auto BitMask, auto Reg, uintptr_t RegStructBase>
void set_register_bits(decltype(BitMask) value) noexcept;
```
Sets `value` bits of a bitmask `BitMask` to `1` in a register `Reg` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW_PLL, &RCC_TypeDef::CFGR, RCC_BASE>(RCC_CFGR_SW);
```

---

```cpp
template<auto BitMask, decltype(BitMask) BitValues, auto Reg, uintptr_t RegStructBase>
void set_register_bits() noexcept;
```
Sets `BitValues` bits of a bitmask `BitMask` to `1` in a register `Reg` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW_PLL, RCC_CFGR_SW, &RCC_TypeDef::CFGR, RCC_BASE>();
```

---

```cpp
template<auto Value, auto Reg, typename RegStruct>
inline void set_register_value(volatile RegStruct* ptr) noexcept;
```
Writes `Value` to a register `Reg` of a struct `RegStruct`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<RCC_CFGR_USBPRE, &RCC_TypeDef::CFGR>(RCC);
```

---

```cpp
template<auto Reg, typename RegStruct, typename Value>
inline void set_register_value(volatile RegStruct* ptr, Value value) noexcept;
```
Writes `value` to a register `Reg` of a struct `RegStruct`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<&RCC_TypeDef::CFGR>(RCC, RCC_CFGR_USBPRE);
```

---

```cpp
template<auto Reg, uintptr_t RegStructBase, typename Value>
inline void set_register_value(Value value) noexcept;
```
Writes `value` to a register `Reg` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<&RCC_TypeDef::CFGR, RCC_BASE>(RCC_CFGR_USBPRE);
```

---

```cpp
template<auto Value, auto Reg, uintptr_t RegStructBase>
inline void set_register_value() noexcept;
```
Writes `Value` to a register `Reg` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<RCC_CFGR_USBPRE, &RCC_TypeDef::CFGR, RCC_BASE>();
```

---

```cpp
template<auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) noexcept;
```
Returns `Reg` register value of a struct `RegStruct`. For example, instead of the following code:
```c
auto value = RCC->CFGR;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<&RCC_TypeDef::CFGR>(RCC);
```

---

```cpp
template<auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits() noexcept;
```
Returns `Reg` register value of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto value = RCC->CFGR;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE>();
```

---

```cpp
template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline auto get_register_bits() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register value masked with `BitMask`. For example, instead of the following code:
```c
auto value = RCC->CFGR & RCC_CFGR_SWS;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE, RCC_CFGR_SWS>();
```

---

```cpp
template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `Reg` register value masked with `BitMask`. For example, instead of the following code:
```c
auto value = RCC->CFGR & RCC_CFGR_SWS;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_CFGR_SWS>(RCC);
```

---

```cpp
template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline bool get_register_flag() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = !!(RCC->CFGR & RCC_CFGR_SWS);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_flag<&RCC_TypeDef::CFGR, RCC_BASE, RCC_CFGR_SWS>();
```

---

```cpp
template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `Reg` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = !!(RCC->CFGR & RCC_CFGR_SWS);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_flag<RCC_CFGR_SWS, &RCC_TypeDef::CFGR>(RCC);
```