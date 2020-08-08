# MCUTL memory access layer (mcutl/memory)
MCUTL routes all MCU memory accesses to a separate memory access layer. This is a zero-cost abstraction, which allows to unit-test your firmware on a host PC. Moreover, all the MCU-specific type casting is located exclusively in this layer. If you plan to unit-test your firmware on host PC, you need to use the functions described below when accessing the MCU memory. The MCUTL library uses these functions and never accesses MCU memory directly, which makes it testable. All of the definitions are in the `mcutl::memory` namespace, unless otherwise stated.

## memory.h header
Contains functions to convert addresses and references to byte pointers.

#### to_bytes
```cpp
template<typename Address>
auto to_bytes(Address* address) noexcept;
template<typename Struct>
auto to_bytes(Struct& obj) noexcept;
```
Convert either a pointer or a struct reference to the MCU byte pointer. `Address` and `Struct` types must be trivially copyable.

## volatile_memory.h header
Contains functions to access volatile memory of the MCU. When compiling for the MCU, these functions represent raw volatile memory accesses. When the `MCUTL_TEST` macro is defined, these functions call the special test interface, which is then used for unit-testing. Basically, to test the firmware, you need to check that the code performs correct writes to (and sometime reads from) correct memory locations with the right order. The test mocks and interfaces are described in [mcutl/tests](tests.md) in detail.

For Cortex-M3 microcontrollers, the memory access layer performs automatic memory bit-banding, when possible (i.e., when writing a single bit of a memory word inside the bitband regions). If this behavior is not desired, it can be disabled by defining the `MCUTL_CORTEX_M3_BITBAND_DISABLE` macro.

#### volatile_memory
```cpp
template<typename Type, auto Address>
auto volatile_memory() noexcept;
template<typename Type, typename Address>
auto volatile_memory(Address address) noexcept;
```
Provides access to the memory location with specified address and type. Basically, casts the `address` to the volatile pointer of the required type.

#### to_address
```cpp
template<typename Pointer>
uintptr_t to_address(volatile Pointer* pointer) noexcept;
```
Casts `pointer` to its integer address value.

#### max_bitmask
```cpp
template<typename ValueType>
constexpr std::enable_if_t<std::is_unsigned_v<ValueType>, ValueType>
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
The `set_register_bits`, `set_register_value`, `get_register_bits`, `get_register_flag` groups of overridden functions are designed to work with such registers. You can write to the registers and read them directly, but it's much better to use these MCUTL functions, which enables unit-testing of your firmware on a host PC. MCUTL itself never writes or reads any registers directly to allow unit-testing. You may want to use as much template arguments as possible when calling these functions to allow more compile-time optimizations.

---

```cpp
template<auto BitMask, auto BitValues, auto Reg, typename RegStruct>
void set_register_bits(volatile RegStruct* ptr) noexcept;
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
void set_register_bits(volatile RegStruct* ptr, type_of_Reg_value value) noexcept;
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
void set_register_bits(type_of_Reg_value value) noexcept;
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
template<auto BitMask, auto BitValues, auto Reg, uintptr_t RegStructBase>
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
template<auto BitMask, auto BitValues, typename RegType, typename RegStruct>
void set_register_bits(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) noexcept;
```
Sets `BitValues` bits of a bitmask `BitMask` to `1` in a register `reg_ptr` of a struct `RegStruct`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW, RCC_CFGR_SW_PLL>(&RCC_TypeDef::CFGR, RCC);
```

---

```cpp
template<auto BitMask, typename RegType, typename RegStruct>
void set_register_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, type_of_RegType_value value) noexcept;
```
Sets `value` bits of a bitmask `BitMask` to `1` in a register `reg_ptr` of a struct `RegStruct`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW_PLL>(&RCC_TypeDef::CFGR, RCC, RCC_CFGR_SW);
```

---

```cpp
template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
void set_register_bits(RegType RegStruct::*reg_ptr, type_of_RegType_value value) noexcept;
```
Sets `value` bits of a bitmask `BitMask` to `1` in a register `reg_ptr` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW_PLL, RCC_BASE>(&RCC_TypeDef::CFGR, RCC_CFGR_SW);
```

---

