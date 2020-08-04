#pragma once

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/device/interrupt/device_interrupt.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::interrupt
{

[[maybe_unused]] constexpr bool has_priorities = device::interrupt::has_priorities;
[[maybe_unused]] constexpr bool has_subpriorities = device::interrupt::has_subpriorities;
[[maybe_unused]] constexpr bool has_interrupt_context = device::interrupt::has_interrupt_context;
[[maybe_unused]] constexpr bool has_get_active = device::interrupt::has_get_active;

template<auto PriorityCount = device::interrupt::maximum_priorities>
inline void initialize_controller() MCUTL_NOEXCEPT
{
	device::interrupt::initialize_controller<PriorityCount>();
}

template<typename Interrupt, auto PriorityCount = device::interrupt::maximum_priorities>
inline void enable() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	if constexpr (traits::priority != default_priority)
	{
		device::interrupt::set_priority<
			typename traits::interrupt_t, traits::priority,
			traits::subpriority, PriorityCount>();
	}
	
	device::interrupt::enable<typename traits::interrupt_t>();
}

template<typename Interrupt>
inline void disable() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	device::interrupt::disable<typename traits::interrupt_t>();
}

template<typename Interrupt>
[[nodiscard]] inline bool is_enabled() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	return device::interrupt::is_enabled<typename traits::interrupt_t>();
}

template<typename Interrupt, auto PriorityCount = device::interrupt::maximum_priorities>
inline void set_priority() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	static_assert(traits::priority != default_priority,
		"Invalid priority for set_interrupt_priority()");
	device::interrupt::set_priority<
		typename traits::interrupt_t, traits::priority,
		traits::subpriority, PriorityCount>();
}

template<typename Interrupt, auto PriorityCount = device::interrupt::maximum_priorities>
[[nodiscard]] inline priority_t get_priority() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	return device::interrupt::get_priority<typename traits::interrupt_t, PriorityCount>();
}

template<typename Interrupt, auto PriorityCount = device::interrupt::maximum_priorities>
[[nodiscard]] inline priority_t get_subpriority() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	return device::interrupt::get_subpriority<typename traits::interrupt_t, PriorityCount>();
}

template<typename Interrupt>
inline void clear_pending() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	device::interrupt::clear_pending<typename traits::interrupt_t>();
}

template<typename Interrupt>
[[nodiscard]] inline bool is_pending() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	return device::interrupt::is_pending<typename traits::interrupt_t>();
}

template<typename T = void>
[[nodiscard]] inline bool is_interrupt_context() MCUTL_NOEXCEPT
{
	return device::interrupt::is_interrupt_context<T>();
}

inline void disable_all() MCUTL_NOEXCEPT
{
	device::interrupt::disable_all();
}

inline void enable_all() MCUTL_NOEXCEPT
{
	device::interrupt::enable_all();
}

template<typename T = void>
[[nodiscard]] inline irqn_t get_active() MCUTL_NOEXCEPT
{
	return device::interrupt::get_active<T>();
}

[[maybe_unused]] constexpr bool has_atomic_enable = device::interrupt::has_atomic_enable;
[[maybe_unused]] constexpr bool has_atomic_disable = device::interrupt::has_atomic_disable;
[[maybe_unused]] constexpr bool has_atomic_set_priority = device::interrupt::has_atomic_set_priority;
[[maybe_unused]] constexpr bool has_atomic_clear_pending = device::interrupt::has_atomic_clear_pending;

template<typename Interrupt>
inline void enable_atomic() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	static_assert(traits::priority == default_priority,
		"Unable to set interrupt priority with enable_atomic() call");
	device::interrupt::enable_atomic<typename traits::interrupt_t>();
}

template<typename Interrupt>
inline void disable_atomic() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	device::interrupt::disable_atomic<typename traits::interrupt_t>();
}

template<typename Interrupt, auto PriorityCount = device::interrupt::maximum_priorities>
inline void set_priority_atomic() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	static_assert(traits::priority != default_priority,
		"Invalid priority for set_interrupt_priority()");
	device::interrupt::set_priority_atomic<
		typename traits::interrupt_t, traits::priority,
		traits::subpriority, PriorityCount>();
}

template<typename Interrupt>
inline void clear_pending_atomic() MCUTL_NOEXCEPT
{
	using traits = detail::interrupt_traits<Interrupt>;
	device::interrupt::clear_pending_atomic<typename traits::interrupt_t>();
}

} //namespace mcutl::interrupt
