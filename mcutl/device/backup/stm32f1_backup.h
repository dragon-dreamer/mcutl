#pragma once

#include <stdint.h>
#include <stddef.h>

#include "mcutl/device/device.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::device::backup
{

using backup_register_type = uint16_t;
using peripheral_type = mcutl::periph::pwr;

[[maybe_unused]] constexpr uint32_t min_index = 1;

#if defined(BKP_DR42_D)
[[maybe_unused]] constexpr uint32_t max_index = 42;
#elif defined(BKP_DR10_D)
[[maybe_unused]] constexpr uint32_t max_index = 10;
#endif //defined(BKP_DR42_D)

inline bool enable_backup_writes() MCUTL_NOEXCEPT
{
	auto pwr_cr = mcutl::memory::get_register_bits<&PWR_TypeDef::CR, PWR_BASE>();
	if (pwr_cr & PWR_CR_DBP)
		return false;
	
	mcutl::memory::set_register_value<&PWR_TypeDef::CR, PWR_BASE>(pwr_cr | PWR_CR_DBP);
	return true;
}

//Backup writes can not be disabled, when HSE divided by 128 is used as the RTC clock
inline bool disable_backup_writes() MCUTL_NOEXCEPT
{
	if (mcutl::memory::get_register_bits<RCC_BDCR_RTCSEL_Msk, &RCC_TypeDef::BDCR, RCC_BASE>()
		!= RCC_BDCR_RTCSEL_HSE)
	{
		mcutl::memory::set_register_bits<PWR_CR_DBP_Msk, ~PWR_CR_DBP,
			&PWR_TypeDef::CR, PWR_BASE>();
		return true;
	}
	
	return false;
}

template<uint32_t Index>
constexpr auto get_backup_reg_pointer() noexcept
{
	switch (Index)
	{
	case 1: return &BKP_TypeDef::DR1;
	case 2: return &BKP_TypeDef::DR2;
	case 3: return &BKP_TypeDef::DR3;
	case 4: return &BKP_TypeDef::DR4;
	case 5: return &BKP_TypeDef::DR5;
	case 6: return &BKP_TypeDef::DR6;
	case 7: return &BKP_TypeDef::DR7;
	case 8: return &BKP_TypeDef::DR8;
	case 9: return &BKP_TypeDef::DR9;
	case 10: return &BKP_TypeDef::DR10;
		
#if defined (BKP_DR42_D)
	case 11: return &BKP_TypeDef::DR11;
	case 12: return &BKP_TypeDef::DR12;
	case 13: return &BKP_TypeDef::DR13;
	case 14: return &BKP_TypeDef::DR14;
	case 15: return &BKP_TypeDef::DR15;
	case 16: return &BKP_TypeDef::DR16;
	case 17: return &BKP_TypeDef::DR17;
	case 18: return &BKP_TypeDef::DR18;
	case 19: return &BKP_TypeDef::DR19;
	case 20: return &BKP_TypeDef::DR20;
	case 21: return &BKP_TypeDef::DR21;
	case 22: return &BKP_TypeDef::DR22;
	case 23: return &BKP_TypeDef::DR23;
	case 24: return &BKP_TypeDef::DR24;
	case 25: return &BKP_TypeDef::DR25;
	case 26: return &BKP_TypeDef::DR26;
	case 27: return &BKP_TypeDef::DR27;
	case 28: return &BKP_TypeDef::DR28;
	case 29: return &BKP_TypeDef::DR29;
	case 30: return &BKP_TypeDef::DR30;
	case 31: return &BKP_TypeDef::DR31;
	case 32: return &BKP_TypeDef::DR32;
	case 33: return &BKP_TypeDef::DR33;
	case 34: return &BKP_TypeDef::DR34;
	case 35: return &BKP_TypeDef::DR35;
	case 36: return &BKP_TypeDef::DR36;
	case 37: return &BKP_TypeDef::DR37;
	case 38: return &BKP_TypeDef::DR38;
	case 39: return &BKP_TypeDef::DR39;
	case 40: return &BKP_TypeDef::DR40;
	case 41: return &BKP_TypeDef::DR41;
	case 42: return &BKP_TypeDef::DR42;
#endif //defined (BKP_DR42_D)
		
	default: //Never happens
		return &BKP_TypeDef::DR1;
	}
}

template<uint32_t Index>
backup_register_type read_backup() MCUTL_NOEXCEPT
{
	return static_cast<backup_register_type>(
		mcutl::memory::get_register_bits<get_backup_reg_pointer<Index>(), BKP_BASE>());
}

template<uint32_t Index>
void write_backup(backup_register_type value) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<get_backup_reg_pointer<Index>(), BKP_BASE>(value);
}

} //namespace mcutl::device::backup