```cpp
template<auto BitMask, auto BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
void set_register_bits(RegType RegStruct::*reg_ptr) noexcept;
```
Sets `BitValues` bits of a bitmask `BitMask` to `1` in a register `reg_ptr` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto rcc_cfgr = RCC->CFGR;
rcc_cfgr &= ~RCC_CFGR_SW;
rcc_cfgr |= RCC_CFGR_SW_PLL;
RCC->CFGR = rcc_cfgr;
```
you may write the following:
```cpp
mcutl::memory::set_register_bits<RCC_CFGR_SW_PLL, RCC_CFGR_SW, RCC_BASE>(&RCC_TypeDef::CFGR);
```

---

```cpp
template<auto Value, auto Reg, typename RegStruct>
void set_register_value(volatile RegStruct* ptr) noexcept;
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
void set_register_value(volatile RegStruct* ptr, Value value) noexcept;
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
void set_register_value(Value value) noexcept;
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
void set_register_value() noexcept;
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
template<auto Value, typename RegType, typename RegStruct>
void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) noexcept;
```
Writes `Value` to a register `reg_ptr` of a struct `RegStruct`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<RCC_CFGR_USBPRE>(&RCC_TypeDef::CFGR, RCC);
```

---

```cpp
template<typename RegType, typename RegStruct>
void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr,
	type_of_RegType_value value) noexcept;
```
Writes `value` to a register `reg_ptr` of a struct `RegStruct`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value(&RCC_TypeDef::CFGR, RCC, RCC_CFGR_USBPRE);
```

---

```cpp
template<uintptr_t RegStructBase, typename RegType, typename RegStruct, typename Value>
void set_register_value(RegType RegStruct::*reg_ptr,
	type_of_RegType_value value) noexcept;
```
Writes `value` to a register `reg_ptr` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<RCC_BASE>(&RCC_TypeDef::CFGR, RCC_CFGR_USBPRE);
```

---

```cpp
template<auto Value, uintptr_t RegStructBase, typename RegType, typename RegStruct>
void set_register_value(RegType RegStruct::*reg_ptr) noexcept;
```
Writes `Value` to a register `reg_ptr` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
RCC->CFGR = RCC_CFGR_USBPRE;
```
you may write the following:
```cpp
mcutl::memory::set_register_value<RCC_CFGR_USBPRE, RCC_BASE>(&RCC_TypeDef::CFGR);
```

---

```cpp
template<auto Reg, typename RegStruct>
auto get_register_bits(const volatile RegStruct* ptr) noexcept;
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
auto get_register_bits() noexcept;
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
template<auto BitMask, auto Reg, uintptr_t RegStructBase>
auto get_register_bits() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register value masked with `BitMask`. For example, instead of the following code:
```c
auto value = RCC->CFGR & RCC_CFGR_SWS;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<RCC_CFGR_SWS, &RCC_TypeDef::CFGR, RCC_BASE>();
```

---

```cpp
template<auto BitMask, auto Reg, typename RegStruct>
auto get_register_bits(const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `Reg` register value masked with `BitMask`. For example, instead of the following code:
```c
auto value = RCC->CFGR & RCC_CFGR_SWS;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<RCC_CFGR_SWS, &RCC_TypeDef::CFGR>(RCC);
```

---

```cpp
template<typename RegType, typename RegStruct>
auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept;
```
Returns `reg_ptr` register value of a struct `RegStruct`. For example, instead of the following code:
```c
auto value = RCC->CFGR;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits(&RCC_TypeDef::CFGR, RCC);
```

---

```cpp
template<uintptr_t RegStructBase, typename RegType, typename RegStruct>
auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept;
```
Returns `reg_ptr` register value of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto value = RCC->CFGR;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<RCC_BASE>(&RCC_TypeDef::CFGR);
```

---

```cpp
template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept;
```
For a struct with address `RegStructBase`, returns `reg_ptr` register value masked with `BitMask`. For example, instead of the following code:
```c
auto value = RCC->CFGR & RCC_CFGR_SWS;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<RCC_CFGR_SWS, RCC_BASE>(&RCC_TypeDef::CFGR);
```

---

