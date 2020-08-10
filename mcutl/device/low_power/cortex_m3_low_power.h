#pragma once

#include <type_traits>

#include "mcutl/device/device.h"
#include "mcutl/instruction/instruction.h"
#include "mcutl/low_power/low_power_defs.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/class_limitations.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::low_power::options
{

struct sleep_on_exit : types::static_class {};
struct event_on_pending_interrupt : types::static_class {};

} //namespace mcutl::low_power::options

namespace mcutl::device::low_power
{

[[maybe_unused]] constexpr bool supports_wait_for_event = true;
[[maybe_unused]] constexpr bool supports_wait_for_interrupt = true;

struct cortex_m3_low_power_options : mcutl::low_power::detail::low_power_options_base
{
	uint32_t sleep_on_exit_set_count = 0;
	uint32_t event_on_pending_interrupt_set_count = 0;
};

struct scr_reg_value
{
	uint32_t value = 0;
	uint32_t mask = 0;
};

template<bool DeepSleep, typename WakeUpMode, typename OptionsLambda>
constexpr scr_reg_value get_scr_reg(OptionsLambda options_lambda) noexcept
{
	scr_reg_value result {};
	constexpr auto options = options_lambda();
	result.mask = SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk | SCB_SCR_SEVONPEND_Msk;
	
	if constexpr (DeepSleep)
		result.value |= SCB_SCR_SLEEPDEEP_Msk;
	
	static_assert(!options.event_on_pending_interrupt_set_count
		|| std::is_same_v<WakeUpMode, mcutl::low_power::wakeup_mode::wait_for_event>,
		"event_on_pending_interrupt (SEVONPEND) is compatible with wait_for_event (WFE) only");
	
	if constexpr (options.event_on_pending_interrupt_set_count != 0)
		result.value |= SCB_SCR_SEVONPEND_Msk;
	
	static_assert(!DeepSleep || !options.sleep_on_exit_set_count, 
		"Deep sleep modes are not compatible with SLEEPONEXIT");
	
	if constexpr (options.sleep_on_exit_set_count != 0)
		result.value |= SCB_SCR_SLEEPONEXIT_Msk;
	
	return result;
}

template<bool DeepSleep, typename WakeUpMode, typename OptionsLambda>
inline void reset_sleep_flags(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto scr = get_scr_reg<DeepSleep, WakeUpMode>(options_lambda);
	if constexpr (!!scr.value)
		mcutl::memory::set_register_bits<scr.value, 0, &SCB_Type::SCR, SCB_BASE>();
}

template<bool DeepSleep, typename WakeUpMode, typename OptionsLambda>
inline void execute_sleep(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto scr = get_scr_reg<DeepSleep, WakeUpMode>(options_lambda);
	mcutl::memory::set_register_bits<scr.mask, scr.value, &SCB_Type::SCR, SCB_BASE>();
	mcutl::instruction::execute<mcutl::device::instruction::type::dsb>();
	mcutl::instruction::execute<mcutl::device::instruction::type::isb>();
	
	static constexpr auto options = options_lambda();
	
	static_assert(std::is_same_v<WakeUpMode, mcutl::low_power::wakeup_mode::wait_for_event>
		|| std::is_same_v<WakeUpMode, mcutl::low_power::wakeup_mode::wait_for_interrupt>,
		"Unsupported wakeup mode");
		
	static_assert(!options.sleep_on_exit_set_count
		|| std::is_same_v<WakeUpMode, mcutl::low_power::wakeup_mode::wait_for_interrupt>,
		"sleep_on_exit only works with wait_for_interrupt wakeup mode");
	
	if constexpr (!options.sleep_on_exit_set_count)
	{
		//To debug Stop mode with WFE entry,
		//the WFE instruction must be inside a dedicated function with 1 instruction
		//(NOP) between the execution of the WFE and the BX LR.
		if constexpr (std::is_same_v<WakeUpMode, mcutl::low_power::wakeup_mode::wait_for_event>)
			mcutl::instruction::execute<mcutl::device::instruction::type::wfe>();
		else //WFI
			mcutl::instruction::execute<mcutl::device::instruction::type::wfi>();
	}
}

} //namespace mcutl::device::low_power

namespace mcutl::low_power::detail
{

template<>
struct options_parser<options::sleep_on_exit>
	: opts::base_option_parser<0, nullptr,
	&device::low_power::cortex_m3_low_power_options::sleep_on_exit_set_count> {};
template<>
struct options_parser<options::event_on_pending_interrupt>
	: opts::base_option_parser<0, nullptr,
	&device::low_power::cortex_m3_low_power_options::event_on_pending_interrupt_set_count> {};

} //namespace mcutl::low_power::detail
