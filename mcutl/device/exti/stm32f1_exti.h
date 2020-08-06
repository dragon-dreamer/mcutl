#pragma once

#include <array>
#include <stdint.h>
#include <type_traits>

#include "mcutl/device/device.h"
#include "mcutl/exti/exti_defs.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::exti
{

namespace detail
{

constexpr uint32_t pvd_output_line_index = 16;
constexpr uint32_t rtc_alarm_line_index = 17;
constexpr uint32_t usb_wakeup_line_index = 18;
#ifdef EXTI_PR_PR19 //Connectivity-line devices only
constexpr uint32_t ethernet_wakeup_line_index = 19;
#endif //EXTI_PR_PR19

} //namespace detail

using pvd_output_line = line<detail::pvd_output_line_index>;
using rtc_alarm_line = line<detail::rtc_alarm_line_index>;
using usb_wakeup_line = line<detail::usb_wakeup_line_index>;

#ifdef EXTI_PR_PR19 //Connectivity-line devices only
using ethernet_wakeup_line = line<detail::ethernet_wakeup_line_index>;
#endif //EXTI_PR_PR19

} //namespace mcutl::exti

namespace mcutl::exti::detail
{

template<> struct line_to_interrupt<line<0>> { using type = interrupt::type::exti0; };
template<> struct line_to_interrupt<line<1>> { using type = interrupt::type::exti1; };
template<> struct line_to_interrupt<line<2>> { using type = interrupt::type::exti2; };
template<> struct line_to_interrupt<line<3>> { using type = interrupt::type::exti3; };
template<> struct line_to_interrupt<line<4>> { using type = interrupt::type::exti4; };
template<> struct line_to_interrupt<line<5>> { using type = interrupt::type::exti9_5; };
template<> struct line_to_interrupt<line<6>> { using type = interrupt::type::exti9_5; };
template<> struct line_to_interrupt<line<7>> { using type = interrupt::type::exti9_5; };
template<> struct line_to_interrupt<line<8>> { using type = interrupt::type::exti9_5; };
template<> struct line_to_interrupt<line<9>> { using type = interrupt::type::exti9_5; };
template<> struct line_to_interrupt<line<10>> { using type = interrupt::type::exti15_10; };
template<> struct line_to_interrupt<line<11>> { using type = interrupt::type::exti15_10; };
template<> struct line_to_interrupt<line<12>> { using type = interrupt::type::exti15_10; };
template<> struct line_to_interrupt<line<13>> { using type = interrupt::type::exti15_10; };
template<> struct line_to_interrupt<line<14>> { using type = interrupt::type::exti15_10; };
template<> struct line_to_interrupt<line<15>> { using type = interrupt::type::exti15_10; };
template<> struct line_to_interrupt<line<pvd_output_line_index>> { using type = interrupt::type::pvd; };
template<> struct line_to_interrupt<line<rtc_alarm_line_index>> { using type = interrupt::type::rtc_alarm; };
#ifdef EXTI_PR_PR19 //Connectivity-line devices only
template<> struct line_to_interrupt<line<usb_wakeup_line_index>> { using type = interrupt::type::otg_fs_wkup; };
template<> struct line_to_interrupt<line<ethernet_wakeup_line_index>> { using type = interrupt::type::eth_wkup; };
#else //Other devices
template<> struct line_to_interrupt<line<usb_wakeup_line_index>> { using type = interrupt::type::usb_wakeup; };
#endif //EXTI_PR_PR19

} //namespace mcutl::exti::detail