```cpp
template<auto BitMask, typename RegType, typename RegStruct>
auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `reg_ptr` register value masked with `BitMask`. For example, instead of the following code:
```c
auto value = RCC->CFGR & RCC_CFGR_SWS;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_bits<RCC_CFGR_SWS>(&RCC_TypeDef::CFGR, RCC);
```

---

```cpp
template<auto BitMask, auto Reg, uintptr_t RegStructBase>
bool get_register_flag() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = !!(RCC->CFGR & RCC_CFGR_SWS);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_flag<RCC_CFGR_SWS, &RCC_TypeDef::CFGR, RCC_BASE>();
```

---

```cpp
template<auto BitMask, auto Reg, typename RegStruct>
bool get_register_flag(const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `Reg` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = !!(RCC->CFGR & RCC_CFGR_SWS);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_flag<RCC_CFGR_SWS, &RCC_TypeDef::CFGR>(RCC);
```

---

```cpp
template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
bool get_register_flag(RegType RegStruct::*reg_ptr) noexcept;
```
For a struct with address `RegStructBase`, returns `reg_ptr` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = !!(RCC->CFGR & RCC_CFGR_SWS);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_flag<RCC_CFGR_SWS, RCC_BASE>(&RCC_TypeDef::CFGR);
```

---

```cpp
template<auto BitMask, typename RegType, typename RegStruct>
bool get_register_flag(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `reg_ptr` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = !!(RCC->CFGR & RCC_CFGR_SWS);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_flag<RCC_CFGR_SWS>(&RCC_TypeDef::CFGR, RCC);
```

#### set_register_array_bits, set_register_array_value, get_register_array_bits, get_register_array_flag
These functions are designed to work with the register arrays. For example, this is the definition of the `NVIC_Type` structure, which allows accesses to the nested vectored interrupt controller for Cortex-M3 controllers:
```c
#define     __OM     volatile
#define     __IOM    volatile
typedef struct
{
  __IOM uint32_t ISER[8U];
        uint32_t RESERVED0[24U];
  __IOM uint32_t ICER[8U];
        uint32_t RESERVED1[24U];
  __IOM uint32_t ISPR[8U];
        uint32_t RESERVED2[24U];
  __IOM uint32_t ICPR[8U];
        uint32_t RESERVED3[24U];
  __IOM uint32_t IABR[8U];
        uint32_t RESERVED4[56U];
  __IOM uint8_t  IP[240U];
        uint32_t RESERVED5[644U];
  __OM  uint32_t STIR;
}  NVIC_Type;
```

Here all the registers (`ISER`, `ICER`, etc) are arrays, and the `set_register_array_bits`, `set_register_array_value`, `get_register_array_bits`, `get_register_array_flag` groups of overridden functions are designed to work with such register arrays. These functions support array indexing at compile time only.

---

```cpp
template<auto BitMask, auto BitValues,
	auto Reg, size_t RegArrIndex, typename RegStruct>
void set_register_array_bits(volatile RegStruct* ptr) noexcept;
```
Sets `BitValues` bits of a bitmask `BitMask` to `1` in a register array `Reg` index `RegArrIndex` of a struct with address `RegStructBase`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	AFIO_EXTICR2_EXTI4_PC, &AFIO_TypeDef::EXTICR, 2>(AFIO);
```

---

```cpp
template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
void set_register_array_bits(volatile RegStruct* ptr, type_of_Reg_value values) noexcept;
```
Sets `values` bits of a bitmask `BitMask` to `1` in a register array `Reg` index `RegArrIndex` of a struct `RegStruct`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	&AFIO_TypeDef::EXTICR, 2>(AFIO, AFIO_EXTICR2_EXTI4_PC);
```

---

```cpp
template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
void set_register_array_bits(type_of_Reg_value value) noexcept;
```
For a struct with address `RegStructBase`, sets `value` bits of a bitmask `BitMask` to `1` in a register array `Reg` index `RegArrIndex`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	&AFIO_TypeDef::EXTICR, 2, AFIO_BASE>(AFIO_EXTICR2_EXTI4_PC);
```

---

```cpp
template<auto BitMask, auto BitValues, auto Reg,
	size_t RegArrIndex, uintptr_t RegStructBase>
void set_register_array_bits() noexcept;
```
For a struct with address `RegStructBase`, sets `BitValues` bits of a bitmask `BitMask` to `1` in a register array `Reg` index `RegArrIndex`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	AFIO_EXTICR2_EXTI4_PC, &AFIO_TypeDef::EXTICR, 2, AFIO_BASE>();
```

---

```cpp
template<auto BitMask, auto BitValues,
	size_t RegArrIndex, typename RegType, typename RegStruct>
void set_register_array_bits(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) noexcept;
```
Sets `BitValues` bits of a bitmask `BitMask` to `1` in a register array `reg_ptr` index `RegArrIndex` for a struct `RegStruct`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	AFIO_EXTICR2_EXTI4_PC, 2>(&AFIO_TypeDef::EXTICR, AFIO);
```

---

```cpp
template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
void set_register_array_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, type_of_RegType_value values) noexcept;
```
Sets `values` bits of a bitmask `BitMask` to `1` in a register array `reg_ptr` index `RegArrIndex` for a struct `RegStruct`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	2>(&AFIO_TypeDef::EXTICR, AFIO, AFIO_EXTICR2_EXTI4_PC);
```

