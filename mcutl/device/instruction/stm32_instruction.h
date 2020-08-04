#pragma once

#include "mcutl/device/device.h"
#include "mcutl/device/instruction/instruction_types.h"
#include "mcutl/instruction/instruction_defs.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::instruction::detail
{

template<>
struct executor<instruction_type<device::instruction::type::dmb>>
{
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		__DMB();
	}
};

template<>
struct executor<instruction_type<device::instruction::type::dsb>>
{
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		__DSB();
	}
};

template<>
struct executor<instruction_type<device::instruction::type::isb>>
{
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		__ISB();
	}
};

template<>
struct executor<instruction_type<device::instruction::type::cpsid_i>>
{
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		__disable_irq();
	}
};

template<>
struct executor<instruction_type<device::instruction::type::cpsie_i>>
{
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		__enable_irq();
	}
};

template<>
struct executor<instruction_type<device::instruction::type::wfe>>
{
	MCUTL_STATIC_NAKED_NOINLINE void wfe_call() noexcept
	{
		__asm volatile(
			"wfe\n"
			"nop\n"
			"bx lr\n"
		::: "memory");
	}
	
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		wfe_call();
	}
};

template<>
struct executor<instruction_type<device::instruction::type::wfi>>
{
	MCUTL_STATIC_NAKED_NOINLINE void wfi_call() noexcept
	{
		__asm volatile(
			"wfi\n"
			"nop\n"
			"bx lr\n"
		::: "memory");
	}
	
	MCUTL_STATIC_FORCEINLINE void run() noexcept
	{
		wfi_call();
	}
};

} //namespace mcutl::instruction::detail
