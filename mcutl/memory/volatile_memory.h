#pragma once

#include <type_traits>

#ifdef MCUTL_TEST
#	include "mcutl/tests/volatile_memory.h"
#else //MCUTL_TEST
#	include "mcutl/device/memory/device_memory.h"
#endif //MCUTL_TEST

#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

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

template<auto BitMask, auto BitValues, auto Reg, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues, Reg>(ptr);
}

template<auto BitMask, auto BitValues, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues>(reg_ptr, ptr);
}

template<auto BitMask, auto Reg, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	volatile RegStruct* ptr, std::remove_cv_t<types::type_of_member_pointer_t<Reg>> value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, Reg>(ptr, value);
}

template<auto BitMask, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, std::remove_cv_t<RegType> value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask>(reg_ptr, ptr, value);
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	std::remove_cv_t<types::type_of_member_pointer_t<Reg>> value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, Reg, RegStructBase>(value);
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<RegType> value) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, RegStructBase>(reg_ptr, value);
}

template<auto BitMask, auto BitValues, auto Reg, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits() MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues, Reg, RegStructBase>();
}

template<auto BitMask, auto BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	device::memory::set_register_bits<BitMask, BitValues, RegStructBase>(reg_ptr);
}

template<auto Value, auto Reg, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_value(
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, static_cast<bitmask_t>(Value), Reg>(ptr);
}

template<auto Reg, typename RegStruct, typename Value>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_value(
	volatile RegStruct* ptr, Value value) MCUTL_NOEXCEPT
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, Reg>(ptr, value);
}

template<auto Reg, uintptr_t RegStructBase, typename Value>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_value(
	Value value) MCUTL_NOEXCEPT
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, Reg, RegStructBase>(value);
}

template<auto Value, auto Reg, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_value() MCUTL_NOEXCEPT
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		static_cast<bitmask_t>(Value), Reg, RegStructBase>();
}

template<auto Value, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	set_register_bits<max_bitmask<std::make_unsigned_t<std::remove_cv_t<RegType>>>,
		static_cast<std::remove_cv_t<RegType>>(Value)>(reg_ptr, ptr);
}

template<typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr,
	std::remove_cv_t<RegType> value) MCUTL_NOEXCEPT
{
	set_register_bits<max_bitmask<std::make_unsigned_t<std::remove_cv_t<RegType>>>>(
		reg_ptr, ptr, value);
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<RegType> value) MCUTL_NOEXCEPT
{
	set_register_bits<max_bitmask<std::make_unsigned_t<std::remove_cv_t<RegType>>>,
		RegStructBase>(reg_ptr, value);
}

template<auto Value, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	set_register_bits<max_bitmask<std::make_unsigned_t<std::remove_cv_t<RegType>>>,
		static_cast<std::remove_cv_t<RegType>>(Value), RegStructBase>(reg_ptr);
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

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits() MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<BitMask, Reg, RegStructBase>();
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

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<BitMask, RegStructBase>(reg_ptr);
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_bits<BitMask>(reg_ptr, ptr);
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline bool get_register_flag() MCUTL_NOEXCEPT
{
	return static_cast<bool>(get_register_bits<BitMask, Reg, RegStructBase>());
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return static_cast<bool>(get_register_bits<BitMask, Reg>(ptr));
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	return static_cast<bool>(get_register_bits<BitMask, RegStructBase>(reg_ptr));
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return static_cast<bool>(get_register_bits<BitMask>(reg_ptr, ptr));
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, BitValues, Reg, RegArrIndex>(ptr);
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	volatile RegStruct* ptr,
	std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>> values) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, Reg, RegArrIndex>(ptr, values);
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>> value) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, Reg, RegArrIndex, RegStructBase>(value);
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits() MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, BitValues, Reg, RegArrIndex, RegStructBase>();
}

template<auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	return device::memory::get_register_array_bits<Reg, RegArrIndex>(ptr);
}

template<auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_array_bits() MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	return device::memory::get_register_array_bits<Reg, RegArrIndex, RegStructBase>();
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_array_bits() MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	return device::memory::get_register_array_bits<BitMask, Reg, RegArrIndex, RegStructBase>();
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	return device::memory::get_register_array_bits<BitMask, Reg, RegArrIndex>(ptr);
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] inline bool get_register_array_flag() MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	return static_cast<bool>(get_register_array_bits<BitMask, Reg, RegArrIndex, RegStructBase>());
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] inline bool get_register_array_flag(const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	return static_cast<bool>(get_register_array_bits<BitMask, Reg, RegArrIndex>(ptr));
}

