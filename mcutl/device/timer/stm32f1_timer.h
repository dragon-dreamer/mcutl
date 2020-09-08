#pragma once

#include <limits>
#include <ratio>
#include <stdint.h>
#include <type_traits>

#include "mcutl/clock/clock.h"
#include "mcutl/device/device.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/timer/timer_defs.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::timer
{

namespace update_request_source
{

struct overflow_ug_bit_slave_controller {};
struct overflow {};

} //namespace update_request_source

template<bool Disable>
struct disable_update {};

struct trigger_registers_update {};

namespace master_mode
{

struct none {};
struct output_enable {};
struct output_update {};
	
} //namespace master_mode

namespace interrupt
{

using update = overflow;

} //namespace interrupt

namespace detail
{

struct prescaler_traits : prescaler_traits_base<types::limits<1, 65536>> {};

using reload_limits = types::limits<2, 65536>;

struct timer_frequency_traits_1_8_9_10_11
{
	template<typename ClockConfig>
	static constexpr auto get_timer_frequency() noexcept
	{
		return std::ratio<mcutl::clock::get_clock_info<ClockConfig,
			mcutl::device::clock::device_source_id::timer1_8_9_10_11>()
			.get_exact_frequency()>();
	}
};

struct timer_frequency_traits_2_3_4_5_6_7_12_13_14
{
	template<typename ClockConfig>
	static constexpr auto get_timer_frequency() noexcept
	{
		return std::ratio<mcutl::clock::get_clock_info<ClockConfig,
			mcutl::device::clock::device_source_id::timer2_3_4_5_6_7_12_13_14>()
			.get_exact_frequency()>();
	}
};

} //namespace detail

#ifdef RCC_APB1ENR_TIM2EN
using timer2 = detail::timer_base<2, uint16_t, 0xffffu, mcutl::periph::timer2,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer2, interrupt::update> : types::identity<mcutl::interrupt::type::tim2> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM2EN

#ifdef RCC_APB1ENR_TIM3EN
using timer3 = detail::timer_base<3, uint16_t, 0xffffu, mcutl::periph::timer3,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer3, interrupt::update> : types::identity<mcutl::interrupt::type::tim3> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM3EN

#ifdef RCC_APB1ENR_TIM4EN
using timer4 = detail::timer_base<4, uint16_t, 0xffffu, mcutl::periph::timer4,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer4, interrupt::update> : types::identity<mcutl::interrupt::type::tim4> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM4EN

#ifdef RCC_APB1ENR_TIM5EN
using timer5 = detail::timer_base<5, uint16_t, 0xffffu, mcutl::periph::timer5,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer5, interrupt::update> : types::identity<mcutl::interrupt::type::tim5> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM5EN

#ifdef RCC_APB1ENR_TIM6EN
using timer6 = detail::timer_base<6, uint16_t, 0xffffu, mcutl::periph::timer6,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer6, interrupt::update> : types::identity<mcutl::interrupt::type::tim6> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM6EN

#ifdef RCC_APB1ENR_TIM7EN
using timer7 = detail::timer_base<7, uint16_t, 0xffffu, mcutl::periph::timer7,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer7, interrupt::update> : types::identity<mcutl::interrupt::type::tim7> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM7EN

#ifdef RCC_APB1ENR_TIM12EN
using timer12 = detail::timer_base<12, uint16_t, 0xffffu, mcutl::periph::timer12,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer12, interrupt::update> : types::identity<mcutl::interrupt::type::tim8_brk_tim12> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM12EN

#ifdef RCC_APB1ENR_TIM13EN
using timer13 = detail::timer_base<13, uint16_t, 0xffffu, mcutl::periph::timer13,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer13, interrupt::update> : types::identity<mcutl::interrupt::type::tim8_up_tim13> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM13EN

#ifdef RCC_APB1ENR_TIM14EN
using timer14 = detail::timer_base<14, uint16_t, 0xffffu, mcutl::periph::timer14,
	detail::timer_frequency_traits_2_3_4_5_6_7_12_13_14,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer14, interrupt::update> : types::identity<mcutl::interrupt::type::tim8_trg_com_tim14> {};

} //namespace detail
#endif //RCC_APB1ENR_TIM14EN

#ifdef RCC_APB2ENR_TIM1EN
using timer1 = detail::timer_base<1, uint16_t, 0xffffu, mcutl::periph::timer1,
	detail::timer_frequency_traits_1_8_9_10_11,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

