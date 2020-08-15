#pragma once

#include "mcutl/systick/systick_defs.h"
#include "mcutl/device/systick/device_systick.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::systick
{

[[maybe_unused]] constexpr auto count_direction = device::systick::count_direction;
using value_type = device::systick::value_type;
[[maybe_unused]] constexpr auto min_value = device::systick::min_value;
[[maybe_unused]] constexpr auto max_value = device::systick::max_value;
[[maybe_unused]] constexpr auto supports_overflow_detection
	= device::systick::supports_overflow_detection;
[[maybe_unused]] constexpr auto supports_value_request
	= device::systick::supports_value_request;
[[maybe_unused]] constexpr auto supports_reload_value
	= device::systick::supports_reload_value;
[[maybe_unused]] constexpr auto supports_reload_value_change
	= device::systick::supports_reload_value_change;
[[maybe_unused]] constexpr auto supports_reset_value
	= device::systick::supports_reset_value;
[[maybe_unused]] constexpr auto supports_atomic_clear_pending_flags
	= device::systick::supports_atomic_clear_pending_flags;
using peripheral_type = device::systick::peripheral_type;

template<typename... Options>
inline void configure() MCUTL_NOEXCEPT
{
	device::systick::configure([] () constexpr
		{ return opts::parse_and_validate_options<device::systick::options,
			detail::options_parser, Options...>(); });
}

template<typename... Options>
inline void reconfigure() MCUTL_NOEXCEPT
{
	device::systick::reconfigure([] () constexpr
		{ return opts::parse_and_validate_options<device::systick::options,
			detail::options_parser, Options...>(); });
}

template<typename... Interrupts>
inline void clear_pending_flags() MCUTL_NOEXCEPT
{
	device::systick::clear_pending_flags<Interrupts...>();
}

template<typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	device::systick::clear_pending_flags_atomic<Interrupts...>();
}

template<typename T = void>
inline auto get_value() MCUTL_NOEXCEPT
{
	return device::systick::get_value<T>();
}

template<typename T = void>
inline void reset_value() MCUTL_NOEXCEPT
{
	device::systick::reset_value<T>();
}

template<typename T = void>
inline bool has_overflown() MCUTL_NOEXCEPT
{
	return device::systick::has_overflown<T>();
}

template<typename T = void>
inline auto get_reload_value() MCUTL_NOEXCEPT
{
	return device::systick::get_reload_value<T>();
}

template<typename T = void>
constexpr auto get_default_reload_value() MCUTL_NOEXCEPT
{
	return device::systick::get_default_reload_value<T>();
}

template<typename T = void>
inline void set_reload_value(value_type value) MCUTL_NOEXCEPT
{
	device::systick::set_reload_value<T>(value);
}

template<typename T = void>
inline void reset_reload_value() MCUTL_NOEXCEPT
{
	set_reload_value<T>(get_default_reload_value<T>());
}

} //namespace mcutl::systick
