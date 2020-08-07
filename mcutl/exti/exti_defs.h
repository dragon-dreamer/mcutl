#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>

#include "mcutl/gpio/gpio_defs.h"
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

namespace mcutl::gpio
{

constexpr uint32_t default_exti_line = (std::numeric_limits<uint32_t>::max)();

namespace detail
{

template<uint32_t... ExtiLines>
struct exti_map_helper
{
public:
	static constexpr uint32_t get_default_line() noexcept
	{
		static_assert(sizeof...(ExtiLines) != 0,
			"Selected GPIO does not map to any EXTI lines");
		static_assert(sizeof...(ExtiLines) == 1,
			"Selected GPIO map to more than one EXTI lines");
		return get_first_line<ExtiLines...>();
	}
	
	template<uint32_t ExtiLine>
	static constexpr bool maps_to_line() noexcept
	{
		static_assert(sizeof...(ExtiLines) != 0,
			"Selected GPIO does not map to any EXTI lines");
		return (... || (ExtiLine == ExtiLines));
	}
	
private:
	template<uint32_t ExtiLine, uint32_t...>
	static constexpr uint32_t get_first_line() noexcept
	{
		return ExtiLine;
	}
};

template<typename Pin>
struct exti_map
{
	static_assert(types::always_false<Pin>::value,
		"Pin does not support EXTI mapping");
};

template<typename Mapping, uint32_t ExtiLine>
constexpr auto get_exti_line() noexcept
{
	if constexpr (ExtiLine == default_exti_line)
		return Mapping::get_default_line();
	else
		return ExtiLine;
}

struct exti_tag {};

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct exti_line_options : config_base<Pin, exti_tag, Options...>
{
	using mapping = exti_map<Pin>;
	
	static constexpr auto line = get_exti_line<mapping, ExtiLine>();
	
	static_assert(mapping::template maps_to_line<line>(),
		"Selected GPIO does not map to selected EXTI line");
};

} //namespace detail

template<typename Pin, uint32_t ExtiLine = default_exti_line, typename... Options>
struct connect_to_exti_line : detail::exti_line_options<Pin, ExtiLine, Options...> {};

template<typename Pin, uint32_t ExtiLine = default_exti_line, typename... Options>
struct disconnect_from_exti_line : detail::exti_line_options<Pin, ExtiLine, Options...> {};

namespace detail
{

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct option_category<connect_to_exti_line<Pin, ExtiLine, Options...>> : types::identity<exti_option> {};
template<typename Pin, uint32_t ExtiLine, typename... Options>
struct option_category<disconnect_from_exti_line<Pin, ExtiLine, Options...>> : types::identity<exti_option> {};

} //namespace detail

} //namespace mcutl::gpio
