#pragma once

#include "mcutl/device/device.h"
#include "mcutl/device/memory/volatile_memory.h"

#include "mcutl/utils/math.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::memory
{

template<auto BitMask, std::make_unsigned_t<decltype(BitMask)> BitValues, auto Reg, typename RegStruct>
inline void set_register_bits([[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	return common::set_register_bits<BitMask, BitValues, Reg>(ptr);
}

template<auto BitMask, auto Reg, typename RegStruct>
inline void set_register_bits([[maybe_unused]] volatile RegStruct* reg,
	[[maybe_unused]] std::make_unsigned_t<decltype(BitMask)> values) noexcept
{
	return common::set_register_bits<BitMask, Reg>(reg, values);
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

template<auto BitMask, auto MemberPointer, uintptr_t BasePointer>
constexpr bool bit_band_available() noexcept
{
#ifndef SRAM_BB_BASE
	return false;
#elif defined MCUTL_CORTEX_M3_BITBAND_DISABLE //TODO: document this define
	return false;
#else //SRAM_BB_BASE
	if constexpr (math::set_bits_count(BitMask) != 1)
		return false;
	
	//Currently there's no way to check if BasePointer + MemberPointer offset is aligned
	//Also, there's no way to add MemberPointer offset to BasePointer to perform a better check of address
	using class_t = types::class_of_member_pointer_t<MemberPointer>;
	using member_t = types::type_of_member_pointer_t<MemberPointer>;
	if (BasePointer % alignof(member_t))
		return false;
	
	return (is_sram_bitband_area<BasePointer>() && is_sram_bitband_area<BasePointer + sizeof(class_t)>())
		|| (is_periph_bitband_area<BasePointer>() && is_periph_bitband_area<BasePointer + sizeof(class_t)>());
#endif //SRAM_BB_BASE
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
volatile uint32_t* bitband_alias() noexcept
{
#ifdef SRAM_BB_BASE
	using class_t = types::class_of_member_pointer_t<Reg>;
	constexpr auto bit_offset = math::first_set_bit_position<BitMask>() * 4;
	if constexpr (is_sram_bitband_area<RegStructBase>())
	{
		return common::volatile_memory<uint32_t>(SRAM_BB_BASE
			+ (to_address(&(common::volatile_memory<class_t, RegStructBase>()->*Reg)) - SRAM_BASE) * 32
			+ bit_offset
		);
	}
	else
	{
		return common::volatile_memory<uint32_t>(PERIPH_BB_BASE
			+ (to_address(&(common::volatile_memory<class_t, RegStructBase>()->*Reg)) - PERIPH_BASE) * 32
			+ bit_offset
		);
	}
#else //SRAM_BB_BASE
	return nullptr;
#endif //SRAM_BB_BASE
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
void set_register_bits(decltype(BitMask) value) noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, Reg, RegStructBase>())
		*bitband_alias<unsigned_bitmask, Reg, RegStructBase>() = (value & unsigned_bitmask) ? 1 : 0;
	else
		common::set_register_bits<BitMask, Reg, RegStructBase>(value);
}

template<auto BitMask, std::make_unsigned_t<decltype(BitMask)> BitValues, auto Reg, uintptr_t RegStructBase>
void set_register_bits() noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	
	if constexpr (bit_band_available<unsigned_bitmask, Reg, RegStructBase>())
		*bitband_alias<unsigned_bitmask, Reg, RegStructBase>() = (BitValues & unsigned_bitmask) ? 1 : 0;
	else
		common::set_register_bits<BitMask, BitValues, Reg, RegStructBase>();
}

} //namespace mcutl::device::memory
