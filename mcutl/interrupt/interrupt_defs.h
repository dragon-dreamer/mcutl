#pragma once

#include <stdint.h>
#include <limits>

namespace mcutl::interrupt
{

using priority_t = int32_t; //Lower value means higher priority
using irqn_t = int32_t;
[[maybe_unused]] constexpr auto default_priority = (std::numeric_limits<priority_t>::max)();
[[maybe_unused]] constexpr auto unused_irqn = (std::numeric_limits<int32_t>::max)();

template<typename Type, priority_t Priority, priority_t Subpriority = default_priority>
struct interrupt
{
	using interrupt_type = Type;
	static constexpr auto priority = Priority;
	static constexpr auto subpriority = Subpriority;
};

namespace detail
{

template<irqn_t Irqn>
struct interrupt_base
{
	static constexpr auto irqn = Irqn;
};

template<typename Interrupt>
struct interrupt_traits
{
	static constexpr auto priority = default_priority;
	static constexpr auto subpriority = default_priority;
	using interrupt_type = Interrupt;
};

template<typename Type, priority_t Priority, priority_t Subpriority>
struct interrupt_traits<interrupt<Type, Priority, Subpriority>>
{
	static constexpr auto priority = Priority;
	static constexpr auto subpriority = Subpriority;
	static_assert(subpriority == default_priority || priority != default_priority,
		"Invalid priority/subpriority settings for an interrupt");
	using interrupt_type = Type;
};

} //namespace detail

} //namespace mcutl::interrupt
