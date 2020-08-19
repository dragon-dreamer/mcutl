#pragma once

#include <memory>

#include "mcutl/backup/backup.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using exti_strict_test_fixture_base = mcutl::tests::mcu::strict_test_fixture_base;

class exti_interrupt_test_fixture : virtual public exti_strict_test_fixture_base
{
public:
	void expect_enable_interrupt(uint32_t irqn)
	{
		::testing::InSequence s;
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
			::testing::IsEmpty()));
		EXPECT_CALL(memory(), write(addr(&(NVIC->ISER[irqn / 32])), (1 << (irqn % 32))));
	}
	
	void expect_disable_interrupt(uint32_t irqn)
	{
		::testing::InSequence s;
		EXPECT_CALL(memory(), write(addr(&(NVIC->ICER[irqn / 32])), (1 << (irqn % 32))));
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dsb>(),
			::testing::IsEmpty()));
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
			::testing::IsEmpty()));
	}
	
	void expect_enable_interrupt(uint32_t irqn, uint32_t priority, uint32_t subpriority)
	{
		::testing::InSequence s;
		expect_set_interrupt_priority(irqn, priority, subpriority);
		expect_enable_interrupt(irqn);
	}
	
	void expect_set_interrupt_priority(uint32_t irqn, uint32_t priority, uint32_t subpriority)
	{
		::testing::InSequence s;
		if (priority != mcutl::interrupt::default_priority)
		{
			priority <<= 4;
			if (subpriority != mcutl::interrupt::default_priority)
				priority |= subpriority;
			
			EXPECT_CALL(memory(), write(addr(&(NVIC->IP[irqn])), priority));
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
				::testing::IsEmpty()));
		}
	}
	
	void expect_clear_pending_interrupt(uint32_t irqn)
	{
		EXPECT_CALL(memory(), write(addr(&(NVIC->ICPR[irqn / 32])), (1 << (irqn % 32))));
	}
};
