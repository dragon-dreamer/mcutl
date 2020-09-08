#pragma once

#include "mcutl/timer/timer_defs.h"
#include "mcutl/device/timer/device_timer.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::timer
{

namespace detail
{

template<typename Timer, typename... Options>
constexpr auto parse_and_validate_timer_options() noexcept
{
	return opts::parse_and_validate_options<
		device::timer::options, options_parser, Timer, Options...>();
}

} //namespace detail

template<typename Timer>
[[maybe_unused]] constexpr bool supports_stop_on_overflow
	= device::timer::supports_stop_on_overflow<Timer>;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_prescalers
	= device::timer::supports_prescalers<Timer>;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_reload_value
	= device::timer::supports_reload_value<Timer>;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_reload_value_buffer
	= device::timer::supports_reload_value_buffer<Timer>;
template<typename Timer>
[[maybe_unused]] constexpr count_direction default_count_direction
	= device::timer::default_count_direction<Timer>;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags
	= device::timer::supports_atomic_clear_pending_flags<Timer>;

template<typename Timer, typename... Options>
void configure() MCUTL_NOEXCEPT
{
	device::timer::configure<Timer>([]() constexpr {
		return detail::parse_and_validate_timer_options<Timer, Options...>(); });
}

template<typename Timer, typename... Options>
void reconfigure() MCUTL_NOEXCEPT
{
	device::timer::reconfigure<Timer>([] () constexpr {
		return detail::parse_and_validate_timer_options<Timer, Options...>();
	});
}

template<typename Timer>
[[nodiscard]] auto get_timer_count() MCUTL_NOEXCEPT
{
	return device::timer::get_timer_count<Timer>();
}

template<typename Timer, typename Value>
void set_timer_count(Value value) MCUTL_NOEXCEPT
{
	device::timer::set_timer_count<Timer>(value);
}

template<typename Timer, typename... Interrupts>
inline void clear_pending_flags() MCUTL_NOEXCEPT
{
	device::timer::clear_pending_flags<Timer, Interrupts...>();
}

template<typename Timer, typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	device::timer::clear_pending_flags_atomic<Timer, Interrupts...>();
}

template<typename Timer, typename... Interrupts>
[[nodiscard]] inline auto get_pending_flags() MCUTL_NOEXCEPT
{
	return device::timer::get_pending_flags<Timer, Interrupts...>();
}

template<typename Timer, typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = device::timer::pending_flags_v<Timer, Interrupts...>;

} //namespace mcutl::timer
