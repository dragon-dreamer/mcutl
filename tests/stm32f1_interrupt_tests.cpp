#define STM32F103xE
#define STM32F1

#include "mcutl/interrupt/interrupt.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using interrupt_strict_test_fixture = mcutl::tests::mcu::strict_test_fixture_base;

class atomic_interrupt_strict_test_fixture :
	public interrupt_strict_test_fixture,
	public testing::WithParamInterface<bool>
{
};

struct test_interrupt : mcutl::interrupt::detail::interrupt_base<55>
{
};

TEST_F(interrupt_strict_test_fixture, DefaultInitializeControllerTest)
{
	memory().allow_reads(addr(&SCB->AIRCR));
	memory().set(addr(&SCB->AIRCR), 0xfa055678u);
	EXPECT_CALL(memory(), write(addr(&SCB->AIRCR), 0x05fa5378u));
	mcutl::interrupt::initialize_controller();
}

TEST_F(interrupt_strict_test_fixture, InitializeControllerSinglePriorityTest)
{
	memory().allow_reads(addr(&SCB->AIRCR));
	memory().set(addr(&SCB->AIRCR), 0xfa055678u);
	EXPECT_CALL(memory(), write(addr(&SCB->AIRCR), 0x05fa5778u));
	mcutl::interrupt::initialize_controller<1>();
}

TEST_P(atomic_interrupt_strict_test_fixture, EnableNoPriorityTest)
{
	::testing::InSequence s;
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	EXPECT_CALL(memory(), write(addr(&(NVIC->ISER[1])), (1u << 23)));
	if (GetParam())
		mcutl::interrupt::enable_atomic<test_interrupt>();
	else
		mcutl::interrupt::enable<test_interrupt>();
}

TEST_P(atomic_interrupt_strict_test_fixture, DisableTest)
{
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&(NVIC->ICER[1])), (1u << 23)));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dsb>(),
			::testing::IsEmpty()));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
			::testing::IsEmpty()));
	if (GetParam())
		mcutl::interrupt::disable_atomic<test_interrupt>();
	else
		mcutl::interrupt::disable<test_interrupt>();
}

TEST_F(interrupt_strict_test_fixture, EnableWithPriorityTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), write(addr(&(NVIC->IP[test_interrupt::irqn])), 5u << 4));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
		::testing::IsEmpty()));
	
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	EXPECT_CALL(memory(), write(addr(&(NVIC->ISER[1])), (1u << 23)));
	
	mcutl::interrupt::enable<mcutl::interrupt::interrupt<test_interrupt, 5>>();
}

TEST_F(interrupt_strict_test_fixture, EnableWithPriorityAndSubPriorityTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), write(addr(&(NVIC->IP[test_interrupt::irqn])), (5u << 5) | (3u)));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
			::testing::IsEmpty()));
	
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
			::testing::IsEmpty()));
	EXPECT_CALL(memory(), write(addr(&(NVIC->ISER[1])), (1u << 23)));
	
	mcutl::interrupt::enable<mcutl::interrupt::interrupt<test_interrupt, 5, 3>, 8>();
}

TEST_P(atomic_interrupt_strict_test_fixture, SetPriorityTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), write(addr(&(NVIC->IP[test_interrupt::irqn])), 5u << 4));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
			::testing::IsEmpty()));
	
	if (GetParam())
		mcutl::interrupt::set_priority_atomic<mcutl::interrupt::interrupt<test_interrupt, 5>>();
	else
		mcutl::interrupt::set_priority<mcutl::interrupt::interrupt<test_interrupt, 5>>();
}

TEST_P(atomic_interrupt_strict_test_fixture, SetPriorityAndSubPriorityTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), write(addr(&(NVIC->IP[test_interrupt::irqn])), (5u << 5) | (3u)));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
			::testing::IsEmpty()));
	
	if (GetParam())
		mcutl::interrupt::set_priority_atomic<mcutl::interrupt::interrupt<test_interrupt, 5, 3>, 8>();
	else
		mcutl::interrupt::set_priority<mcutl::interrupt::interrupt<test_interrupt, 5, 3>, 8>();
}

