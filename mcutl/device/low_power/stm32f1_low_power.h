#pragma once

#include <type_traits>

#include "mcutl/device/low_power/cortex_m3_low_power.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/utils/class_limitations.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::low_power::options
{

struct voltage_regulator_low_power_stop : types::static_class {};
struct debug_support : types::static_class {};

} //namespace mcutl::low_power::options

namespace mcutl::low_power::sleep_mode
{

using sleep = core_stop;
using stop = stop_all_clocks;
using standby = power_off;

} //namespace mcutl::low_power::sleep_mode

namespace mcutl::device::low_power
{

[[maybe_unused]] constexpr bool supports_core_stop = true;
[[maybe_unused]] constexpr bool supports_adc_noise_reduction = false;
[[maybe_unused]] constexpr bool supports_stop_all_clocks = true;
[[maybe_unused]] constexpr bool supports_power_off = true;
using peripheral_type = mcutl::periph::pwr;

struct low_power_options : cortex_m3_low_power_options
{
	uint32_t voltage_regulator_low_power_stop_set_count = 0;
	uint32_t debug_support_set_count = 0;
};

struct pwr_cr_reg_value
{
	uint32_t value = 0;
	uint32_t mask = 0;
};

template<typename SleepMode, typename OptionsLambda>
constexpr pwr_cr_reg_value get_pwr_cr_reg(OptionsLambda options_lambda) noexcept
{
	pwr_cr_reg_value result {};
	
	result.mask = PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk;
	
	constexpr auto options = options_lambda();
	static_assert(!options.voltage_regulator_low_power_stop_set_count
		|| std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::stop>,
		"voltage_regulator_low_power_stop is supported only with stop_all_clocks mode (STM32 stop mode)");
	
	if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::standby>)
	{
		result.value |= PWR_CR_PDDS | PWR_CR_CWUF;
		result.mask |= PWR_CR_CWUF_Msk;
	}
	else if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::stop>
		&& options.voltage_regulator_low_power_stop_set_count)
	{
		result.value |= PWR_CR_LPDS;
	}
	
	return result;
}

template<typename SleepMode, typename WakeUpMode, typename OptionsLambda>
inline void reset_sleep_flags(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	reset_sleep_flags<!std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::sleep>,
		WakeUpMode>(options_lambda);
	if constexpr (options_lambda().debug_support_set_count != 0)
	{
		if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::sleep>)
		{
			mcutl::memory::set_register_bits<DBGMCU_CR_DBG_SLEEP_Msk,
				~DBGMCU_CR_DBG_SLEEP, &DBGMCU_TypeDef::CR, DBGMCU_BASE>();
		}
		else if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::stop>)
		{
			mcutl::memory::set_register_bits<DBGMCU_CR_DBG_STOP_Msk,
				~DBGMCU_CR_DBG_STOP, &DBGMCU_TypeDef::CR, DBGMCU_BASE>();
		}
		else if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::standby>)
		{
			mcutl::memory::set_register_bits<DBGMCU_CR_DBG_STANDBY_Msk,
				~DBGMCU_CR_DBG_STANDBY, &DBGMCU_TypeDef::CR, DBGMCU_BASE>();
		}
	}
}

template<typename SleepMode, typename WakeUpMode, typename OptionsLambda>
inline void sleep(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto pwr_cr_info = get_pwr_cr_reg<SleepMode, OptionsLambda>(options_lambda);
	mcutl::memory::set_register_bits<pwr_cr_info.mask,
		pwr_cr_info.value, &PWR_TypeDef::CR, PWR_BASE>();
	
	constexpr auto options = options_lambda();
	if constexpr (options.debug_support_set_count != 0)
	{
		if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::sleep>)
		{
			mcutl::memory::set_register_bits<DBGMCU_CR_DBG_SLEEP_Msk,
				DBGMCU_CR_DBG_SLEEP, &DBGMCU_TypeDef::CR, DBGMCU_BASE>();
		}
		else if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::stop>)
		{
			mcutl::memory::set_register_bits<DBGMCU_CR_DBG_STOP_Msk,
				DBGMCU_CR_DBG_STOP, &DBGMCU_TypeDef::CR, DBGMCU_BASE>();
		}
		else if constexpr (std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::standby>)
		{
			mcutl::memory::set_register_bits<DBGMCU_CR_DBG_STANDBY_Msk,
				DBGMCU_CR_DBG_STANDBY, &DBGMCU_TypeDef::CR, DBGMCU_BASE>();
		}
	}
	
	execute_sleep<!std::is_same_v<SleepMode, mcutl::low_power::sleep_mode::sleep>,
		WakeUpMode>(options_lambda);
	
	if constexpr (!options.sleep_on_exit_set_count)
		reset_sleep_flags<SleepMode, WakeUpMode>(options_lambda);
}

} //namespace mcutl::device::low_power

namespace mcutl::low_power::detail
{

template<>
struct options_parser<options::voltage_regulator_low_power_stop>
	: opts::base_option_parser<0, nullptr,
	&device::low_power::low_power_options::voltage_regulator_low_power_stop_set_count> {};
template<>
struct options_parser<options::debug_support>
	: opts::base_option_parser<0, nullptr,
	&device::low_power::low_power_options::debug_support_set_count> {};

} //namespace mcu::low_power::detail
