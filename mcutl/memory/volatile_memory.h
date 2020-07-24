#pragma once

#ifdef MCUTL_TEST //TODO: document this define
#	include "mcutl/tests/volatile_memory.h"
#else //MCUTL_TEST
#	include "mcutl/device/memory/device_memory.h"
#endif //MCUTL_TEST

#include "mcutl/utils/definitions.h"

namespace mcutl::memory
{

template<typename Type, auto Address>
[[nodiscard]] inline auto volatile_memory() MCUTL_NOEXCEPT
{
	return mcutl::device::memory::volatile_memory<Type, Address>();
}

template<typename Type, typename Address>
[[nodiscard]] inline auto volatile_memory(Address address) MCUTL_NOEXCEPT
{
	return mcutl::device::memory::volatile_memory<Type>(address);
}

template<typename Pointer>
[[nodiscard]] inline auto to_address(volatile Pointer* pointer) MCUTL_NOEXCEPT
{
	return mcutl::device::memory::to_address(pointer);
}

template<typename ValueType>
[[maybe_unused]] constexpr auto max_bitmask = mcutl::device::memory::max_bitmask<ValueType>;

template<auto BitMask, decltype(BitMask) BitValues, auto Reg, typename RegStruct>
inline void set_register_bits(volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_bits<BitMask, BitValues, Reg>(ptr);
}

template<auto BitMask, auto Reg, typename RegStruct>
inline void set_register_bits(volatile RegStruct* reg, decltype(BitMask) value) MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_bits<BitMask, Reg>(reg, value);
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
inline void set_register_bits(decltype(BitMask) value) MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_bits<BitMask, Reg, RegStructBase>(value);
}

template<auto BitMask, decltype(BitMask) BitValues, auto Reg, uintptr_t RegStructBase>
inline void set_register_bits() MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_bits<BitMask, BitValues, Reg, RegStructBase>();
}

template<auto Value, auto Reg, typename RegStruct>
inline void set_register_value(volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_value<Value, Reg>(ptr);
}

template<auto Reg, typename RegStruct, typename Value>
inline void set_register_value(volatile RegStruct* reg, Value value) MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_value<Reg>(reg, value);
}

template<auto Reg, uintptr_t RegStructBase, typename Value>
inline void set_register_value(Value value) MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_value<Reg, RegStructBase>(value);
}

template<auto Value, auto Reg, uintptr_t RegStructBase>
inline void set_register_value() MCUTL_NOEXCEPT
{
	mcutl::device::memory::set_register_value<Value, Reg, RegStructBase>();
}

template<auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* reg) MCUTL_NOEXCEPT
{
	return mcutl::device::memory::get_register_bits<Reg>(reg);
}

template<auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits() MCUTL_NOEXCEPT
{
	return mcutl::device::memory::get_register_bits<Reg, RegStructBase>();
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline auto get_register_bits() MCUTL_NOEXCEPT
{
	return mcutl::device::memory::get_register_bits<Reg, RegStructBase, BitMask>();
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline bool get_register_flag() MCUTL_NOEXCEPT
{
	return mcutl::device::memory::get_register_flag<Reg, RegStructBase, BitMask>();
}

} //namespace mcutl::memory