---

```cpp
template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
void set_register_array_bits(RegType RegStruct::*reg_ptr, type_of_RegType_value values) noexcept;
```
For a struct with address `RegStructBase`, sets `values` bits of a bitmask `BitMask` to `1` in a register array `reg_ptr` index `RegArrIndex`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	2, AFIO_BASE>(&AFIO_TypeDef::EXTICR, AFIO_EXTICR2_EXTI4_PC);
```

---

```cpp
template<auto BitMask, auto BitValues,
	size_t RegArrIndex, uintptr_t RegStructBase, typename RegType, typename RegStruct>
void set_register_array_bits(RegType RegStruct::*reg_ptr) noexcept;
```
For a struct with address `RegStructBase`, sets `BitValues` bits of a bitmask `BitMask` to `1` in a register array `reg_ptr` index `RegArrIndex`. For example, instead of the following code:
```c
auto afio_exticr_2 = AFIO->EXTICR[2];
afio_exticr_2 &= ~AFIO_EXTICR2_EXTI4;
afio_exticr_2 |= AFIO_EXTICR2_EXTI4_PC;
AFIO->EXTICR[2] = afio_exticr_2;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_bits<AFIO_EXTICR2_EXTI4,
	AFIO_EXTICR2_EXTI4_PC, 2, AFIO_BASE>(&AFIO_TypeDef::EXTICR);
```

---

```cpp
template<auto Value, auto Reg, size_t RegArrIndex, typename RegStruct>
void set_register_array_value(volatile RegStruct* ptr) noexcept;
```
Writes `Value` to a register array `Reg` index `RegArrIndex` of a struct `RegStruct`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<32, &NVIC_Type::ISER, 3>(NVIC);
```

---

```cpp
template<auto Reg, size_t RegArrIndex, typename RegStruct, typename Value>
void set_register_array_value(volatile RegStruct* ptr, Value value) noexcept;
```
Writes `value` to a register array `Reg` index `RegArrIndex` of a struct `RegStruct`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<&NVIC_Type::ISER, 3>(NVIC, 32);
```

---

```cpp
template<auto Reg, size_t RegArrIndex, uintptr_t RegStructBase, typename Value>
void set_register_array_value(Value value) noexcept;
```
For a struct with address `RegStructBase`, writes `value` to a register array `Reg` index `RegArrIndex`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<&NVIC_Type::ISER, 3, NVIC_BASE>(32);
```

---

```cpp
template<auto Value, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
void set_register_array_value() noexcept;
```
For a struct with address `RegStructBase`, writes `Value` to a register array `Reg` index `RegArrIndex`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<32, &NVIC_Type::ISER, 3, NVIC_BASE>();
```

---

```cpp
template<auto Value, size_t RegArrIndex, typename RegType, typename RegStruct>
void set_register_array_value(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr) noexcept;
```
Writes `Value` to a register array `reg_ptr` index `RegArrIndex` of a struct `RegStruct`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<32, 3>(&NVIC_Type::ISER, NVIC);
```

---

```cpp
template<size_t RegArrIndex, typename RegType, typename RegStruct>
void set_register_array_value(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr,
	std::remove_cv_t<std::remove_all_extents_t<RegType>> value) noexcept;
```
Writes `value` to a register array `reg_ptr` index `RegArrIndex` of a struct `RegStruct`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<3>(&NVIC_Type::ISER, NVIC, 32);
```

---

```cpp
template<size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
void set_register_array_value(RegType RegStruct::*reg_ptr, type_of_RegType_value value) noexcept;
```
For a struct with address `RegStructBase`, writes `value` to a register array `reg_ptr` index `RegArrIndex`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<3, NVIC_BASE>(&NVIC_Type::ISER, 32);
```

---

