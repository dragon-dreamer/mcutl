#pragma once

#include <stdint.h>

#include "mcutl/device/device.h"
#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/instruction/instruction.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/math.h"

namespace mcutl::device::interrupt
{

[[maybe_unused]] constexpr uint32_t maximum_priorities = 16;
[[maybe_unused]] constexpr bool has_atomic_enable = true;
[[maybe_unused]] constexpr bool has_atomic_disable = true;
[[maybe_unused]] constexpr bool has_atomic_set_priority = true;
[[maybe_unused]] constexpr bool has_atomic_clear_pending = true;
[[maybe_unused]] constexpr bool has_priorities = true;
[[maybe_unused]] constexpr bool has_subpriorities = true;
[[maybe_unused]] constexpr bool has_interrupt_context = true;
[[maybe_unused]] constexpr bool has_get_active = true;

template<uint32_t PriorityCount>
inline constexpr void validate_priority_count() noexcept
{
	static_assert(PriorityCount && math::is_power_of_2<PriorityCount>()
		&& PriorityCount <= (1 << __NVIC_PRIO_BITS),
		"PriorityCount must be power of 2 and have __NVIC_PRIO_BITS bits maximum");
}

template<uint32_t PriorityCount>
inline constexpr uint32_t get_priority_bitcount() noexcept
{
	return math::log2<PriorityCount>();
}

template<mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t SubPriority, uint32_t PriorityCount>
constexpr uint8_t get_priority_value() noexcept
{
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	uint8_t res = (Priority << (8u - priority_bitcount)) & 0xFFu;
	if constexpr (SubPriority != mcutl::interrupt::default_priority)
		res |= SubPriority;
	
	return res;
}

template<uint32_t PriorityCount>
void initialize_controller() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_grouping = 7 - get_priority_bitcount<PriorityCount>();
	mcutl::memory::set_register_bits<SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_PRIGROUP_Msk,
		(0x5FAu << SCB_AIRCR_VECTKEY_Pos) | (priority_grouping << SCB_AIRCR_PRIGROUP_Pos),
		&SCB_Type::AIRCR, SCB_BASE>();
}

template<typename Interrupt>
void enable() noexcept
{
	mcutl::instruction::execute<instruction::type::dmb>();
	__COMPILER_BARRIER();
	mcutl::memory::set_register_array_value<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ISER, (Interrupt::irqn >> 5u), NVIC_BASE>();
	__COMPILER_BARRIER();
}

template<typename Interrupt, mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t SubPriority, uint32_t PriorityCount>
void set_priority() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	static_assert(Priority >= 0 && Priority < (1 << priority_bitcount), "Invalid interrupt priority");
	
	constexpr uint8_t priority_value = get_priority_value<Priority, SubPriority, PriorityCount>();
	mcutl::memory::set_register_array_value<priority_value,
		&NVIC_Type::IP, Interrupt::irqn, NVIC_BASE>();
	mcutl::instruction::execute<instruction::type::isb>();
}

template<typename Interrupt, uint32_t PriorityCount>
mcutl::interrupt::priority_t get_priority() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	if constexpr (!priority_bitcount)
	{
		return {};
	}
	else
	{
		return static_cast<mcutl::interrupt::priority_t>(
			mcutl::memory::get_register_array_bits<0xffu, &NVIC_Type::IP, Interrupt::irqn, NVIC_BASE>()
				>> (8u - priority_bitcount));
	}
}

template<typename Interrupt, uint32_t PriorityCount>
mcutl::interrupt::priority_t get_subpriority() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	return static_cast<mcutl::interrupt::priority_t>(
		mcutl::memory::get_register_array_bits<0xffu, &NVIC_Type::IP, Interrupt::irqn, NVIC_BASE>()
			& (0xffu >> priority_bitcount));
}

template<typename Interrupt>
void disable() noexcept
{
	mcutl::memory::set_register_array_value<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ICER, (Interrupt::irqn >> 5u), NVIC_BASE>();
	mcutl::instruction::execute<instruction::type::dsb>();
	mcutl::instruction::execute<instruction::type::isb>();
}

template<typename Interrupt>
void clear_pending() noexcept
{
	mcutl::memory::set_register_array_value<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ICPR, (Interrupt::irqn >> 5u), NVIC_BASE>();
}

template<typename Interrupt>
bool is_pending() noexcept
{
	return mcutl::memory::get_register_array_flag<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ISPR, (Interrupt::irqn >> 5u), NVIC_BASE>();
}

template<typename Interrupt>
bool is_enabled() noexcept
{
	return mcutl::memory::get_register_array_flag<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ISER, (Interrupt::irqn >> 5u), NVIC_BASE>();
}

template<typename = void>
bool is_interrupt_context() noexcept
{
	return mcutl::memory::get_register_flag<SCB_ICSR_VECTACTIVE_Msk, &SCB_Type::ICSR, SCB_BASE>();
}

template<typename T = void>
mcutl::interrupt::irqn_t get_active() noexcept
{
	auto active = mcutl::memory::get_register_bits<SCB_ICSR_VECTACTIVE_Msk, &SCB_Type::ICSR, SCB_BASE>();
	return active ? active : mcutl::interrupt::unused_irqn;
}

inline void disable_all() noexcept
{
	mcutl::instruction::execute<instruction::type::cpsid_i>();
}

inline void enable_all() noexcept
{
	mcutl::instruction::execute<instruction::type::cpsie_i>();
}

template<typename Interrupt>
inline void enable_atomic() noexcept
{
	enable<Interrupt>();
}

template<typename Interrupt>
inline void disable_atomic() noexcept
{
	disable<Interrupt>();
}

template<typename Interrupt, mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t SubPriority, uint32_t PriorityCount>
inline void set_priority_atomic() noexcept
{
	set_priority<Interrupt, Priority, SubPriority, PriorityCount>();
}

template<typename Interrupt>
inline void clear_pending_atomic() noexcept
{
	clear_pending<Interrupt>();
}

} //namespace mcutl::device::interrupt