template<auto Value, auto Reg, size_t RegArrIndex, typename RegStruct>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_value(
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	using bitmask_t = std::remove_cv_t<std::remove_all_extents_t<
		mcutl::types::type_of_member_pointer_t<Reg>>>;
	set_register_array_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		static_cast<bitmask_t>(Value), Reg, RegArrIndex>(ptr);
}

template<auto Reg, size_t RegArrIndex, typename RegStruct, typename Value>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_value(
	volatile RegStruct* ptr, Value value) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	using bitmask_t = std::remove_cv_t<std::remove_all_extents_t<
		mcutl::types::type_of_member_pointer_t<Reg>>>;
	set_register_array_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		Reg, RegArrIndex>(ptr, value);
}

template<auto Reg, size_t RegArrIndex, uintptr_t RegStructBase, typename Value>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_value(
	Value value) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	using bitmask_t = std::remove_cv_t<std::remove_all_extents_t<
		mcutl::types::type_of_member_pointer_t<Reg>>>;
	set_register_array_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		Reg, RegArrIndex, RegStructBase>(value);
}

template<auto Value, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
inline std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_value(
	) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<types::type_of_member_pointer_t<Reg>>, "Invalid array index");
	using bitmask_t = std::remove_cv_t<std::remove_all_extents_t<
		mcutl::types::type_of_member_pointer_t<Reg>>>;
	set_register_array_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		static_cast<bitmask_t>(Value), Reg, RegArrIndex, RegStructBase>();
}

template<auto BitMask, auto BitValues, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, BitValues, RegArrIndex>(reg_ptr, ptr);
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, std::remove_cv_t<std::remove_all_extents_t<RegType>> values) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, RegArrIndex>(reg_ptr, ptr, values);
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<std::remove_all_extents_t<RegType>> values) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, RegArrIndex, RegStructBase>(reg_ptr, values);
}

template<auto BitMask, auto BitValues,
	size_t RegArrIndex, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	device::memory::set_register_array_bits<BitMask, BitValues, RegArrIndex, RegStructBase>(reg_ptr);
}

template<size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return device::memory::get_register_array_bits<RegArrIndex>(reg_ptr, ptr);
}

template<size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	return device::memory::get_register_array_bits<RegArrIndex, RegStructBase>(reg_ptr);
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	return device::memory::get_register_array_bits<BitMask, RegArrIndex, RegStructBase>(reg_ptr);
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	return device::memory::get_register_array_bits<BitMask, RegArrIndex>(reg_ptr, ptr);
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_array_flag(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	return static_cast<bool>(get_register_array_bits<BitMask, RegArrIndex, RegStructBase>(reg_ptr));
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_array_flag(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	return static_cast<bool>(get_register_array_bits<BitMask, RegArrIndex>(reg_ptr, ptr));
}

template<auto Value, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_value(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	set_register_array_bits<max_bitmask<std::make_unsigned_t<
			std::remove_cv_t<std::remove_all_extents_t<RegType>>>>,
		static_cast<std::remove_cv_t<std::remove_all_extents_t<RegType>>>(Value),
		RegArrIndex>(reg_ptr, ptr);
}

template<size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_value(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr,
	std::remove_cv_t<std::remove_all_extents_t<RegType>> value) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	set_register_array_bits<max_bitmask<std::make_unsigned_t<
			std::remove_cv_t<std::remove_all_extents_t<RegType>>>>,
		RegArrIndex>(reg_ptr, ptr, value);
}

template<size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_array_value(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<std::remove_all_extents_t<RegType>> value) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	set_register_array_bits<max_bitmask<std::make_unsigned_t<
			std::remove_cv_t<std::remove_all_extents_t<RegType>>>>,
		RegArrIndex, RegStructBase>(reg_ptr, value);
}

template<auto Value, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_array_value(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	static_assert(RegArrIndex < std::extent_v<RegType>, "Invalid array index");
	set_register_array_bits<max_bitmask<std::make_unsigned_t<
			std::remove_cv_t<std::remove_all_extents_t<RegType>>>>,
		static_cast<std::remove_cv_t<std::remove_all_extents_t<RegType>>>(Value),
		RegArrIndex, RegStructBase>(reg_ptr);
}

} //namespace mcutl::memory
