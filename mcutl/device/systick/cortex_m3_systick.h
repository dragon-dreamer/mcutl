#pragma once

#include <stdint.h>

#include "mcutl/device/device.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph_defs.h"
#include "mcutl/systick/systick_defs.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::systick::detail
{

template<> struct interrupt_type_helper<interrupt::tick>
	: types::identity<mcutl::interrupt::type::systick> {};

} //namespace mcutl::systick::detail

namespace mcutl::device::systick
{

using value_type = uint32_t;
[[maybe_unused]] constexpr auto count_direction = mcutl::systick::direction_type::top_down;
[[maybe_unused]] constexpr value_type min_value = 0;
[[maybe_unused]] constexpr value_type max_value = 0xffffffu;
[[maybe_unused]] constexpr auto supports_overflow_detection = true;
[[maybe_unused]] constexpr auto supports_value_request = true;
[[maybe_unused]] constexpr auto supports_reload_value = true;
[[maybe_unused]] constexpr auto supports_reload_value_change = true;
[[maybe_unused]] constexpr auto supports_atomic_clear_pending_flags = true;
[[maybe_unused]] constexpr auto supports_reset_value = true;
[[maybe_unused]] constexpr auto overflow_flag_cleared_on_read = true;
using peripheral_type = mcutl::periph::no_periph;

struct options : mcutl::systick::detail::options {};

struct ctrl_info
{
	uint32_t ctrl = 0;
	uint32_t ctrl_mask = 0;
};

template<bool ReConfigure, typename OptionsLambda>
constexpr ctrl_info get_ctrl(OptionsLambda options_lambda) noexcept
{
	constexpr auto options = options_lambda();
	
	ctrl_info ctrl {};
	if constexpr (ReConfigure || options.enable_set_count)
	{
		ctrl.ctrl_mask |= SysTick_CTRL_ENABLE_Msk;
		if constexpr (options.enable)
			ctrl.ctrl |= SysTick_CTRL_ENABLE_Msk;
	}
	
	if constexpr (ReConfigure || options.clock_source_set_count)
	{
		ctrl.ctrl_mask |= SysTick_CTRL_CLKSOURCE_Msk;
		if constexpr (options.clock_source_type == mcutl::systick::detail::clock_source::processor)
			ctrl.ctrl |= SysTick_CTRL_CLKSOURCE_Msk;
	}
	
	if constexpr (ReConfigure || options.tick_set_count)
	{
		ctrl.ctrl_mask |= SysTick_CTRL_TICKINT_Msk;
		if constexpr (options.tick_set_count && !options.tick.disable)
			 ctrl.ctrl |= SysTick_CTRL_TICKINT_Msk;
	}
	
	return ctrl;
}

template<typename OptionsLambda>
inline void set_tick_priority(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto options = options_lambda();
	if constexpr (options.tick_set_count && !options.tick.disable
		&& options.tick.priority != mcutl::interrupt::default_priority)
	{
		mcutl::interrupt::set_priority_atomic<
			mcutl::interrupt::interrupt<mcutl::interrupt::type::systick,
			options.tick.priority, options.tick.subpriority>,
			options.priority_count_set_count ? options.priority_count : mcutl::interrupt::maximum_priorities>();
	}
}

template<typename OptionsLambda>
void configure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto ctrl = get_ctrl<true>(options_lambda);
	
	mcutl::memory::set_register_value<ctrl.ctrl, &SysTick_Type::CTRL, SysTick_BASE>();
	
	set_tick_priority(options_lambda);
}

template<typename OptionsLambda>
void reconfigure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto ctrl = get_ctrl<false>(options_lambda);
	
	mcutl::memory::set_register_bits<ctrl.ctrl_mask, ctrl.ctrl,
		&SysTick_Type::CTRL, SysTick_BASE>();
	
	set_tick_priority(options_lambda);
}

template<typename... Interrupts>
inline void clear_pending_flags() MCUTL_NOEXCEPT
{
	if constexpr (sizeof...(Interrupts))
	{
		static_assert(sizeof...(Interrupts) == 1
			&& std::is_same_v<types::first_type_t<Interrupts...>, mcutl::systick::interrupt::tick>,
			"Invalid SYSTICK interrupts to clear flags for");
		mcutl::memory::set_register_value<SCB_ICSR_PENDSTCLR_Msk,
			&SCB_Type::ICSR, SCB_BASE>();
	}
}

template<typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	clear_pending_flags<Interrupts...>();
}

template<typename T>
inline value_type get_value() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_bits<0xffffffu, &SysTick_Type::VAL, SysTick_BASE>();
}

template<typename T>
inline bool has_overflown() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_flag<SysTick_CTRL_COUNTFLAG_Msk,
		&SysTick_Type::CTRL, SysTick_BASE>();
}

template<typename T>
inline void clear_overflow_flag() MCUTL_NOEXCEPT
{
	has_overflown<T>();
}

template<typename T>
inline value_type get_reload_value() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_bits<0xffffffu, &SysTick_Type::LOAD, SysTick_BASE>();
}

template<typename T>
constexpr value_type get_default_reload_value() noexcept
{
	return SysTick_LOAD_RELOAD_Msk;
}

template<typename T>
inline void set_reload_value(value_type value) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<&SysTick_Type::LOAD, SysTick_BASE>(value);
}

template<typename T>
inline void reset_value() MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<0, &SysTick_Type::VAL, SysTick_BASE>();
}

} //namespace mcutl::device::systick