#if defined(STM32F103xG) || defined(STM32F101xG) //XL-density
template<>
struct interrupt_type_helper<timer1, interrupt::update> : types::identity<mcutl::interrupt::type::tim1_up_tim10> {};
#else //XL-density
template<>
struct interrupt_type_helper<timer1, interrupt::update> : types::identity<mcutl::interrupt::type::tim1_up> {};
#endif //XL-density

} //namespace detail
#endif //RCC_APB2ENR_TIM1EN

#ifdef RCC_APB2ENR_TIM8EN
using timer8 = detail::timer_base<8, uint16_t, 0xffffu, mcutl::periph::timer8,
	detail::timer_frequency_traits_1_8_9_10_11,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

#if defined(STM32F103xG) || defined(STM32F101xG) //XL-density
template<>
struct interrupt_type_helper<timer8, interrupt::update> : types::identity<mcutl::interrupt::type::tim8_up_tim13> {};
#else //XL-density
template<>
struct interrupt_type_helper<timer8, interrupt::update> : types::identity<mcutl::interrupt::type::tim8_up> {};
#endif //XL-density

} //namespace detail
#endif //RCC_APB2ENR_TIM8EN

#ifdef RCC_APB2ENR_TIM9EN
using timer9 = detail::timer_base<9, uint16_t, 0xffffu, mcutl::periph::timer9,
	detail::timer_frequency_traits_1_8_9_10_11,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer9, interrupt::update> : types::identity<mcutl::interrupt::type::tim1_brk_tim9> {};

} //namespace detail
#endif //RCC_APB2ENR_TIM9EN

#ifdef RCC_APB2ENR_TIM10EN
using timer10 = detail::timer_base<10, uint16_t, 0xffffu, mcutl::periph::timer10,
	detail::timer_frequency_traits_1_8_9_10_11,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer10, interrupt::update> : types::identity<mcutl::interrupt::type::tim1_up_tim10> {};

} //namespace detail
#endif //RCC_APB2ENR_TIM10EN

#ifdef RCC_APB2ENR_TIM11EN
using timer11 = detail::timer_base<11, uint16_t, 0xffffu, mcutl::periph::timer11,
	detail::timer_frequency_traits_1_8_9_10_11,
	detail::prescaler_traits, detail::reload_limits>;
namespace detail
{

template<>
struct interrupt_type_helper<timer11, interrupt::update> : types::identity<mcutl::interrupt::type::tim1_trg_com_tim11> {};

} //namespace detail
#endif //RCC_APB2ENR_TIM11EN

} //namespace mcutl::timer

namespace mcutl::device::timer
{

struct update_request_source
{
	enum value : uint8_t
	{
		overflow,
		overflow_ug_bit_slave_controller
	};
};

struct master_mode
{
	enum value : uint8_t
	{
		none,
		output_enable,
		output_update
	};
};

struct options : mcutl::timer::detail::options
{
	update_request_source::value update_request_source_type
		= update_request_source::overflow_ug_bit_slave_controller;
	bool disable_update = false;
	master_mode::value master = master_mode::none;
	
	uint32_t update_request_source_set_count = 0;
	uint32_t disable_update_set_count = 0;
	uint32_t trigger_registers_update_set_count = 0;
	uint32_t master_set_count = 0;
};

} // namespace mcutl::device::timer

namespace mcutl::timer::detail
{

template<typename Timer>
struct options_parser<Timer, update_request_source::overflow>
	: opts::base_option_parser<device::timer::update_request_source::overflow,
	&device::timer::options::update_request_source_type,
	&device::timer::options::update_request_source_set_count> {};
template<typename Timer>
struct options_parser<Timer, update_request_source::overflow_ug_bit_slave_controller>
	: opts::base_option_parser<device::timer::update_request_source::overflow_ug_bit_slave_controller,
	&device::timer::options::update_request_source_type,
	&device::timer::options::update_request_source_set_count> {};

template<typename Timer, bool Disable>
struct options_parser<Timer, disable_update<Disable>>
	: opts::base_option_parser<Disable,
	&device::timer::options::disable_update,
	&device::timer::options::disable_update_set_count> {};

template<typename Timer>
struct options_parser<Timer, trigger_registers_update>
	: opts::base_option_parser<0, nullptr,
	&device::timer::options::trigger_registers_update_set_count> {};

template<typename Timer>
struct options_parser<Timer, master_mode::none>
	: opts::base_option_parser<device::timer::master_mode::none,
	&device::timer::options::master,
	&device::timer::options::master_set_count> {};
template<typename Timer>
struct options_parser<Timer, master_mode::output_enable>
	: opts::base_option_parser<device::timer::master_mode::output_enable,
	&device::timer::options::master,
	&device::timer::options::master_set_count> {};
template<typename Timer>
struct options_parser<Timer, master_mode::output_update>
	: opts::base_option_parser<device::timer::master_mode::output_update,
	&device::timer::options::master,
	&device::timer::options::master_set_count> {};

} //namespace mcutl::timer::detail

