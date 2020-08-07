#pragma once

#include <array>
#include <tuple>
#include <type_traits>

#include "mcutl/exti/exti_defs.h"
#include "mcutl/device/exti/device_exti.h"

#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::exti
{

template<uint32_t Index>
inline constexpr bool has_line() noexcept
{
	return Index < std::tuple_size_v<decltype(device::exti::present_exti_lines)>
		&& device::exti::present_exti_lines[Index];
}

namespace detail
{

template<typename Line, typename Arr>
constexpr bool validate_line(Arr& processed_lines) noexcept
{
	static_assert(Line::line::value < std::tuple_size_v<Arr>, "Invalid EXTI line number");
	if constexpr (Line::line::value < std::tuple_size_v<Arr>)
	{
		if (processed_lines[Line::line::value])
			return false;
		
		if (!device::exti::present_exti_lines[Line::line::value])
			return false;
		
		processed_lines[Line::line::value] = true;
	}
	
	return true;
}

template<typename... Lines>
constexpr bool validate_lines() noexcept
{
	[[maybe_unused]] std::array<bool,
		std::tuple_size_v<decltype(device::exti::present_exti_lines)>> processed_lines{{}};
	return (... && validate_line<Lines>(processed_lines));
}

template<typename... LineOptions>
struct configuration_helper
{
	static_assert(validate_lines<LineOptions...>(), 
		"Duplicate or non-present EXTI line definitions are present");
	
	static void enable_lines() MCUTL_NOEXCEPT
	{
		device::exti::enable_lines<LineOptions...>();
	}

	static void disable_lines() MCUTL_NOEXCEPT
	{
		device::exti::disable_lines<LineOptions...>();
	}

	static void clear_pending_line_bits() MCUTL_NOEXCEPT
	{
		device::exti::clear_pending_line_bits<LineOptions...>();
	}

	static void clear_pending_line_bits_atomic() MCUTL_NOEXCEPT
	{
		device::exti::clear_pending_line_bits_atomic<LineOptions...>();
	}

	static auto get_pending_line_bits() MCUTL_NOEXCEPT
	{
		return device::exti::get_pending_line_bits<LineOptions...>();
	}

	static void clear_pending_interrupts() MCUTL_NOEXCEPT
	{
		device::exti::clear_pending_interrupts<LineOptions...>();
	}
	
	static void disable_interrupts() MCUTL_NOEXCEPT
	{
		device::exti::disable_interrupts<LineOptions...>();
	}

	template<auto PriorityCount>
	static void enable_interrupts() MCUTL_NOEXCEPT
	{
		device::exti::enable_interrupts<PriorityCount, LineOptions...>();
	}

	template<auto PriorityCount>
	static void set_interrupt_prioritites() MCUTL_NOEXCEPT
	{
		device::exti::set_interrupt_prioritites<PriorityCount, LineOptions...>();
	}

	static void software_trigger() MCUTL_NOEXCEPT
	{
		device::exti::software_trigger<LineOptions...>();
	}

	static void software_trigger_atomic() MCUTL_NOEXCEPT
	{
		device::exti::software_trigger_atomic<LineOptions...>();
	}

	static constexpr auto get_lines_bit_mask() noexcept
	{
		return device::exti::get_lines_bit_mask<LineOptions...>();
	}
};

template<typename... LineOptions>
struct configuration_helper<config<LineOptions...>> : configuration_helper<LineOptions...> {};

} //namespace detail

template<typename... Lines>
inline void enable_lines() MCUTL_NOEXCEPT
{
	detail::configuration_helper<Lines...>::enable_lines();
}

template<typename... Lines>
inline void disable_lines() MCUTL_NOEXCEPT
{
	detail::configuration_helper<Lines...>::disable_lines();
}

template<typename... Lines>
inline void clear_pending_line_bits() MCUTL_NOEXCEPT
{
	detail::configuration_helper<Lines...>::clear_pending_line_bits();
}

template<typename... Lines>
inline void clear_pending_line_bits_atomic() MCUTL_NOEXCEPT
{
	detail::configuration_helper<Lines...>::clear_pending_line_bits_atomic();
}

template<typename... Lines>
[[nodiscard]] inline auto get_pending_line_bits() MCUTL_NOEXCEPT
{
	return detail::configuration_helper<Lines...>::get_pending_line_bits();
}

template<typename... LineInterruptOptions>
inline void clear_pending_interrupts() MCUTL_NOEXCEPT
{
	detail::configuration_helper<LineInterruptOptions...>::clear_pending_interrupts();
}

template<typename... LineInterruptOptions>
inline void disable_interrupts() MCUTL_NOEXCEPT
{
	detail::configuration_helper<LineInterruptOptions...>::disable_interrupts();
}

template<typename... LineInterruptOptions>
inline void enable_interrupts() MCUTL_NOEXCEPT
{
	detail::configuration_helper<LineInterruptOptions...>
		::template enable_interrupts<interrupt::maximum_priorities>();
}

template<typename... LineInterruptOptions>
inline void set_interrupt_prioritites() MCUTL_NOEXCEPT
{
	detail::configuration_helper<LineInterruptOptions...>
		::template set_interrupt_prioritites<interrupt::maximum_priorities>();
}

template<auto PriorityCount, typename... LineInterruptOptions>
inline void enable_interrupts() MCUTL_NOEXCEPT
{
	detail::configuration_helper<LineInterruptOptions...>
		::template enable_interrupts<PriorityCount>();
}

template<auto PriorityCount, typename... LineInterruptOptions>
inline void set_interrupt_prioritites() MCUTL_NOEXCEPT
{
	detail::configuration_helper<LineInterruptOptions...>
		::template set_interrupt_prioritites<PriorityCount>();
}

template<typename... Lines>
inline void software_trigger() MCUTL_NOEXCEPT
{
	detail::configuration_helper<Lines...>::software_trigger();
}

template<typename... Lines>
inline void software_trigger_atomic() MCUTL_NOEXCEPT
{
	detail::configuration_helper<Lines...>::software_trigger_atomic();
}

template<typename... Lines>
[[maybe_unused]] constexpr auto lines_bit_mask_v = detail::configuration_helper<Lines...>::get_lines_bit_mask();

[[maybe_unused]] constexpr bool has_events = device::exti::has_events;
[[maybe_unused]] constexpr bool has_rising_trigger = device::exti::has_rising_trigger;
[[maybe_unused]] constexpr bool has_falling_trigger = device::exti::has_falling_trigger;
[[maybe_unused]] constexpr bool has_rising_and_falling_trigger = device::exti::has_rising_and_falling_trigger;
[[maybe_unused]] constexpr bool has_software_trigger = device::exti::has_software_trigger;
[[maybe_unused]] constexpr bool has_pending_line_bits = device::exti::has_pending_line_bits;
[[maybe_unused]] constexpr bool has_atomic_clear_pending_line_bits
	= device::exti::has_atomic_clear_pending_line_bits;
[[maybe_unused]] constexpr bool has_atomic_software_trigger
	= device::exti::has_atomic_software_trigger;

} //namespace mcutl::exti

