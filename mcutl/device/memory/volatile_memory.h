#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>

#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::memory::common
{

template<typename Type, auto Address>
[[nodiscard]] inline volatile Type* volatile_memory() noexcept
{
	return reinterpret_cast<volatile Type*>(Address);
}

template<typename Type, typename Address>
[[nodiscard]] inline volatile Type* volatile_memory(Address address) noexcept
{
	return reinterpret_cast<volatile Type*>(address);
}

template<typename Pointer>
[[nodiscard]] inline uintptr_t to_address(volatile Pointer* pointer) noexcept
{
	return reinterpret_cast<uintptr_t>(pointer);
}

template<typename ValueType>
[[maybe_unused]] constexpr std::enable_if_t<std::is_unsigned_v<ValueType>, ValueType>
	max_bitmask = (std::numeric_limits<ValueType>::max)();

template<auto BitMask, std::make_unsigned_t<decltype(BitMask)> BitValues, auto Reg, typename RegStruct>
void set_register_bits([[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		ptr->*Reg = BitValues;
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = ptr->*Reg;
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		ptr->*Reg = value;
	}
}

template<auto BitMask, auto Reg, typename RegStruct>
void set_register_bits([[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::make_unsigned_t<decltype(BitMask)> value) noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		ptr->*Reg = value;
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = ptr->*Reg;
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & value);
		ptr->*Reg = value;
	}
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
void set_register_bits(decltype(BitMask) value) noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, Reg>(volatile_memory<class_t, RegStructBase>(), value);
}

template<auto BitMask, decltype(BitMask) BitValues, auto Reg, uintptr_t RegStructBase>
void set_register_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, BitValues, Reg>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) noexcept
{
	return ptr->*Reg;
}

template<auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	return get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline auto get_register_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	return static_cast<types::type_of_member_pointer_t<Reg>>(
		get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	return static_cast<types::type_of_member_pointer_t<Reg>>(get_register_bits<Reg>(ptr) & unsigned_bitmask);
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline bool get_register_flag() noexcept
{
	return static_cast<bool>(get_register_bits<Reg, RegStructBase, BitMask>());
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(const volatile RegStruct* ptr) noexcept
{
	return static_cast<bool>(get_register_bits<BitMask, Reg>(ptr));
}

template<auto Value, auto Reg, typename RegStruct>
inline void set_register_value(volatile RegStruct* ptr) noexcept
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, static_cast<bitmask_t>(Value), Reg>(ptr);
}

template<auto Reg, typename RegStruct, typename Value>
inline void set_register_value(volatile RegStruct* ptr, Value value) noexcept
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, Reg>(ptr, value);
}

template<auto Reg, uintptr_t RegStructBase, typename Value>
inline void set_register_value(Value value) noexcept
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, Reg, RegStructBase>(value);
}

template<auto Value, auto Reg, uintptr_t RegStructBase>
inline void set_register_value() noexcept
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		static_cast<bitmask_t>(Value), Reg, RegStructBase>();
}

} //namespace mcutl::device::memory::common
