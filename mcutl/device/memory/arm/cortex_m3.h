#pragma once

#include <stddef.h>
#include <type_traits>

#include "mcutl/device/device.h"
#include "mcutl/device/memory/volatile_memory.h"

#include "mcutl/utils/math.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::memory
{

template<auto BitMask, auto BitValues, auto Reg, typename RegStruct>
inline void set_register_bits(volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return common::set_register_bits<BitMask, BitValues, Reg>(ptr);
}

template<auto BitMask, auto Reg, typename RegStruct>
inline void set_register_bits(volatile RegStruct* ptr,
	std::make_unsigned_t<decltype(BitMask)> values) MCUTL_NOEXCEPT
{
	return common::set_register_bits<BitMask, Reg>(ptr, values);
}

template<auto BitMask, decltype(BitMask) BitValues, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return common::set_register_bits<BitMask, BitValues>(reg_ptr, ptr);
}

template<auto BitMask, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, decltype(BitMask) value) MCUTL_NOEXCEPT
{
	return common::set_register_bits<BitMask>(reg_ptr, ptr, value);
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, typename RegStruct>
inline void set_register_array_bits(volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return common::set_register_array_bits<BitMask, BitValues, Reg, RegArrIndex>(ptr);
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
inline void set_register_array_bits(volatile RegStruct* ptr,
	std::make_unsigned_t<decltype(BitMask)> values) MCUTL_NOEXCEPT
{
	return common::set_register_array_bits<BitMask, Reg, RegArrIndex>(ptr, values);
}

template<auto BitMask, auto BitValues, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr) MCUTL_NOEXCEPT
{
	return common::set_register_array_bits<BitMask, BitValues, RegArrIndex>(reg_ptr, ptr);
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr,
	volatile RegStruct* ptr, std::make_unsigned_t<decltype(BitMask)> values) MCUTL_NOEXCEPT
{
	return common::set_register_array_bits<BitMask, RegArrIndex>(reg_ptr, ptr, values);
}

template<uintptr_t BasePointer>
constexpr bool is_sram_bitband_area() noexcept
{
	return BasePointer >= SRAM_BASE && BasePointer <= SRAM_BASE + 0xfffff;
}

template<uintptr_t BasePointer>
constexpr bool is_periph_bitband_area() noexcept
{
	return BasePointer >= PERIPH_BASE && BasePointer <= PERIPH_BASE + 0xfffff;
}

template<auto BitMask, typename MemberType, typename ClassType, uintptr_t BasePointer>
constexpr bool bit_band_available() noexcept
{
#ifndef SRAM_BB_BASE
	return false;
#elif defined MCUTL_CORTEX_M3_BITBAND_DISABLE
	return false;
#else //SRAM_BB_BASE
	if constexpr (math::set_bits_count(BitMask) != 1)
		return false;
	
	//Currently there's no way to check if BasePointer + MemberPointer offset is aligned
	//Also, there's no way to add MemberPointer offset to BasePointer to perform a better check of address
	if (BasePointer % alignof(MemberType))
		return false;
	
	return (is_sram_bitband_area<BasePointer>() && is_sram_bitband_area<BasePointer + sizeof(ClassType)>())
		|| (is_periph_bitband_area<BasePointer>() && is_periph_bitband_area<BasePointer + sizeof(ClassType)>());
#endif //SRAM_BB_BASE
}

template<auto BitMask, typename MemberPointer, uintptr_t BasePointer>
constexpr bool bit_band_available() noexcept
{
	using class_t = types::class_of_member_pointer_type_t<MemberPointer>;
	using member_t = std::remove_cv_t<types::type_of_member_pointer_type_t<MemberPointer>>;
	return bit_band_available<BitMask, member_t, class_t, BasePointer>();
}

template<auto BitMask, typename MemberArrayPointer, size_t Index, uintptr_t BasePointer>
constexpr bool bit_band_available() noexcept
{
	using class_t = types::class_of_member_pointer_type_t<MemberArrayPointer>;
	using member_t = std::remove_cv_t<std::remove_all_extents_t<
		types::type_of_member_pointer_type_t<MemberArrayPointer>>>;
	return bit_band_available<BitMask, member_t, class_t, BasePointer>();
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
volatile uint32_t* bitband_alias() MCUTL_NOEXCEPT
{
#ifdef SRAM_BB_BASE
	using class_t = types::class_of_member_pointer_t<Reg>;
	constexpr auto bit_offset = math::first_set_bit_position<BitMask>() * 4;
	if constexpr (is_sram_bitband_area<RegStructBase>())
	{
		return common::volatile_memory<uint32_t>(SRAM_BB_BASE
			+ (to_address(&(common::volatile_memory<class_t, RegStructBase>()
				->*Reg)) - SRAM_BASE) * 32
			+ bit_offset
		);
	}
	else
	{
		return common::volatile_memory<uint32_t>(PERIPH_BB_BASE
			+ (to_address(&(common::volatile_memory<class_t, RegStructBase>()
				->*Reg)) - PERIPH_BASE) * 32
			+ bit_offset
		);
	}
#else //SRAM_BB_BASE
	return nullptr;
#endif //SRAM_BB_BASE
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
volatile uint32_t* bitband_alias(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
#ifdef SRAM_BB_BASE
	constexpr auto bit_offset = math::first_set_bit_position<BitMask>() * 4;
	if constexpr (is_sram_bitband_area<RegStructBase>())
	{
		return common::volatile_memory<uint32_t>(SRAM_BB_BASE
			+ (to_address(&(common::volatile_memory<RegStruct, RegStructBase>()
				->*reg_ptr)) - SRAM_BASE) * 32
			+ bit_offset
		);
	}
	else
	{
		return common::volatile_memory<uint32_t>(PERIPH_BB_BASE
			+ (to_address(&(common::volatile_memory<RegStruct, RegStructBase>()
				->*reg_ptr)) - PERIPH_BASE) * 32
			+ bit_offset
		);
	}
#else //SRAM_BB_BASE
	return nullptr;
#endif //SRAM_BB_BASE
}

template<auto BitMask, auto Reg, size_t Index, uintptr_t RegStructBase>
volatile uint32_t* bitband_alias() MCUTL_NOEXCEPT
{
#ifdef SRAM_BB_BASE
	using class_t = types::class_of_member_pointer_t<Reg>;
	constexpr auto bit_offset = math::first_set_bit_position<BitMask>() * 4;
	if constexpr (is_sram_bitband_area<RegStructBase>())
	{
		return common::volatile_memory<uint32_t>(SRAM_BB_BASE
			+ (to_address(&((common::volatile_memory<class_t, RegStructBase>()
				->*Reg)[Index])) - SRAM_BASE) * 32
			+ bit_offset
		);
	}
	else
	{
		return common::volatile_memory<uint32_t>(PERIPH_BB_BASE
			+ (to_address(&((common::volatile_memory<class_t, RegStructBase>()
				->*Reg)[Index])) - PERIPH_BASE) * 32
			+ bit_offset
		);
	}
#else //SRAM_BB_BASE
	return nullptr;
#endif //SRAM_BB_BASE
}

template<auto BitMask, size_t Index, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
volatile uint32_t* bitband_alias(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
#ifdef SRAM_BB_BASE
	constexpr auto bit_offset = math::first_set_bit_position<BitMask>() * 4;
	if constexpr (is_sram_bitband_area<RegStructBase>())
	{
		return common::volatile_memory<uint32_t>(SRAM_BB_BASE
			+ (to_address(&((common::volatile_memory<RegStruct, RegStructBase>()
				->*reg_ptr)[Index])) - SRAM_BASE) * 32
			+ bit_offset
		);
	}
	else
	{
		return common::volatile_memory<uint32_t>(PERIPH_BB_BASE
			+ (to_address(&((common::volatile_memory<RegStruct, RegStructBase>()
				->*reg_ptr)[Index])) - PERIPH_BASE) * 32
			+ bit_offset
		);
	}
#else //SRAM_BB_BASE
	return nullptr;
#endif //SRAM_BB_BASE
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
inline void set_register_bits(decltype(BitMask) value) MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(Reg), RegStructBase>())
		*bitband_alias<unsigned_bitmask, Reg, RegStructBase>() = (value & unsigned_bitmask) ? 1 : 0;
	else
		common::set_register_bits<BitMask, Reg, RegStructBase>(value);
}

template<auto BitMask, auto BitValues, auto Reg, uintptr_t RegStructBase>
inline void set_register_bits() MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(Reg), RegStructBase>())
	{
		*bitband_alias<unsigned_bitmask, Reg, RegStructBase>()
			= (std::make_unsigned_t<decltype(BitMask)>(BitValues) & unsigned_bitmask) ? 1 : 0;
	}
	else
	{
		common::set_register_bits<BitMask, BitValues, Reg, RegStructBase>();
	}
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr,
	std::make_unsigned_t<decltype(BitMask)> value) MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(reg_ptr), RegStructBase>())
		*bitband_alias<unsigned_bitmask, RegStructBase>(reg_ptr) = (value & unsigned_bitmask) ? 1 : 0;
	else
		common::set_register_bits<BitMask, RegStructBase>(reg_ptr, value);
}