namespace mcutl::gpio
{

namespace detail
{

template<typename PinConfig>
struct line_bits_helper
{
	static constexpr auto get_line_bits() noexcept
	{
		return 0;
	}
};

template<char PortLetter, uint32_t PinNumber>
struct line_bits_helper<pin<PortLetter, PinNumber>>
{
	static constexpr auto get_line_bits() noexcept
	{
		constexpr auto line_number = detail::exti_line_options<
			pin<PortLetter, PinNumber>, default_exti_line>::line;
		return exti::lines_bit_mask_v<exti::line<line_number>>;
	}
};

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct line_bits_helper<connect_to_exti_line<Pin, ExtiLine, Options...>>
{
	static constexpr auto get_line_bits() noexcept
	{
		constexpr auto line_number = detail::exti_line_options<Pin, ExtiLine>::line;
		return exti::lines_bit_mask_v<exti::line<line_number>>;
	}
};

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct line_bits_helper<disconnect_from_exti_line<Pin, ExtiLine, Options...>>
{
	static constexpr auto get_line_bits() noexcept
	{
		constexpr auto line_number = detail::exti_line_options<Pin, ExtiLine>::line;
		return exti::lines_bit_mask_v<exti::line<line_number>>;
	}
};

template<typename... PinConfig>
struct exti_helper
{
	static constexpr auto get_exti_lines_bit_mask() noexcept
	{
		return (0u | ... | line_bits_helper<PinConfig>::get_line_bits());
	}
};

template<typename... PinConfig>
struct exti_helper<config<PinConfig...>> : exti_helper<PinConfig...> {};

} //namespace detail

template<typename Pin>
using default_exti_line_type = exti::line<detail::exti_line_options<Pin, default_exti_line>::line>;

template<typename... PinConfig>
[[maybe_unused]] constexpr auto exti_lines_bit_mask_v
	= detail::exti_helper<PinConfig...>::get_exti_lines_bit_mask();

} //namespace mcutl::gpio
