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

template<auto BitMask, auto BitValues, auto Reg, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>>
	set_register_bits([[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<types::type_of_member_pointer_t<Reg>>;
	
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		ptr->*Reg = static_cast<reg_type>(BitValues);
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
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<types::type_of_member_pointer_t<Reg>> values) noexcept
{
	using reg_type = decltype(values);
	
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		ptr->*Reg = values;
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = ptr->*Reg;
		value &= ~unsigned_bitmask;
		value |= (values & unsigned_bitmask);
		ptr->*Reg = value;
	}
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	std::remove_cv_t<types::type_of_member_pointer_t<Reg>> value) noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, Reg>(volatile_memory<class_t, RegStructBase>(), value);
}

template<auto BitMask, auto BitValues, auto Reg, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits() noexcept
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

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	using reg_type = std::remove_cv_t<types::type_of_member_pointer_t<Reg>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(
		get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<types::type_of_member_pointer_t<Reg>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(get_register_bits<Reg>(ptr) & unsigned_bitmask);
}

template<auto BitMask, auto BitValues, typename RegType, typename RegStruct>
inline void set_register_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<RegType>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		ptr->*reg_ptr = static_cast<reg_type>(BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = ptr->*reg_ptr;
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		ptr->*reg_ptr = value;
	}
}

template<auto BitMask, typename RegType, typename RegStruct>
inline void set_register_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<RegType> values) noexcept
{
	using reg_type = decltype(values);
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		ptr->*reg_ptr = values;
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = ptr->*reg_ptr;
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		ptr->*reg_ptr = value;
	}
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<RegType> value) noexcept
{
	set_register_bits<BitMask>(reg_ptr, volatile_memory<RegStruct, RegStructBase>(), value);
}

template<auto BitMask, auto BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	set_register_bits<BitMask, BitValues>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>());
}

template<typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	return ptr->*reg_ptr;
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	return get_register_bits(reg_ptr, volatile_memory<RegStruct, RegStructBase>());
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	using reg_type = std::remove_cv_t<RegType>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(
		get_register_bits(reg_ptr, volatile_memory<RegStruct, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<RegType>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(get_register_bits(reg_ptr, ptr) & unsigned_bitmask);
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	[[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<
		std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>>;
	
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		(ptr->*Reg)[RegArrIndex] = static_cast<reg_type>(BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = (ptr->*Reg)[RegArrIndex];
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		(ptr->*Reg)[RegArrIndex] = value;
	}
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<
		std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>> values) noexcept
{
	using reg_type = decltype(values);
	
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		(ptr->*Reg)[RegArrIndex] = values;
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = (ptr->*Reg)[RegArrIndex];
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		(ptr->*Reg)[RegArrIndex] = value;
	}
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>> value) noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_array_bits<BitMask, Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>(), value);
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_array_bits<BitMask, BitValues, Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(const volatile RegStruct* ptr) noexcept
{
	return (ptr->*Reg)[RegArrIndex];
}

template<auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_array_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	return get_register_array_bits<Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>());
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_array_bits() noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>>;
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(get_register_array_bits<Reg, RegArrIndex>(
		volatile_memory<class_t, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(get_register_array_bits<Reg, RegArrIndex>(ptr) & unsigned_bitmask);
}

template<auto BitMask, auto BitValues, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<RegType>>;
	
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		(ptr->*reg_ptr)[RegArrIndex] = static_cast<reg_type>(BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = (ptr->*reg_ptr)[RegArrIndex];
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		(ptr->*reg_ptr)[RegArrIndex] = value;
	}
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<std::remove_all_extents_t<RegType>> values) noexcept
{
	using reg_type = decltype(values);
	
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		(ptr->*reg_ptr)[RegArrIndex] = values;
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = (ptr->*reg_ptr)[RegArrIndex];
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		(ptr->*reg_ptr)[RegArrIndex] = value;
	}
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<std::remove_all_extents_t<RegType>> value) noexcept
{
	set_register_array_bits<BitMask, RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>(), value);
}

template<auto BitMask, auto BitValues,
	size_t RegArrIndex, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr) noexcept
{
	set_register_array_bits<BitMask, BitValues, RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>());
}

template<size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	return (ptr->*reg_ptr)[RegArrIndex];
}

template<size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr) noexcept
{
	return get_register_array_bits<RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>());
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<RegType>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(get_register_array_bits<RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<RegType>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	return static_cast<reg_type>(get_register_array_bits<RegArrIndex>(reg_ptr, ptr) & unsigned_bitmask);
}

} //namespace mcutl::device::memory::common
