#pragma once

#include <limits>
#include <stdint.h>

#include "mcutl/systick/systick.h"
#include "mcutl/utils/duration.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/math.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::systick
{

namespace detail
{

template<typename TickType>
void wait_systicks(TickType ticks) MCUTL_NOEXCEPT
{
	static_assert(count_direction == direction_type::top_down,
		"Bottom-up SYSTICK count direction is not supported yet");
	static_assert(supports_reload_value && supports_value_request
		&& supports_overflow_detection,
		"Insufficient SYSTICK capabilities");
	static_assert(sizeof(ticks) >= sizeof(value_type),
		"Insufficiently large tick type");
	static_assert(std::is_unsigned_v<TickType>,
		"Tick type must be unsigned");
	
	auto load_value = get_reload_value();
	auto initial_value = get_value();
	if constexpr (!overflow_flag_cleared_on_read)
		clear_overflow_flag();
	while (ticks > initial_value)
	{
		while (!has_overflown()) {}
		
		if constexpr (!overflow_flag_cleared_on_read)
			clear_overflow_flag();
		
		ticks -= initial_value;
		initial_value = load_value;
	}

	ticks = initial_value - ticks;
	if constexpr (!overflow_flag_cleared_on_read)
		clear_overflow_flag();
	if (ticks)
	{
		while (!has_overflown() && get_value() > ticks)
		{
		}
	}
}

template<uint64_t SysTickFrequency, typename ConstDuration, typename TickType>
constexpr TickType get_ticks_to_wait() noexcept
{
	if constexpr (ConstDuration::duration_type::period::den <= 1'000'000ull)
	{
		return math::mul_with_check<TickType,
			math::mul_with_check<TickType, ConstDuration::interval,
				math::div_ceil<SysTickFrequency, ConstDuration::duration_type::period::den>()>(),
			ConstDuration::duration_type::period::num>();
	}
	else
	{
		return math::mul_with_check<TickType,
			math::div_ceil<
				math::mul_with_check<uint64_t,
					math::mul_with_check<uint64_t, ConstDuration::interval, SysTickFrequency>(),
				ConstDuration::duration_type::period::num>(), ConstDuration::duration_type::period::den>(),
			1u
		>();
	}
}

template<auto Ticks>
inline void wait_systicks() MCUTL_NOEXCEPT
{
	if constexpr (Ticks != 0)
		wait_systicks(Ticks);
}

} //namespace detail

template<uint64_t SysTickFrequency, uint16_t MSec, typename TickType = value_type>
inline void wait_msec() MCUTL_NOEXCEPT
{
	detail::wait_systicks<detail::get_ticks_to_wait<
		SysTickFrequency, types::milliseconds<MSec>, TickType>()>();
}

template<uint64_t SysTickFrequency, uint16_t USec, typename TickType = value_type>
inline void wait_usec() MCUTL_NOEXCEPT
{
	detail::wait_systicks<detail::get_ticks_to_wait<
		SysTickFrequency, types::microseconds<USec>, TickType>()>();
}

template<uint64_t SysTickFrequency, uint16_t NSec, typename TickType = value_type>
inline void wait_nsec() MCUTL_NOEXCEPT
{
	detail::wait_systicks<detail::get_ticks_to_wait<
		SysTickFrequency, types::nanoseconds<NSec>, TickType>()>();
}

template<uint64_t SysTickFrequency, typename ConstDuration, typename TickType = value_type>
inline void wait() MCUTL_NOEXCEPT
{
	detail::wait_systicks<detail::get_ticks_to_wait<
		SysTickFrequency, ConstDuration, TickType>()>();
}

} //namespace mcutl::systick