```cpp
template<auto Value, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
void set_register_array_value(RegType RegStruct::*reg_ptr) noexcept;
```
For a struct with address `RegStructBase`, writes `Value` to a register array `reg_ptr` index `RegArrIndex`. For example, instead of the following code:
```c
NVIC->ISER[3] = 32;
```
you may write the following:
```cpp
mcutl::memory::set_register_array_value<32, 3, NVIC_BASE>(&NVIC_Type::ISER);
```

---

```cpp
template<auto Reg, size_t RegArrIndex, typename RegStruct>
auto get_register_array_bits(const volatile RegStruct* ptr) noexcept;
```
Returns `Reg` register array index `RegArrIndex` value of a struct `RegStruct`. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2];
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<&AFIO_TypeDef::EXTICR, 2>(AFIO);
```

---

```cpp
template<auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
auto get_register_array_bits() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register array index `RegArrIndex` value. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2];
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<&AFIO_TypeDef::EXTICR, 2, AFIO_BASE>();
```

---

```cpp
template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
auto get_register_array_bits() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register array index `RegArrIndex` value masked with `BitMask`. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2] & AFIO_EXTICR2_EXTI4_PC;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<AFIO_EXTICR2_EXTI4_PC,
	&AFIO_TypeDef::EXTICR, 2, AFIO_BASE>();
```

---

```cpp
template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
auto get_register_array_bits(const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `Reg` register array index `RegArrIndex` value masked with `BitMask`. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2] & AFIO_EXTICR2_EXTI4_PC;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<AFIO_EXTICR2_EXTI4_PC,
	&AFIO_TypeDef::EXTICR, 2>(AFIO);
```

---

```cpp
template<size_t RegArrIndex, typename RegType, typename RegStruct>
auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept;
```
Returns `reg_ptr` register array index `RegArrIndex` value of a struct `RegStruct`. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2];
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<2>(&AFIO_TypeDef::EXTICR, AFIO);
```

---

```cpp
template<size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
auto get_register_array_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
```
For a struct with address `RegStructBase`, returns `reg_ptr` register array index `RegArrIndex` value. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2];
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<2, AFIO_BASE>(&AFIO_TypeDef::EXTICR);
```

---

```cpp
template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
auto get_register_array_bits(RegType RegStruct::*reg_ptr) noexcept;
```
For a struct with address `RegStructBase`, returns `reg_ptr` register array index `RegArrIndex` value masked with `BitMask`. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2] & AFIO_EXTICR2_EXTI4_PC;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<AFIO_EXTICR2_EXTI4_PC,
	2, AFIO_BASE>(&AFIO_TypeDef::EXTICR);
```

---

```cpp
template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `reg_ptr` register array index `RegArrIndex` value masked with `BitMask`. For example, instead of the following code:
```c
auto value = AFIO->EXTICR[2] & AFIO_EXTICR2_EXTI4_PC;
```
you may write the following:
```cpp
auto value = mcutl::memory::get_register_array_bits<AFIO_EXTICR2_EXTI4_PC, 2>(
	&AFIO_TypeDef::EXTICR, AFIO);
```

---

```cpp
template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
bool get_register_array_flag() noexcept;
```
For a struct with address `RegStructBase`, returns `Reg` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = NVIC->ISER[1] & (1 << 5);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_array_flag<(1 << 5), &NVIC_Type::ISER, 1, NVIC_BASE>();
```

---

```cpp
template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
bool get_register_array_flag(const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `Reg` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = NVIC->ISER[1] & (1 << 5);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_array_flag<(1 << 5), &NVIC_Type::ISER, 1>(NVIC);
```

---

```cpp
template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
bool get_register_array_flag(RegType RegStruct::*reg_ptr) noexcept;
```
For a struct with address `RegStructBase`, returns `reg_ptr` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = NVIC->ISER[1] & (1 << 5);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_array_flag<(1 << 5), 1, NVIC_BASE>(&NVIC_Type::ISER);
```

---

```cpp
template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
bool get_register_array_flag(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept;
```
For a struct `RegStruct`, returns `reg_ptr` register value masked with `BitMask` and converted to `bool`. For example, instead of the following code:
```c
bool value = NVIC->ISER[1] & (1 << 5);
```
you may write the following:
```cpp
bool value = mcutl::memory::get_register_array_flag<(1 << 5), 1>(&NVIC_Type::ISER, NVIC);
```