namespace mcutl::device::timer
{

template<typename Timer>
constexpr uint32_t no_timer_assert() noexcept
{
	static_assert(types::always_false<Timer>::value, "Unsupported timer");
	return 0;
}

template<typename Timer>
constexpr uint32_t get_timer_register() noexcept
{
	if constexpr (false)
	{}
#ifdef TIM1_BASE
	else if constexpr (Timer::index == 1)
		return TIM1_BASE;
#endif //TIM1_BASE
#ifdef TIM2_BASE
	else if constexpr (Timer::index == 2)
		return TIM2_BASE;
#endif //TIM2_BASE
#ifdef TIM3_BASE
	else if constexpr (Timer::index == 3)
		return TIM3_BASE;
#endif //TIM3_BASE
#ifdef TIM4_BASE
	else if constexpr (Timer::index == 4)
		return TIM4_BASE;
#endif //TIM4_BASE
#ifdef TIM5_BASE
	else if constexpr (Timer::index == 5)
		return TIM5_BASE;
#endif //TIM5_BASE
#ifdef TIM6_BASE
	else if constexpr (Timer::index == 6)
		return TIM6_BASE;
#endif //TIM6_BASE
#ifdef TIM7_BASE
	else if constexpr (Timer::index == 7)
		return TIM7_BASE;
#endif //TIM7_BASE
#ifdef TIM8_BASE
	else if constexpr (Timer::index == 8)
		return TIM8_BASE;
#endif //TIM8_BASE
#ifdef TIM9_BASE
	else if constexpr (Timer::index == 9)
		return TIM9_BASE;
#endif //TIM9_BASE
#ifdef TIM10_BASE
	else if constexpr (Timer::index == 10)
		return TIM10_BASE;
#endif //TIM10_BASE
#ifdef TIM11_BASE
	else if constexpr (Timer::index == 11)
		return TIM11_BASE;
#endif //TIM11_BASE
#ifdef TIM12_BASE
	else if constexpr (Timer::index == 12)
		return TIM12_BASE;
#endif //TIM12
#ifdef TIM13_BASE
	else if constexpr (Timer::index == 13)
		return TIM13_BASE;
#endif //TIM13
#ifdef TIM14_BASE
	else if constexpr (Timer::index == 14)
		return TIM14_BASE;
#endif //TIM14_BASE
	else
		return no_timer_assert<Timer>();
}

template<typename Timer>
auto get_timer_count() MCUTL_NOEXCEPT
{
	constexpr auto timer_reg_base = get_timer_register<Timer>();
	return mcutl::memory::get_register_bits<&TIM_TypeDef::CNT, timer_reg_base>();
}

template<typename Timer>
void set_timer_count(uint16_t value) MCUTL_NOEXCEPT
{
	constexpr auto timer_reg_base = get_timer_register<Timer>();
	mcutl::memory::set_register_value<&TIM_TypeDef::CNT, timer_reg_base>(value);
}

template<typename Interrupt>
struct interrupt_flag
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown timer interrupt type");
};

template<>
struct interrupt_flag<mcutl::timer::interrupt::update>
{
	static constexpr uint32_t value = TIM_SR_UIF;
};

template<typename Timer, typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = (0u | ... | interrupt_flag<Interrupts>::value);

