#pragma once

#include <stdint.h>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/class_limitations.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::systick
{

template<bool Enable>
struct enable : types::static_class {};

namespace clock_source
{

struct processor {};
struct external {};

} //namespace clock_source

namespace interrupt
{

struct tick {};

} //namespace interrupt

namespace detail
{

struct clock_source
{
	enum value { processor, external, end };
};

struct options
{
	bool enable = false;
	clock_source::value clock_source_type = clock_source::processor;
	mcutl::interrupt::detail::interrupt_info tick {};
	uint64_t priority_count = 0;
	
	uint32_t enable_set_count = 0;
	uint32_t tick_set_count = 0;
	uint32_t clock_source_set_count = 0;
	uint32_t priority_count_set_count = 0;
};

template<typename Option>
struct options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown SYSTICK option");
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
struct options_parser<systick::clock_source::external>
	: opts::base_option_parser<clock_source::external,
	&options::clock_source_type, &options::clock_source_set_count> {};

template<>
struct options_parser<systick::clock_source::processor>
	: opts::base_option_parser<clock_source::processor,
	&options::clock_source_type, &options::clock_source_set_count> {};

template<typename Interrupt>
struct interrupt_map : mcutl::interrupt::detail::map<Interrupt> {};

template<> struct interrupt_map<interrupt::tick>
	: mcutl::interrupt::detail::map_base<&options::tick_set_count, &options::tick> {};

template<>
struct options_parser<interrupt::tick>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::tick, interrupt_map> {};

template<typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct options_parser<mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: mcutl::interrupt::detail::interrupt_parser<Interrupt, interrupt_map, Priority, SubPriority> {};

template<typename Interrupt>
struct options_parser<mcutl::interrupt::disabled<Interrupt>>
	: mcutl::interrupt::detail::interrupt_parser<mcutl::interrupt::disabled<Interrupt>, interrupt_map> {};

template<auto PriorityCount>
struct options_parser<mcutl::interrupt::priority_count<PriorityCount>>
	: opts::base_option_parser<PriorityCount, &options::priority_count,
		&options::priority_count_set_count> {};

template<typename Interrupt>
struct interrupt_type_helper
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown SYSTICK interrupt type");
};

} //namespace detail

enum class direction_type
{
	top_down,
	bottom_up
};

template<typename Interrupt>
using interrupt_type = typename detail::interrupt_type_helper<Interrupt>::type;

} //namespace mcutl::systick
