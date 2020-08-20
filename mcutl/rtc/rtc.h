#pragma once

#include "mcutl/device/rtc/device_rtc.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/rtc/rtc_defs.h"
#include "mcutl/utils/datetime.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::rtc
{

using peripheral_type = device::rtc::peripheral_type;


template<uint32_t PrescalerValue>
[[maybe_unused]] constexpr bool prescaler_supported
	= device::rtc::prescaler_supported<PrescalerValue>;

[[maybe_unused]] constexpr auto min_prescaler = device::rtc::min_prescaler;
[[maybe_unused]] constexpr auto max_prescaler = device::rtc::max_prescaler;

[[maybe_unused]] constexpr bool supports_prescalers = device::rtc::supports_prescalers;
[[maybe_unused]] constexpr bool supports_alarm = device::rtc::supports_alarm;
[[maybe_unused]] constexpr bool supports_second_interrupt
	= device::rtc::supports_second_interrupt;
[[maybe_unused]] constexpr bool supports_overflow_interrupt
	= device::rtc::supports_overflow_interrupt;
[[maybe_unused]] constexpr bool supports_internal_clock_source
	= device::rtc::supports_internal_clock_source;
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags
	= device::rtc::supports_atomic_clear_pending_flags;
[[maybe_unused]] constexpr bool supports_clock_source_reconfiguration
	= device::rtc::supports_clock_source_reconfiguration;

namespace detail
{

template<uint32_t PrescalerValue>
struct options_parser<rtc::clock::prescaler<PrescalerValue>>
	: opts::base_option_parser<PrescalerValue,
		&options::prescaler, &options::prescaler_set_count>
{
	static_assert(prescaler_supported<PrescalerValue>, "Prescaler is not supported");
};

} //namespace detail

template<typename... Options>
void configure() MCUTL_NOEXCEPT
{
	device::rtc::configure([] () constexpr {
		return opts::parse_and_validate_options<device::rtc::options,
			detail::options_parser, Options...>();
	});
}

template<typename... Options>
void reconfigure() MCUTL_NOEXCEPT
{
	device::rtc::reconfigure([] () constexpr {
		return opts::parse_and_validate_options<device::rtc::options,
			detail::options_parser, Options...>();
	});
}

template<typename... Interrupts>
inline void clear_pending_flags() MCUTL_NOEXCEPT
{
	device::rtc::clear_pending_flags<Interrupts...>();
}

template<typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	device::rtc::clear_pending_flags_atomic<Interrupts...>();
}

template<typename... Interrupts>
inline auto get_pending_flags() MCUTL_NOEXCEPT
{
	return device::rtc::get_pending_flags<Interrupts...>();
}

template<typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = device::rtc::pending_flags_v<Interrupts...>;

[[nodiscard]] inline bool is_enabled() MCUTL_NOEXCEPT
{
	return device::rtc::is_enabled();
}

template<typename T = void>
[[nodiscard]] inline bool has_alarmed() MCUTL_NOEXCEPT
{
	return device::rtc::has_alarmed<T>();
}

template<typename DateTime>
inline void get_date_time(DateTime& value) MCUTL_NOEXCEPT
{
	device::rtc::get_date_time(value);
}

template<typename DateTimeOrTimeStamp>
inline void set_date_time(const DateTimeOrTimeStamp& value) MCUTL_NOEXCEPT
{
	device::rtc::set_date_time(value);
}

template<typename T = void>
inline void disable_alarm() MCUTL_NOEXCEPT
{
	device::rtc::disable_alarm<T>();
}

template<typename DateTimeOrTimeStamp>
inline void set_alarm(const DateTimeOrTimeStamp& value) MCUTL_NOEXCEPT
{
	device::rtc::set_alarm(value);
}

} //namespace mcutl::rtc