template<typename Timer, typename... Interrupts>
void clear_pending_flags() MCUTL_NOEXCEPT
{
	constexpr uint32_t flags = pending_flags_v<Timer, Interrupts...>;
	if constexpr (flags != 0)
	{
		constexpr uint32_t all_flags
			= TIM_SR_CC4OF | TIM_SR_CC3OF | TIM_SR_CC2OF | TIM_SR_CC1OF
			| TIM_SR_TIF | TIM_SR_CC4IF | TIM_SR_CC3IF | TIM_SR_CC2IF | TIM_SR_CC1IF
			| TIM_SR_UIF;
		constexpr auto timer_reg_base = get_timer_register<Timer>();
		mcutl::memory::set_register_value<all_flags & ~flags,
			&TIM_TypeDef::SR, timer_reg_base>();
	}
}

template<typename Timer, typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	clear_pending_flags<Timer, Interrupts...>();
}

template<typename Timer, typename... Interrupts>
uint32_t get_pending_flags() MCUTL_NOEXCEPT
{
	constexpr auto timer_reg_base = get_timer_register<Timer>();
	return mcutl::memory::get_register_bits<pending_flags_v<Timer, Interrupts...>,
		&TIM_TypeDef::SR, timer_reg_base>();
}

struct general_purpose_timer_registers
{
	uint32_t cr1 = 0;
	uint32_t cr2 = 0;
	uint32_t cr1_mask = 0;
	uint32_t cr2_mask = 0;
};

template<typename OptionsLambda>
constexpr general_purpose_timer_registers get_general_purpose_registers(
	OptionsLambda options_lambda) noexcept
{
	general_purpose_timer_registers result {};
	
	constexpr auto options = options_lambda();
	
	if constexpr (!!options.count_mode_set_count)
	{
		result.cr1_mask |= TIM_CR1_DIR_Msk;
		if constexpr (options.mode == mcutl::timer::detail::count_mode::count_down)
			result.cr1 |= TIM_CR1_DIR;
	}
	
	if constexpr (!!options.stop_on_overflow_set_count)
	{
		result.cr1_mask |= TIM_CR1_OPM_Msk;
		if constexpr (options.stop_on_overflow)
			result.cr1 |= TIM_CR1_OPM;
	}
	
	if constexpr (!!options.update_request_source_set_count)
	{
		result.cr1_mask |= TIM_CR1_URS_Msk;
		if constexpr (options.update_request_source_type == update_request_source::overflow)
			result.cr1 |= TIM_CR1_URS;
	}
	
	if constexpr (!!options.buffer_reload_value_set_count)
	{
		result.cr1_mask |= TIM_CR1_ARPE_Msk;
		if constexpr (options.buffer_reload_value)
			result.cr1 |= TIM_CR1_ARPE;
	}
	
	if constexpr (!!options.disable_update_set_count)
	{
		result.cr1_mask |= TIM_CR1_UDIS_Msk;
		if constexpr (options.disable_update)
			result.cr1 |= TIM_CR1_UDIS;
	}
	
	if constexpr (!!options.master_set_count)
	{
		result.cr2_mask |= TIM_CR2_MMS_Msk;
		if constexpr (options.master == master_mode::output_enable)
			result.cr2 |= TIM_CR2_MMS_0;
		else if constexpr (options.master == master_mode::output_update)
			result.cr2 |= TIM_CR2_MMS_1;
	}
	
	return result;
}