namespace mcutl::device::exti
{

[[maybe_unused]] constexpr bool has_events = true;
[[maybe_unused]] constexpr bool has_rising_trigger = true;
[[maybe_unused]] constexpr bool has_falling_trigger = true;
[[maybe_unused]] constexpr bool has_rising_and_falling_trigger = true;
[[maybe_unused]] constexpr bool has_software_trigger = true;
[[maybe_unused]] constexpr bool has_pending_line_bits = true;
[[maybe_unused]] constexpr bool has_atomic_clear_pending_line_bits = true;
[[maybe_unused]] constexpr bool has_atomic_software_trigger = true;

[[maybe_unused]] constexpr std::array present_exti_lines {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
#ifdef EXTI_PR_PR19 //EXTI line 19 is available only in connectivity line devices
	, 1
#endif //EXTI_PR_PR19
};

template<uint32_t EmrBits, uint32_t EmrMask, uint32_t ImrBits, uint32_t ImrMask,
	uint32_t FtsrBits, uint32_t FtsrMask, uint32_t RtsrBits, uint32_t RtsrMask,
	uint32_t PrBits, uint32_t SwierBits, IRQn_Type Irq>
struct exti_bits
{
	static constexpr uint32_t emr_bits = EmrBits;
	static constexpr uint32_t emr_mask = EmrMask;
	static constexpr uint32_t imr_bits = ImrBits;
	static constexpr uint32_t imr_mask = ImrMask;
	static constexpr uint32_t ftsr_bits = FtsrBits;
	static constexpr uint32_t ftsr_mask = FtsrMask;
	static constexpr uint32_t rtsr_bits = RtsrBits;
	static constexpr uint32_t rtsr_mask = RtsrMask;
	
