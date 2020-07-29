#pragma once

#ifdef MCUTL_TEST
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
	return device::memory::volatile_memory<Type, Address>();
}

template<typename Type, typename Address>
[[nodiscard]] inline auto volatile_memory(Address address) MCUTL_NOEXCEPT
{
	return device::memory::volatile_memory<Type>(address);
}

template<typename Pointer>
[[nodiscard]] inline auto to_address(volatile Pointer* pointer) MCUTL_NOEXCEPT
{
	return device::memory::to_address(pointer);
}

template<typename ValueType>
[[maybe_unused]] constexpr auto max_bitmask = mcutl::device::memory::max_bitmask<ValueType>;

template<auto BitMask, decltype(BitMask) BitValues, auto Reg, typename RegStruct>
inline void set_register_bits(volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues, Reg>(ptr);
}

template<auto BitMask, decltype(BitMask) BitValues, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues>(reg_ptr, ptr);
}

template<auto BitMask, auto Reg, typename RegStruct>
inline void set_register_bits(volatile RegStruct* ptr, decltype(BitMask) value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, Reg>(ptr, value);
}

template<auto BitMask, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, decltype(BitMask) value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask>(reg_ptr, ptr, value);
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
inline void set_register_bits(decltype(BitMask) value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, Reg, RegStructBase>(value);
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr, decltype(BitMask) value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, RegStructBase>(reg_ptr, value);
}

template<auto BitMask, decltype(BitMask) BitValues, auto Reg, uintptr_t RegStructBase>
inline void set_register_bits() MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues, Reg, RegStructBase>();
}

template<auto BitMask, decltype(BitMask) BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues, RegStructBase>(reg_ptr);
}

template<auto Value, auto Reg, typename RegStruct>
inline void set_register_value(volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_value<Value, Reg>(ptr);
}

template<auto Reg, typename RegStruct, typename Value>
inline void set_register_value(volatile RegStruct* ptr, Value value) MCUTL_NOEXCEPT
{
	device::memory::set_register_value<Reg>(ptr, value);
}

template<auto Reg, uintptr_t RegStructBase, typename Value>
inline void set_register_value(Value value) MCUTL_NOEXCEPT
{
	device::memory::set_register_value<Reg, RegStructBase>(value);
}

template<auto Value, auto Reg, uintptr_t RegStructBase>
inline void set_register_value() MCUTL_NOEXCEPT
{
	device::memory::set_register_value<Value, Reg, RegStructBase>();
}

template<auto Value, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_value<Value>(reg_ptr, ptr);
}

template<typename RegType, typename RegStruct, typename Value>
inline void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr, Value value) MCUTL_NOEXCEPT
{
	device::memory::set_register_value(reg_ptr, ptr, value);
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct, typename Value>
inline void set_register_value(RegType RegStruct::*reg_ptr, Value value) MCUTL_NOEXCEPT
{
	device::memory::set_register_value<RegStructBase>(reg_ptr, value);
}

template<auto Value, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_value<Value, RegStructBase>(reg_ptr);
}

template<auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<Reg>(ptr);
}

template<auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits() MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<Reg, RegStructBase>();
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline auto get_register_bits() MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<Reg, RegStructBase, BitMask>();
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<BitMask, Reg>(ptr);
}

template<typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* reg) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits(reg_ptr, reg);
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<RegStructBase>(reg_ptr);
}

template<uintptr_t RegStructBase, auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<RegStructBase, BitMask>(reg_ptr);
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<BitMask>(reg_ptr, ptr);
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline bool get_register_flag() MCUTL_NOEXCEPT
{
	return device::memory::get_register_flag<Reg, RegStructBase, BitMask>();
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_flag<BitMask, Reg>(ptr);
}

template<uintptr_t RegStructBase, auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_flag<RegStructBase, BitMask>(reg_ptr);
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_flag<BitMask>(reg_ptr, ptr);
}

} //namespace mcutl::memory