template<typename Timer, typename OptionsLambda>
void configure_general_purpose_timer(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto options = options_lambda();
	constexpr auto registers = get_general_purpose_registers(options_lambda);
	constexpr auto timer_reg_base = get_timer_register<Timer>();
	
	if constexpr (!!options.enable_peripheral_set_count)
	{
		if constexpr (options.enable_peripheral)
		{
			mcutl::periph::configure_peripheral<
				mcutl::periph::enable<mcutl::timer::peripheral_type<Timer>>>();
		}
	}
	
	if constexpr (!options.base_configuration_set_count)
	{
		//This call also disables the timer in case it was enabled before
		mcutl::memory::set_register_value<registers.cr1, &TIM_TypeDef::CR1, timer_reg_base>();
		mcutl::memory::set_register_value<registers.cr2, &TIM_TypeDef::CR2, timer_reg_base>();
	}
	else
	{
		if constexpr (!!registers.cr1)
			mcutl::memory::set_register_value<registers.cr1, &TIM_TypeDef::CR1, timer_reg_base>();
		if constexpr (!!registers.cr2)
			mcutl::memory::set_register_value<registers.cr2, &TIM_TypeDef::CR2, timer_reg_base>();
	}
	
	if constexpr (options.prescaler_set_count && options.prescaler > 1u)
	{
		static_assert(options.prescaler != mcutl::timer::detail::unset_value);
		mcutl::memory::set_register_value<static_cast<uint32_t>(options.prescaler - 1),
			&TIM_TypeDef::PSC, timer_reg_base>();
	}
	else if (!options.base_configuration_set_count)
	{
		mcutl::memory::set_register_value<0u, &TIM_TypeDef::PSC, timer_reg_base>();
	}
	
	if constexpr (!!options.reload_value_set_count)
	{
		static_assert(options.reload_value != mcutl::timer::detail::unset_value);
		mcutl::memory::set_register_value<static_cast<uint32_t>(options.reload_value - 1),
			&TIM_TypeDef::ARR, timer_reg_base>();
	}
	else if (!options.base_configuration_set_count)
	{
		mcutl::memory::set_register_value<0xffffu, &TIM_TypeDef::ARR, timer_reg_base>();
	}
	
	if constexpr (!!options.trigger_registers_update_set_count)
		mcutl::memory::set_register_value<TIM_EGR_UG, &TIM_TypeDef::EGR, timer_reg_base>();
	
	if constexpr (options.overflow_set_count && !options.overflow.disable)
		mcutl::memory::set_register_value<TIM_DIER_UIE, &TIM_TypeDef::DIER, timer_reg_base>();
	else if (!options.base_configuration_set_count)
		mcutl::memory::set_register_value<0u, &TIM_TypeDef::DIER, timer_reg_base>();
	
	if constexpr (options.enable_controller_interrupts_set_count
		&& options.overflow_set_count && !options.overflow.disable)
	{
		mcutl::interrupt::enable<
			mcutl::interrupt::interrupt<
				mcutl::timer::interrupt_type<Timer, mcutl::timer::interrupt::overflow>,
				options.overflow.priority,
				options.overflow.subpriority
			>,
			options.priority_count_set_count ? options.priority_count : mcutl::interrupt::maximum_priorities
		>();
	}
	
	if constexpr (options.disable_controller_interrupts_set_count
		&& options.overflow_set_count && options.overflow.disable)
	{
		mcutl::interrupt::disable<
			mcutl::timer::interrupt_type<Timer, mcutl::timer::interrupt::overflow>
		>();
	}
	
	if constexpr (options.enable_set_count && options.enable)
	{
		mcutl::memory::set_register_value<registers.cr1 | TIM_CR1_CEN,
			&TIM_TypeDef::CR1, timer_reg_base>();
	}
	
	if constexpr (!!options.enable_peripheral_set_count)
	{
		if constexpr (!options.enable_peripheral)
		{
			static_assert(!options.enable, "Can not enable timer while disabling its peripheral");
			
			mcutl::periph::configure_peripheral<
				mcutl::periph::disable<mcutl::timer::peripheral_type<Timer>>>();
		}
	}
}