	static constexpr uint32_t pr_bits = PrBits;
	static constexpr uint32_t swier_bits = SwierBits;
	static constexpr IRQn_Type irqn = Irq;
};

struct exti_config
{
	uint32_t emr_bits = 0;
	uint32_t emr_mask = 0;
	uint32_t imr_bits = 0;
	uint32_t imr_mask = 0;
	uint32_t ftsr_bits = 0;
	uint32_t ftsr_mask = 0;
	uint32_t rtsr_bits = 0;
	uint32_t rtsr_mask = 0;
};

template<uint32_t Line>
struct exti_bits_by_line
{
	static_assert(types::value_always_false<Line>::value, "Unsupported EXTI line");
};

template<>
struct exti_bits_by_line<0> : exti_bits<EXTI_EMR_MR0, EXTI_EMR_MR0_Msk, EXTI_IMR_MR0, EXTI_IMR_MR0_Msk,
	EXTI_FTSR_TR0, EXTI_FTSR_TR0_Msk, EXTI_RTSR_TR0, EXTI_RTSR_TR0_Msk, EXTI_PR_PR0, EXTI_SWIER_SWIER0,
	EXTI0_IRQn> {};

template<>
struct exti_bits_by_line<1> : exti_bits<EXTI_EMR_MR1, EXTI_EMR_MR1_Msk, EXTI_IMR_MR1, EXTI_IMR_MR1_Msk,
	EXTI_FTSR_TR1, EXTI_FTSR_TR1_Msk, EXTI_RTSR_TR1, EXTI_RTSR_TR1_Msk, EXTI_PR_PR1, EXTI_SWIER_SWIER1,
	EXTI1_IRQn> {};

template<>
struct exti_bits_by_line<2> : exti_bits<EXTI_EMR_MR2, EXTI_EMR_MR2_Msk, EXTI_IMR_MR2, EXTI_IMR_MR2_Msk,
	EXTI_FTSR_TR2, EXTI_FTSR_TR2_Msk, EXTI_RTSR_TR2, EXTI_RTSR_TR2_Msk, EXTI_PR_PR2, EXTI_SWIER_SWIER2,
	EXTI2_IRQn> {};

template<>
struct exti_bits_by_line<3> : exti_bits<EXTI_EMR_MR3, EXTI_EMR_MR3_Msk, EXTI_IMR_MR3, EXTI_IMR_MR3_Msk,
	EXTI_FTSR_TR3, EXTI_FTSR_TR3_Msk, EXTI_RTSR_TR3, EXTI_RTSR_TR3_Msk, EXTI_PR_PR3, EXTI_SWIER_SWIER3,
	EXTI3_IRQn> {};

template<>
struct exti_bits_by_line<4> : exti_bits<EXTI_EMR_MR4, EXTI_EMR_MR4_Msk, EXTI_IMR_MR4, EXTI_IMR_MR4_Msk,
	EXTI_FTSR_TR4, EXTI_FTSR_TR4_Msk, EXTI_RTSR_TR4, EXTI_RTSR_TR4_Msk, EXTI_PR_PR4, EXTI_SWIER_SWIER4,
	EXTI4_IRQn> {};

template<>
struct exti_bits_by_line<5> : exti_bits<EXTI_EMR_MR5, EXTI_EMR_MR5_Msk, EXTI_IMR_MR5, EXTI_IMR_MR5_Msk,
	EXTI_FTSR_TR5, EXTI_FTSR_TR5_Msk, EXTI_RTSR_TR5, EXTI_RTSR_TR5_Msk, EXTI_PR_PR5, EXTI_SWIER_SWIER5,
	EXTI9_5_IRQn> {};

template<>
struct exti_bits_by_line<6> : exti_bits<EXTI_EMR_MR6, EXTI_EMR_MR6_Msk, EXTI_IMR_MR6, EXTI_IMR_MR6_Msk,
	EXTI_FTSR_TR6, EXTI_FTSR_TR6_Msk, EXTI_RTSR_TR6, EXTI_RTSR_TR6_Msk, EXTI_PR_PR6, EXTI_SWIER_SWIER6,
	EXTI9_5_IRQn> {};

template<>
struct exti_bits_by_line<7> : exti_bits<EXTI_EMR_MR7, EXTI_EMR_MR7_Msk, EXTI_IMR_MR7, EXTI_IMR_MR7_Msk,
	EXTI_FTSR_TR7, EXTI_FTSR_TR7_Msk, EXTI_RTSR_TR7, EXTI_RTSR_TR7_Msk, EXTI_PR_PR7, EXTI_SWIER_SWIER7,
	EXTI9_5_IRQn> {};

template<>
struct exti_bits_by_line<8> : exti_bits<EXTI_EMR_MR8, EXTI_EMR_MR8_Msk, EXTI_IMR_MR8, EXTI_IMR_MR8_Msk,
	EXTI_FTSR_TR8, EXTI_FTSR_TR8_Msk, EXTI_RTSR_TR8, EXTI_RTSR_TR8_Msk, EXTI_PR_PR8, EXTI_SWIER_SWIER8,
	EXTI9_5_IRQn> {};

template<>
struct exti_bits_by_line<9> : exti_bits<EXTI_EMR_MR9, EXTI_EMR_MR9_Msk, EXTI_IMR_MR9, EXTI_IMR_MR9_Msk,
	EXTI_FTSR_TR9, EXTI_FTSR_TR9_Msk, EXTI_RTSR_TR9, EXTI_RTSR_TR9_Msk, EXTI_PR_PR9, EXTI_SWIER_SWIER9,
	EXTI9_5_IRQn> {};

template<>
struct exti_bits_by_line<10> : exti_bits<EXTI_EMR_MR10, EXTI_EMR_MR10_Msk, EXTI_IMR_MR10, EXTI_IMR_MR10_Msk,
	EXTI_FTSR_TR10, EXTI_FTSR_TR10_Msk, EXTI_RTSR_TR10, EXTI_RTSR_TR10_Msk, EXTI_PR_PR10, EXTI_SWIER_SWIER10,
	EXTI15_10_IRQn> {};

template<>
struct exti_bits_by_line<11> : exti_bits<EXTI_EMR_MR11, EXTI_EMR_MR11_Msk, EXTI_IMR_MR11, EXTI_IMR_MR11_Msk,
	EXTI_FTSR_TR11, EXTI_FTSR_TR11_Msk, EXTI_RTSR_TR11, EXTI_RTSR_TR11_Msk, EXTI_PR_PR11, EXTI_SWIER_SWIER11,
	EXTI15_10_IRQn> {};

template<>
struct exti_bits_by_line<12> : exti_bits<EXTI_EMR_MR12, EXTI_EMR_MR12_Msk, EXTI_IMR_MR12, EXTI_IMR_MR12_Msk,
	EXTI_FTSR_TR12, EXTI_FTSR_TR12_Msk, EXTI_RTSR_TR12, EXTI_RTSR_TR12_Msk, EXTI_PR_PR12, EXTI_SWIER_SWIER12,
	EXTI15_10_IRQn> {};

template<>
struct exti_bits_by_line<13> : exti_bits<EXTI_EMR_MR13, EXTI_EMR_MR13_Msk, EXTI_IMR_MR13, EXTI_IMR_MR13_Msk,
	EXTI_FTSR_TR13, EXTI_FTSR_TR13_Msk, EXTI_RTSR_TR13, EXTI_RTSR_TR13_Msk, EXTI_PR_PR13, EXTI_SWIER_SWIER13,
	EXTI15_10_IRQn> {};

template<>
struct exti_bits_by_line<14> : exti_bits<EXTI_EMR_MR14, EXTI_EMR_MR14_Msk, EXTI_IMR_MR14, EXTI_IMR_MR14_Msk,
	EXTI_FTSR_TR14, EXTI_FTSR_TR14_Msk, EXTI_RTSR_TR14, EXTI_RTSR_TR14_Msk, EXTI_PR_PR14, EXTI_SWIER_SWIER14,
	EXTI15_10_IRQn> {};

template<>
struct exti_bits_by_line<15> : exti_bits<EXTI_EMR_MR15, EXTI_EMR_MR15_Msk, EXTI_IMR_MR15, EXTI_IMR_MR15_Msk,
	EXTI_FTSR_TR15, EXTI_FTSR_TR15_Msk, EXTI_RTSR_TR15, EXTI_RTSR_TR15_Msk, EXTI_PR_PR15, EXTI_SWIER_SWIER15,
	EXTI15_10_IRQn> {};

template<>
struct exti_bits_by_line<16> : exti_bits<EXTI_EMR_MR16, EXTI_EMR_MR16_Msk, EXTI_IMR_MR16, EXTI_IMR_MR16_Msk,
	EXTI_FTSR_TR16, EXTI_FTSR_TR16_Msk, EXTI_RTSR_TR16, EXTI_RTSR_TR16_Msk, EXTI_PR_PR16, EXTI_SWIER_SWIER16,
	PVD_IRQn> {};

template<>
struct exti_bits_by_line<17> : exti_bits<EXTI_EMR_MR17, EXTI_EMR_MR17_Msk, EXTI_IMR_MR17, EXTI_IMR_MR17_Msk,
	EXTI_FTSR_TR17, EXTI_FTSR_TR17_Msk, EXTI_RTSR_TR17, EXTI_RTSR_TR17_Msk, EXTI_PR_PR17, EXTI_SWIER_SWIER17,
	RTC_Alarm_IRQn> {};

template<>
struct exti_bits_by_line<18> : exti_bits<EXTI_EMR_MR18, EXTI_EMR_MR18_Msk, EXTI_IMR_MR18, EXTI_IMR_MR18_Msk,
	EXTI_FTSR_TR18, EXTI_FTSR_TR18_Msk, EXTI_RTSR_TR18, EXTI_RTSR_TR18_Msk, EXTI_PR_PR18, EXTI_SWIER_SWIER18,
	OTG_FS_WKUP_IRQn> {};

#ifdef EXTI_EMR_MR19 //Connectivity line devices only
template<>
struct exti_bits_by_line<19> : exti_bits<EXTI_EMR_MR19, EXTI_EMR_MR19_Msk, EXTI_IMR_MR19, EXTI_IMR_MR19_Msk,
	EXTI_FTSR_TR19, EXTI_FTSR_TR19_Msk, EXTI_RTSR_TR19, EXTI_RTSR_TR19_Msk, EXTI_PR_PR19, EXTI_SWIER_SWIER19,
	ETH_WKUP_IRQn> {};
#endif //EXTI_EMR_MR19

template<typename LineOptions>
constexpr void fill_exti_config(exti_config& config) noexcept
{
	using config_type = exti_bits_by_line<LineOptions::line::value>;
	if constexpr (std::is_same_v<typename LineOptions::mode, mcutl::exti::line_mode::event>
		|| std::is_same_v<typename LineOptions::mode, mcutl::exti::line_mode::event_and_interrupt>)
	{
		config.emr_bits |= config_type::emr_bits;
	}
	
	if constexpr (std::is_same_v<typename LineOptions::mode, mcutl::exti::line_mode::interrupt>
		|| std::is_same_v<typename LineOptions::mode, mcutl::exti::line_mode::event_and_interrupt>)
	{
		config.imr_bits |= config_type::imr_bits;
	}
	
	if constexpr (std::is_same_v<typename LineOptions::trigger, mcutl::exti::line_trigger::falling>
		|| std::is_same_v<typename LineOptions::trigger, mcutl::exti::line_trigger::rising_and_falling>)
	{
		config.ftsr_bits |= config_type::ftsr_bits;
	}
	
	if constexpr (std::is_same_v<typename LineOptions::trigger, mcutl::exti::line_trigger::rising>
		|| std::is_same_v<typename LineOptions::trigger, mcutl::exti::line_trigger::rising_and_falling>)
	{
		config.rtsr_bits |= config_type::rtsr_bits;
	}
	
	config.emr_mask |= config_type::emr_mask;
	config.imr_mask |= config_type::imr_mask;
	config.ftsr_mask |= config_type::ftsr_mask;
	config.rtsr_mask |= config_type::rtsr_mask;
	
	static_assert(LineOptions::options::length == 0,
		"No additional options are supported by line_options");
}
	
template<typename... LineOptions>
constexpr exti_config get_exti_config() noexcept
{
	exti_config result {};
	(..., fill_exti_config<LineOptions>(result));
	return result;
}

template<typename... LineOptions>
void enable_lines() noexcept
{
	constexpr auto config = get_exti_config<LineOptions...>();
	mcutl::memory::set_register_bits<config.ftsr_mask, config.ftsr_bits, &EXTI_TypeDef::FTSR, EXTI_BASE>();
	mcutl::memory::set_register_bits<config.rtsr_mask, config.rtsr_bits, &EXTI_TypeDef::RTSR, EXTI_BASE>();
	mcutl::memory::set_register_bits<config.emr_mask, config.emr_bits, &EXTI_TypeDef::EMR, EXTI_BASE>();
	mcutl::memory::set_register_bits<config.imr_mask, config.imr_bits, &EXTI_TypeDef::IMR, EXTI_BASE>();
}

template<typename... LineOptions>
void disable_lines() noexcept
{
	constexpr auto config = get_exti_config<LineOptions...>();
	mcutl::memory::set_register_bits<config.ftsr_mask, 0, &EXTI_TypeDef::FTSR, EXTI_BASE>();
	mcutl::memory::set_register_bits<config.rtsr_mask, 0, &EXTI_TypeDef::RTSR, EXTI_BASE>();
	mcutl::memory::set_register_bits<config.emr_mask, 0, &EXTI_TypeDef::EMR, EXTI_BASE>();
	mcutl::memory::set_register_bits<config.imr_mask, 0, &EXTI_TypeDef::IMR, EXTI_BASE>();
}

template<typename Line>
constexpr uint32_t get_pending_line_bit() noexcept
{
	return exti_bits_by_line<Line::line::value>::pr_bits;
}

template<typename... Lines>
constexpr uint32_t get_pending_line_bits_impl() noexcept
{
	return (0 | ... | get_pending_line_bit<Lines>());
}

template<typename Line>
constexpr uint32_t get_software_trigger_bit() noexcept
{
	return exti_bits_by_line<Line::line::value>::swier_bits;
}

template<typename... Lines>
constexpr uint32_t get_software_trigger_bits() noexcept
{
	return (0 | ... | get_software_trigger_bit<Lines>());
}

template<typename... Lines>
void clear_pending_line_bits() noexcept
{
	constexpr auto pending_bits = get_pending_line_bits_impl<Lines...>();
	if constexpr (pending_bits != 0)
		mcutl::memory::set_register_value<pending_bits, &EXTI_TypeDef::PR, EXTI_BASE>();
}

template<typename... Lines>
inline void clear_pending_line_bits_atomic() noexcept
{
	clear_pending_line_bits<Lines...>();
}

template<typename... Lines>
uint32_t get_pending_line_bits() noexcept
{
	constexpr auto pending_bits = get_pending_line_bits_impl<Lines...>();
	return mcutl::memory::get_register_bits<pending_bits, &EXTI_TypeDef::PR, EXTI_BASE>();
}

template<typename... Lines>
void software_trigger() noexcept
{
	constexpr auto software_trigger_bits = get_software_trigger_bits<Lines...>();
	if constexpr (software_trigger_bits != 0)
		mcutl::memory::set_register_value<software_trigger_bits, &EXTI_TypeDef::SWIER, EXTI_BASE>();
}

template<typename... Lines>
inline void software_trigger_atomic() noexcept
{
	software_trigger<Lines...>();
}

template<typename Line>
constexpr uint32_t get_exti_line_bit_mask() noexcept
{
	return (1u << Line::line::value);
}

template<typename... Lines>
constexpr uint32_t get_exti_lines_bit_mask() noexcept
{
	return (0u | ... | get_exti_line_bit_mask<Lines>());
}

template<typename... Interrupt>
struct interrupt_requests {};

template<typename Interrupt, typename... OtherInterrupts>
constexpr bool is_duplicate_irq() noexcept
{
	return (... || (Interrupt::interrupt_type::interrupt_type::irqn
		== OtherInterrupts::interrupt_type::interrupt_type::irqn));
}

template<typename Interrupt, typename OtherInterrupt>
constexpr bool is_mismatching_priority() noexcept
{
	return Interrupt::interrupt_type::interrupt_type::irqn
		== OtherInterrupt::interrupt_type::interrupt_type::irqn
		&& (Interrupt::interrupt_type::priority != OtherInterrupt::interrupt_type::priority
			|| Interrupt::interrupt_type::subpriority != OtherInterrupt::interrupt_type::subpriority);
}

template<typename Interrupt, typename... OtherInterrupts>
constexpr bool is_mismatching_priorities() noexcept
{
	return (... || is_mismatching_priority<Interrupt, OtherInterrupts>());
}

template<bool CheckPriorities, typename... Interrupts>
constexpr auto add_interrupt_request(interrupt_requests<Interrupts...> requests) noexcept
{
	return requests;
}

template<bool CheckPriorities, typename FirstLineInterruptOptions,
	typename... LineInterruptOptions, typename... InterruptInfos>
constexpr auto add_interrupt_request(interrupt_requests<InterruptInfos...>) noexcept
{
	static_assert(FirstLineInterruptOptions::options::length == 0,
		"No additional options are supported by line_interrupt_options");
	
	static_assert(!CheckPriorities
		|| !is_mismatching_priorities<FirstLineInterruptOptions, LineInterruptOptions...>(),
		"Mismatching priorities for EXTI interrupts");
	if constexpr (!is_duplicate_irq<FirstLineInterruptOptions, InterruptInfos...>())
	{
		return add_interrupt_request<CheckPriorities, LineInterruptOptions...>(
			interrupt_requests<InterruptInfos..., FirstLineInterruptOptions>());
	}
	else
	{
		return add_interrupt_request<CheckPriorities, LineInterruptOptions...>(
			interrupt_requests<InterruptInfos...>());
	}
}

template<typename... InterruptInfos>
void clear_pending_interrupts(interrupt_requests<InterruptInfos...>) noexcept
{
	(..., mcutl::interrupt::clear_pending<typename InterruptInfos::interrupt_type>());
}

template<typename... LineInterruptOptions>
void clear_pending_interrupts() noexcept
{
	using requests = decltype(add_interrupt_request<false, LineInterruptOptions...>(
		interrupt_requests<>{}));
	clear_pending_interrupts(requests{});
}

template<typename... InterruptInfos>
void disable_interrupts(interrupt_requests<InterruptInfos...>) noexcept
{
	(..., mcutl::interrupt::disable<typename InterruptInfos::interrupt_type>());
}

template<typename... LineInterruptOptions>
void disable_interrupts() noexcept
{
	using requests = decltype(add_interrupt_request<false, LineInterruptOptions...>(
		interrupt_requests<>{}));
	disable_interrupts(requests{});
}

template<auto PriorityCount, typename... InterruptInfos>
void enable_interrupts(interrupt_requests<InterruptInfos...>) noexcept
{
	(..., mcutl::interrupt::enable<typename InterruptInfos::interrupt_type, PriorityCount>());
}

template<auto PriorityCount, typename... LineInterruptOptions>
void enable_interrupts() noexcept
{
	using requests = decltype(add_interrupt_request<true, LineInterruptOptions...>(
		interrupt_requests<>{}));
	enable_interrupts<PriorityCount>(requests{});
}

template<auto PriorityCount, typename... InterruptInfos>
void set_interrupt_prioritites(interrupt_requests<InterruptInfos...>) noexcept
{
	(..., interrupt::set_priority_optional<
		typename InterruptInfos::interrupt_type::interrupt_type,
		InterruptInfos::interrupt_type::priority,
		InterruptInfos::interrupt_type::subpriority,
		PriorityCount>());
}

template<auto PriorityCount, typename... LineInterruptOptions>
void set_interrupt_prioritites() noexcept
{
	using requests = decltype(add_interrupt_request<true, LineInterruptOptions...>(
		interrupt_requests<>{}));
	set_interrupt_prioritites<PriorityCount>(requests{});
}

} //namespace mcutl::device::exti