TEST_F(interrupt_strict_test_fixture, GetPriorityAndSubPriorityTest)
{
	memory().set(addr(&(NVIC->IP[test_interrupt::irqn])), (6u << 5) | (4u));
	memory().allow_reads(addr(&(NVIC->IP[test_interrupt::irqn])));
	
	EXPECT_EQ((mcutl::interrupt::get_priority<test_interrupt, 8>()), 6);
	EXPECT_EQ((mcutl::interrupt::get_subpriority<test_interrupt, 8>()), 4);
}

TEST_F(interrupt_strict_test_fixture, IsEnabledTest)
{
	memory().allow_reads(addr(&(NVIC->ISER[1])));
	
	memory().set(addr(&(NVIC->ISER[1])), (1u << 23));
	EXPECT_TRUE((mcutl::interrupt::is_enabled<test_interrupt>()));
	
	memory().set(addr(&(NVIC->ISER[1])), (1u << 22));
	EXPECT_FALSE((mcutl::interrupt::is_enabled<test_interrupt>()));
}

TEST_F(interrupt_strict_test_fixture, IsPendingTest)
{
	memory().allow_reads(addr(&(NVIC->ISPR[1])));
	
	memory().set(addr(&(NVIC->ISPR[1])), (1u << 23));
	EXPECT_TRUE((mcutl::interrupt::is_pending<test_interrupt>()));
	
	memory().set(addr(&(NVIC->ISPR[1])), (1u << 22));
	EXPECT_FALSE((mcutl::interrupt::is_pending<test_interrupt>()));
}

TEST_P(atomic_interrupt_strict_test_fixture, ClearPendingTest)
{
	EXPECT_CALL(memory(), write(addr(&(NVIC->ICPR[1])), (1u << 23)));
	
	if (GetParam())
		mcutl::interrupt::clear_pending_atomic<test_interrupt>();
	else
		mcutl::interrupt::clear_pending<test_interrupt>();
}

TEST_F(interrupt_strict_test_fixture, IsInterruptContextTest)
{
	memory().allow_reads(addr(&(SCB->ICSR)));
	
	memory().set(addr(&(SCB->ICSR)), 1u);
	EXPECT_TRUE(mcutl::interrupt::is_interrupt_context());
	
	memory().set(addr(&(SCB->ICSR)), 0x8000u);
	EXPECT_FALSE(mcutl::interrupt::is_interrupt_context());
}

TEST_F(interrupt_strict_test_fixture, GetActiveTest)
{
	memory().allow_reads(addr(&(SCB->ICSR)));
	
	memory().set(addr(&(SCB->ICSR)), 15u);
	EXPECT_EQ(mcutl::interrupt::get_active(), 15);
	
	memory().set(addr(&(SCB->ICSR)), 0xfffffe00u);
	EXPECT_EQ(mcutl::interrupt::get_active(), mcutl::interrupt::unused_irqn);
}

TEST_F(interrupt_strict_test_fixture, EnableAllTest)
{
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::cpsie_i>(),
		::testing::IsEmpty()));
	mcutl::interrupt::enable_all();
}

TEST_F(interrupt_strict_test_fixture, DisableAllTest)
{
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::cpsid_i>(),
			::testing::IsEmpty()));
	mcutl::interrupt::disable_all();
}

TEST_F(interrupt_strict_test_fixture, TraitsTest)
{
	EXPECT_EQ(mcutl::interrupt::maximum_priorities, 16u);
	EXPECT_TRUE(mcutl::interrupt::has_atomic_enable);
	EXPECT_TRUE(mcutl::interrupt::has_atomic_disable);
	EXPECT_TRUE(mcutl::interrupt::has_atomic_set_priority);
	EXPECT_TRUE(mcutl::interrupt::has_atomic_clear_pending);
	EXPECT_TRUE(mcutl::interrupt::has_priorities);
	EXPECT_TRUE(mcutl::interrupt::has_subpriorities);
	EXPECT_TRUE(mcutl::interrupt::has_interrupt_context);
	EXPECT_TRUE(mcutl::interrupt::has_get_active);
}

INSTANTIATE_TEST_SUITE_P(atomic_and_nonatomic_tests,
	atomic_interrupt_strict_test_fixture,
	::testing::Values(true, false));
