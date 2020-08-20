#pragma once

#include <stdint.h>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::rtc
{

template<bool Enable>
struct enable {};

struct base_configuration_is_currently_present {};

namespace interrupt
{

struct alarm {};
struct second {};
struct overflow {};

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};

} //namespace interrupt

namespace clock
{

struct internal {};
struct external_crystal {};

template<uint32_t PrescalerValue>
struct prescaler {};

struct one_second_prescaler {};

} //namespace clock

namespace detail
{

template<typename Interrupt>
struct interrupt_type_helper
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown RTC interrupt type");
};

struct clock
{
	enum value { internal, external_crystal, end };
};

struct options
{
	clock::value clock_value = clock::internal;
	mcutl::interrupt::detail::interrupt_info alarm {};
	mcutl::interrupt::detail::interrupt_info second {};
	mcutl::interrupt::detail::interrupt_info overflow {};
	uint32_t prescaler = 0;
	bool one_second_prescaler = false;
	bool enable = false;
	uint64_t priority_count = 0;
	uint32_t prescaler_set_count = 0;
	uint32_t clock_set_count = 0;
	uint32_t alarm_set_count = 0;
	uint32_t second_set_count = 0;
	uint32_t overflow_set_count = 0;
	uint32_t enable_controller_interrupts_set_count = 0;
	uint32_t disable_controller_interrupts_set_count = 0;
	uint32_t priority_count_set_count = 0;
	uint32_t enable_set_count = 0;
	uint32_t base_configuration_set_count = 0;
};

template<typename Option>
struct options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown RTC option");
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda) noexcept
	{
	}
};

template<bool Enable>
struct options_parser<enable<Enable>>
	: opts::base_option_parser<Enable, &options::enable, &options::enable_set_count> {};

template<>
struct options_parser<base_configuration_is_currently_present>
	: opts::base_option_parser<0, nullptr, &options::base_configuration_set_count> {};

template<>
struct options_parser<rtc::clock::internal>
	: opts::base_option_parser<clock::internal,
		&options::clock_value, &options::clock_set_count> {};

template<>
struct options_parser<rtc::clock::external_crystal>
	: opts::base_option_parser<clock::external_crystal,
		&options::clock_value, &options::clock_set_count> {};

template<>
struct options_parser<rtc::clock::one_second_prescaler>
	: opts::base_option_parser<true,
		&options::one_second_prescaler, &options::prescaler_set_count> {};

template<typename Interrupt>
struct interrupt_map : mcutl::interrupt::detail::map<Interrupt> {};

template<> struct interrupt_map<interrupt::alarm>
	: mcutl::interrupt::detail::map_base<&options::alarm_set_count, &options::alarm> {};
template<> struct interrupt_map<interrupt::second>
	: mcutl::interrupt::detail::map_base<&options::second_set_count, &options::second> {};
template<> struct interrupt_map<interrupt::overflow>
	: mcutl::interrupt::detail::map_base<&options::overflow_set_count, &options::overflow> {};

template<>
struct options_parser<interrupt::alarm>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::alarm, interrupt_map> {};

template<>
struct options_parser<interrupt::second>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::second, interrupt_map> {};

template<>
struct options_parser<interrupt::overflow>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::overflow, interrupt_map> {};

template<typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct options_parser<mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: mcutl::interrupt::detail::interrupt_parser<Interrupt, interrupt_map, Priority, SubPriority> {};

template<typename Interrupt>
struct options_parser<mcutl::interrupt::disabled<Interrupt>>
	: mcutl::interrupt::detail::interrupt_parser<mcutl::interrupt::disabled<Interrupt>, interrupt_map> {};

template<>
struct options_parser<interrupt::enable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &options::enable_controller_interrupts_set_count> {};

template<>
struct options_parser<interrupt::disable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &options::disable_controller_interrupts_set_count> {};

template<auto PriorityCount>
struct options_parser<mcutl::interrupt::priority_count<PriorityCount>>
	: opts::base_option_parser<PriorityCount, &options::priority_count,
		&options::priority_count_set_count> {};

} //namespace detail

template<typename Interrupt>
using interrupt_type = typename detail::interrupt_type_helper<Interrupt>::type;

} //namespace mcutl::rtc
