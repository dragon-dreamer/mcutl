#pragma once

#include "mcutl/low_power/low_power_defs.h"
#include "mcutl/device/low_power/device_low_power.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::low_power
{

[[maybe_unused]] constexpr bool supports_wait_for_event
	= device::low_power::supports_wait_for_event;
[[maybe_unused]] constexpr bool wait_for_interrupt
	= device::low_power::wait_for_interrupt;
[[maybe_unused]] constexpr bool supports_core_stop
	= device::low_power::supports_core_stop;
[[maybe_unused]] constexpr bool supports_adc_noise_reduction
	= device::low_power::supports_adc_noise_reduction;
[[maybe_unused]] constexpr bool supports_stop_all_clocks
	= device::low_power::supports_stop_all_clocks;
[[maybe_unused]] constexpr bool supports_power_off
	= device::low_power::supports_power_off;

using peripheral_type = device::low_power::peripheral_type;

template<typename SleepMode, typename WakeUpMode, typename... Options>
void sleep() MCUTL_NOEXCEPT
{
	device::low_power::sleep<SleepMode, WakeUpMode>([]() constexpr {
		return opts::parse_and_validate_options<device::low_power::low_power_options,
			detail::options_parser, Options...>();
	});
}

template<typename SleepMode, typename WakeUpMode, typename... Options>
inline void reset_sleep_flags() MCUTL_NOEXCEPT
{
	device::low_power::reset_sleep_flags<SleepMode, WakeUpMode>([] () constexpr {
		return opts::parse_and_validate_options<device::low_power::low_power_options,
			detail::options_parser, Options...>();
	});
}

} //namespace mcutl::low_power