template<typename Timer, typename OptionsLambda>
void reconfigure_general_purpose_timer(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto options = options_lambda();
	constexpr auto registers = get_general_purpose_registers(options_lambda);
	constexpr auto timer_reg_base = get_timer_register<Timer>();
	
	static_assert(!options.base_configuration_set_count,
		"base_configuration_is_currently_present makes no sense when used with timer reconfigure()");
	
	if constexpr (!!options.enable_peripheral_set_count)
	{
		if constexpr (options.enable_peripheral)
		{
			mcutl::periph::configure_peripheral<
				mcutl::periph::enable<mcutl::timer::peripheral_type<Timer>>>();
		}
	}
	
	[[maybe_unused]] uint32_t prev_cr1;
	if constexpr ((registers.cr1_mask & TIM_CR1_DIR) != 0)
	{
		prev_cr1 = mcutl::memory::get_register_bits<&TIM_TypeDef::CR1, timer_reg_base>();
		prev_cr1 &= ~registers.cr1_mask;
		prev_cr1 |= registers.cr1;
		mcutl::memory::set_register_value<&TIM_TypeDef::CR1, timer_reg_base>(prev_cr1 & ~TIM_CR1_CEN);
	}
	else
	{
		mcutl::memory::set_register_bits<registers.cr1_mask, registers.cr1,
			&TIM_TypeDef::CR1, timer_reg_base>();
	}
	
	mcutl::memory::set_register_bits<registers.cr2_mask, registers.cr2, &TIM_TypeDef::CR2, timer_reg_base>();
	
	if constexpr (!!options.prescaler_set_count)
	{
		static_assert(options.prescaler != mcutl::timer::detail::unset_value);
		mcutl::memory::set_register_value<static_cast<uint32_t>(options.prescaler - 1),
			&TIM_TypeDef::PSC, timer_reg_base>();
	}
	
	if constexpr (!!options.reload_value_set_count)
	{
		static_assert(options.reload_value != mcutl::timer::detail::unset_value);
		mcutl::memory::set_register_value<static_cast<uint32_t>(options.reload_value - 1),
			&TIM_TypeDef::ARR, timer_reg_base>();
	}
	
	if constexpr (!!options.trigger_registers_update_set_count)
		mcutl::memory::set_register_value<TIM_EGR_UG, &TIM_TypeDef::EGR, timer_reg_base>();
	
	[[maybe_unused]] uint32_t dier;
	if constexpr (options.overflow_set_count
		|| options.enable_controller_interrupts_set_count
		|| options.disable_controller_interrupts_set_count)
	{
		dier = mcutl::memory::get_register_bits<&TIM_TypeDef::DIER, timer_reg_base>();
		if constexpr (!!options.overflow_set_count)
		{
			dier &= ~TIM_DIER_UIE_Msk;
			if constexpr (!options.overflow.disable)
				dier |= TIM_DIER_UIE;
			
			mcutl::memory::set_register_value<&TIM_TypeDef::DIER, timer_reg_base>(dier);
		}
	}
	
	if constexpr (!!options.enable_controller_interrupts_set_count)
	{
		if (dier & TIM_DIER_UIE)
		{
			mcutl::interrupt::enable<
				mcutl::interrupt::interrupt<
					mcutl::timer::interrupt_type<Timer, mcutl::timer::interrupt::overflow>,
					options.overflow.priority,
					options.overflow.subpriority
				>,
				options.priority_count_set_count ? options.priority_count : mcutl::interrupt::maximum_priorities
			>();
		}
	}
	
	if constexpr (!!options.disable_controller_interrupts_set_count)
	{
		if (!(dier & TIM_DIER_UIE))
		{
			mcutl::interrupt::disable<
				mcutl::timer::interrupt_type<Timer, mcutl::timer::interrupt::overflow>
			>();
		}
	}
	
	if constexpr (!!options.enable_set_count)
	{
		if constexpr (options.enable)
		{
			mcutl::memory::set_register_bits<TIM_CR1_CEN_Msk, TIM_CR1_CEN,
				&TIM_TypeDef::CR1, timer_reg_base>();
		}
		else
		{
			mcutl::memory::set_register_bits<TIM_CR1_CEN_Msk, ~TIM_CR1_CEN,
				&TIM_TypeDef::CR1, timer_reg_base>();
		}
	}
	else if constexpr ((registers.cr1_mask & TIM_CR1_DIR) != 0)
	{
		if (prev_cr1 & TIM_CR1_CEN)
			mcutl::memory::set_register_value<&TIM_TypeDef::CR1, timer_reg_base>(prev_cr1);
	}
	
	if constexpr (!!options.enable_peripheral_set_count)
	{
		if constexpr (!options.enable_peripheral)
		{
			static_assert(!options.enable, "Can not enable timer while disabling its peripheral");
			
			mcutl::periph::configure_peripheral<
				mcutl::periph::disable<mcutl::timer::peripheral_type<Timer>>>();
		}
	}
}

template<typename Timer, typename OptionsLambda>
void configure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	if constexpr (Timer::index >= 2 && Timer::index <= 5)
		configure_general_purpose_timer<Timer>(options_lambda);
	
	static_assert(Timer::index >= 2 && Timer::index <= 5,
		"Selected timer is not supported");
}

template<typename Timer, typename OptionsLambda>
void reconfigure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	if constexpr (Timer::index >= 2 && Timer::index <= 5)
		reconfigure_general_purpose_timer<Timer>(options_lambda);
	
	static_assert(Timer::index >= 2 && Timer::index <= 5,
		"Selected timer is not supported");
}

template<typename Timer>
[[maybe_unused]] constexpr bool supports_stop_on_overflow = true;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_prescalers = true;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_reload_value = true;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_reload_value_buffer = true;
template<typename Timer>
[[maybe_unused]] constexpr auto default_count_direction = mcutl::timer::count_direction::up;
template<typename Timer>
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags = true;

} //namespace mcutl::device::timer