template<auto BitMask, decltype(BitMask) BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(reg_ptr), RegStructBase>())
	{
		*bitband_alias<unsigned_bitmask, RegStructBase>(reg_ptr)
			= (std::make_unsigned_t<decltype(BitMask)>(BitValues) & unsigned_bitmask) ? 1 : 0;
	}
	else
	{
		common::set_register_bits<BitMask, BitValues, RegStructBase>(reg_ptr);
	}
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
inline void set_register_array_bits(
	std::make_unsigned_t<decltype(BitMask)> value) MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(Reg), RegArrIndex, RegStructBase>())
	{
		*bitband_alias<unsigned_bitmask, Reg, RegArrIndex, RegStructBase>()
			= (value & unsigned_bitmask) ? 1 : 0;
	}
	else
	{
		common::set_register_array_bits<BitMask, Reg, RegArrIndex, RegStructBase>(value);
	}
}

template<auto BitMask, decltype(BitMask) BitValues, auto Reg,
	size_t RegArrIndex, uintptr_t RegStructBase>
inline void set_register_array_bits() MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(Reg), RegArrIndex, RegStructBase>())
	{
		*bitband_alias<unsigned_bitmask, Reg, RegArrIndex, RegStructBase>()
			= (std::make_unsigned_t<decltype(BitMask)>(BitValues) & unsigned_bitmask) ? 1 : 0;
	}
	else
	{
		common::set_register_array_bits<BitMask, BitValues, Reg, RegArrIndex, RegStructBase>();
	}
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr,
	std::make_unsigned_t<decltype(BitMask)> value) MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(reg_ptr), RegArrIndex, RegStructBase>())
	{
		*bitband_alias<unsigned_bitmask, RegArrIndex, RegStructBase>(reg_ptr)
			= (value & unsigned_bitmask) ? 1 : 0;
	}
	else
	{
		common::set_register_array_bits<BitMask, RegArrIndex, RegStructBase>(reg_ptr, value);
	}
}

template<auto BitMask, decltype(BitMask) BitValues,
	size_t RegArrIndex, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_array_bits(RegType RegStruct::*reg_ptr) MCUTL_NOEXCEPT
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, decltype(reg_ptr), RegArrIndex, RegStructBase>())
	{
		*bitband_alias<unsigned_bitmask, RegArrIndex, RegStructBase>(reg_ptr)
			= (std::make_unsigned_t<decltype(BitMask)>(BitValues) & unsigned_bitmask) ? 1 : 0;
	}
	else
	{
		common::set_register_array_bits<BitMask, BitValues, RegArrIndex, RegStructBase>(reg_ptr);
	}
}

} //namespace mcutl::device::memory
