#pragma once

#include <stdint.h>
#include <type_traits>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::exti
{

namespace line_mode
{

struct event {};
struct interrupt {};
struct event_and_interrupt {};

} //namespace line_mode

namespace line_trigger
{

struct software_only {};
struct rising {};
struct falling {};
struct rising_and_falling {};

} //namespace line_trigger

namespace detail
{

template<typename Line>
struct line_to_interrupt
{
	static_assert(types::always_false<Line>::value, "Unknown external line");
};

template<typename Line>
using line_to_interrupt_t = typename line_to_interrupt<Line>::type;

} //namespace detail

template<uint32_t Index>
struct line final : std::integral_constant<uint32_t, Index> {};

template<typename Line, typename LineMode, typename LineTrigger,
	typename... Options>
struct line_options
{
	using line = Line;
	using mode = LineMode;
	using trigger = LineTrigger;
	using options = types::list<Options...>;
};

template<typename Line,
	interrupt::priority_t Priority = interrupt::default_priority,
	interrupt::priority_t Subpriority = interrupt::default_priority,
	typename... Options>
struct line_interrupt_options
{
	using interrupt_type = interrupt::interrupt<
		detail::line_to_interrupt_t<Line>, Priority, Subpriority>;
	using line = Line;
	using options = types::list<Options...>;
};

template<typename... LineOptions>
struct config {};

} //namespace mcutl::exti
