#pragma once

#include <climits>
#include <limits>
#include <type_traits>

#include "mcutl/adc/adc_defs.h"
#include "mcutl/device/adc/device_adc.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::adc
{

using gpio_config = device::adc::gpio_config;

template<typename Adc>
[[maybe_unused]] constexpr bool supports_calibration
	= device::adc::supports_calibration<Adc>;
template<typename Adc>
[[maybe_unused]] constexpr bool supports_input_impedance_option
	= device::adc::supports_input_impedance_option<Adc>;
template<typename Adc>
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags
	= device::adc::supports_atomic_clear_pending_flags<Adc>;
template<typename Adc>
[[maybe_unused]] constexpr bool supports_data_alignment
	= device::adc::supports_data_alignment<Adc>;

template<typename Adc, typename... Interrupts>
inline void clear_pending_flags() MCUTL_NOEXCEPT
{
	device::adc::clear_pending_flags<Adc, Interrupts...>();
}

template<typename Adc, typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	device::adc::clear_pending_flags_atomic<Adc, Interrupts...>();
}

template<typename Adc, typename... Interrupts>
[[nodiscard]] inline auto get_pending_flags() MCUTL_NOEXCEPT
{
	return device::adc::get_pending_flags<Adc, Interrupts...>();
}

template<typename Adc, typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = device::adc::pending_flags_v<Adc, Interrupts...>;

template<typename Adc, typename... Options>
inline void calibrate() MCUTL_NOEXCEPT
{
	device::adc::calibrate<Adc, Options...>(
		[] () constexpr {
			return opts::parse_and_validate_options<
				device::adc::cal_options, detail::cal_options_parser, Adc, Options...>();
		});
}

template<typename Adc>
[[nodiscard]] inline bool is_calibration_finished() MCUTL_NOEXCEPT
{
	return device::adc::is_calibration_finished<Adc>();
}

template<typename Adc, typename... Options>
inline void configure() MCUTL_NOEXCEPT
{
	device::adc::configure<Adc, Options...>(
		[]() constexpr {
			return opts::parse_and_validate_options<
				device::adc::init_options, detail::init_options_parser, Adc, Options...>();
		});
}

template<typename Adc, typename... Options>
inline void reconfigure() MCUTL_NOEXCEPT
{
	device::adc::reconfigure<Adc, Options...>(
		[] () constexpr {
			return opts::parse_and_validate_options<
				device::adc::init_options, detail::init_options_parser, Adc, Options...>();
		});
}

template<typename Adc, typename... Options>
inline decltype(auto) prepare_conversion() MCUTL_NOEXCEPT
{
	return device::adc::prepare_conversion<Adc, Options...>(
		[] () constexpr {
			return opts::parse_and_validate_options<
				device::adc::conv_options, detail::conv_options_parser, Adc, Options...>();
		});
}

} //namespace mcutl::adc
