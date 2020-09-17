#pragma once

#include <stdint.h>
#include <limits>
#include <type_traits>

#include "mcutl/utils/options_parser.h"

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

template<typename Interrupt>
struct disabled {};

template<auto PriorityCount>
struct priority_count {};

namespace detail
{

template<typename Type>
struct interrupt_type
{
	using type = Type;
};

template<typename Type, priority_t Priority, priority_t Subpriority>
struct interrupt_type<interrupt<Type, Priority, Subpriority>>
{
	using type = Type;
};

template<typename Type>
struct interrupt_type<disabled<Type>>
{
	using type = Type;
};

template<typename Type>
using interrupt_type_t = typename interrupt_type<Type>::type;

struct interrupt_info
{
	priority_t priority = default_priority;
	priority_t subpriority = default_priority;
	bool disable = false;
};

template<typename Interrupt>
struct map {};

template<auto InterruptSetCount, auto InterruptInfo>
struct map_base
{
	static constexpr auto interrupt_set_count = InterruptSetCount;
	static constexpr auto interrupt_info = InterruptInfo;
};

template<typename Interrupt, template <typename> typename InterruptMap,
	priority_t Priority = default_priority, priority_t SubPriority = default_priority>
struct interrupt_parser
	: opts::base_option_parser<0, nullptr, InterruptMap<Interrupt>::interrupt_set_count>
{
	template<typename Options>
	static constexpr void parse(Options& options) noexcept
	{
		options.*(InterruptMap<Interrupt>::interrupt_info)
			= { Priority, SubPriority, false };
		++(options.*(InterruptMap<Interrupt>::interrupt_set_count));
	}
};

template<typename Interrupt, template <typename> typename InterruptMap,
	priority_t Priority, priority_t SubPriority>
struct interrupt_parser<mcutl::interrupt::disabled<Interrupt>,
	InterruptMap, Priority, SubPriority>
	: opts::base_option_parser<0, nullptr, InterruptMap<Interrupt>::interrupt_set_count>
{
	template<typename Options>
	static constexpr void parse(Options& options) noexcept
	{
		(options.*(InterruptMap<Interrupt>::interrupt_info)).disable = true;
		++(options.*(InterruptMap<Interrupt>::interrupt_set_count));
	}
};

template<template <typename> typename InterruptMap,
	typename ControllerInterruptTypeMap,
	typename... Interrupts>
struct interrupt_priority_conflict_checker
{
public:
	template<typename Options>
	static constexpr bool check(Options& options) noexcept
	{
		return check_interrupt<Interrupts...>(options);
	}
	
private:
	template<typename Options>
	static constexpr bool check_interrupt(Options&) noexcept
	{
		return true;
	}
		
	template<typename Interrupt, typename... OtherInterrupts, typename Options>
	static constexpr bool check_interrupt(Options& options) noexcept
	{
		const auto& info = options.*(InterruptMap<Interrupt>::interrupt_info);
		if (options.*(InterruptMap<Interrupt>::interrupt_set_count) != 0 && !info.disable)
		{
			if (!(... && check_interrupt<typename ControllerInterruptTypeMap::template type<Interrupt>,
				OtherInterrupts>(options, info)))
			{
				return false;
			}
		}
		
		return check_interrupt<OtherInterrupts...>(options);
	}
	
	template<typename ControllerInterrupt,
		typename Interrupt, typename Options, typename InterruptInfo>
	static constexpr bool check_interrupt(Options& options, const InterruptInfo& info) noexcept
	{
		if constexpr (std::is_same_v<ControllerInterrupt,
			typename ControllerInterruptTypeMap::template type<Interrupt>>)
		{
			const auto& other_info = options.*(InterruptMap<Interrupt>::interrupt_info);
			if (options.*(InterruptMap<Interrupt>::interrupt_set_count) != 0 && !other_info.disable)
			{
				return other_info.priority == info.priority
					&& other_info.subpriority == info.subpriority;
			}
		}
		
		return true;
	}
};

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
