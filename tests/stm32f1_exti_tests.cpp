#define STM32F107xC
#define STM32F1

#include "mcutl/exti/exti.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using exti_strict_test_fixture_base = mcutl::tests::mcu::strict_test_fixture_base;

class exti_interrupt_test_fixture : public exti_strict_test_fixture_base
{
public:
	void expect_enable(uint32_t irqn)
	{
		::testing::InSequence s;
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
			::testing::IsEmpty()));
		EXPECT_CALL(memory(), write(addr(&(NVIC->ISER[irqn / 32])), (1 << (irqn % 32))));
	}
	
	void expect_disable(uint32_t irqn)
	{
		::testing::InSequence s;
		EXPECT_CALL(memory(), write(addr(&(NVIC->ICER[irqn / 32])), (1 << (irqn % 32))));
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dsb>(),
				::testing::IsEmpty()));
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
				::testing::IsEmpty()));
	}
	
	void expect_enable(uint32_t irqn, uint32_t priority, uint32_t subpriority)
	{
		::testing::InSequence s;
		expect_set_priority(irqn, priority, subpriority);
		expect_enable(irqn);
	}
	
	void expect_set_priority(uint32_t irqn, uint32_t priority, uint32_t subpriority)
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
	
	void expect_clear_pending(uint32_t irqn)
	{
		EXPECT_CALL(memory(), write(addr(&(NVIC->ICPR[irqn / 32])), (1 << (irqn % 32))));
	}
};

class enable_disable_test_fixture
	: public exti_strict_test_fixture_base
	, public testing::WithParamInterface<bool>
{
public:
	void expect_enable_disable(uint32_t mask, uint32_t ft, uint32_t rt,
		uint32_t em, uint32_t im)
	{
		memory().set(addr(&EXTI->FTSR), 0x12345678);
		memory().set(addr(&EXTI->RTSR), 0x90abcdef);
		memory().set(addr(&EXTI->EMR), 0x12345678);
		memory().set(addr(&EXTI->IMR), 0x90abcdef);
	
		memory().allow_reads(addr(&EXTI->FTSR));
		memory().allow_reads(addr(&EXTI->RTSR));
		memory().allow_reads(addr(&EXTI->EMR));
		memory().allow_reads(addr(&EXTI->IMR));
		
		if (!GetParam())
			ft = rt = em = im = 0;
		
		//These registers may be written in any order
		EXPECT_CALL(memory(), write(addr(&EXTI->FTSR),
			(0x12345678 & ~mask) | ft));
		EXPECT_CALL(memory(), write(addr(&EXTI->RTSR),
			(0x90abcdef & ~mask) | rt));
		EXPECT_CALL(memory(), write(addr(&EXTI->EMR),
			(0x12345678 & ~mask) | em));
		EXPECT_CALL(memory(), write(addr(&EXTI->IMR),
			(0x90abcdef & ~mask) | im));
	}
	
	template<typename... T>
	void enable_disable()
	{
		if (GetParam())
			mcutl::exti::enable_lines<T...>();
		else
			mcutl::exti::disable_lines<T...>();
	}
};

class pending_bits_test_fixture
	: public exti_strict_test_fixture_base
	, public testing::WithParamInterface<bool>
{
public:
	template<typename... T>
	void clear_pending()
	{
		if (GetParam())
			mcutl::exti::clear_pending_line_bits<T...>();
		else
			mcutl::exti::clear_pending_line_bits_atomic<T...>();
	}
	
	void expect_clear_pending_bits(uint32_t mask)
	{
		EXPECT_CALL(memory(), write(addr(&EXTI->PR), mask));
	}
};

class software_trigger_test_fixture
	: public exti_strict_test_fixture_base
	, public testing::WithParamInterface<bool>
{
public:
	template<typename... T>
	void software_trigger()
	{
		if (GetParam())
			mcutl::exti::software_trigger<T...>();
		else
			mcutl::exti::software_trigger_atomic<T...>();
	}
	
	void expect_software_trigger(uint32_t mask)
	{
		EXPECT_CALL(memory(), write(addr(&EXTI->SWIER), mask));
	}
};

TEST_P(enable_disable_test_fixture, EnableDisableSingleLineTest)
{
	expect_enable_disable(1 << 5, 1 << 5, 1 << 5, 1 << 5, 1 << 5);
	enable_disable<
		mcutl::exti::line_options<
			mcutl::exti::line<5>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::rising_and_falling
		>
	>();
}

TEST_P(enable_disable_test_fixture, EnableDisableComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_options<
			mcutl::exti::line<1>,
			mcutl::exti::line_mode::event,
			mcutl::exti::line_trigger::rising
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<2>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::rising_and_falling
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<6>,
			mcutl::exti::line_mode::interrupt,
			mcutl::exti::line_trigger::falling
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<15>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::software_only
		>
	>;
	
	expect_enable_disable(1 << 1 | 1 << 2 | 1 << 6 | 1 << 15,
		1 << 2 | 1 << 6,
		1 << 1 | 1 << 2,
		1 << 1 | 1 << 2 | 1 << 15,
		1 << 2 | 1 << 6 | 1 << 15);
	
	enable_disable<config>();
}

TEST_P(enable_disable_test_fixture, EnableDisableEmptyTest)
{
	enable_disable();
}

TEST_P(pending_bits_test_fixture, ClearPendingLineBitsSimpleTest)
{
	expect_clear_pending_bits(1 << 17);
	clear_pending<
		mcutl::exti::line_options<
			mcutl::exti::line<17>,
			mcutl::exti::line_mode::event,
			mcutl::exti::line_trigger::software_only
		>
	>();
}

TEST_P(pending_bits_test_fixture, ClearPendingLineBitsComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_options<
			mcutl::exti::line<0>,
			mcutl::exti::line_mode::event,
			mcutl::exti::line_trigger::rising
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<3>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::rising_and_falling
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<18>,
			mcutl::exti::line_mode::interrupt,
			mcutl::exti::line_trigger::falling
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<8>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::software_only
		>
	>;
	
	expect_clear_pending_bits(1 << 0 | 1 << 3 | 1 << 8 | 1 << 18);
	clear_pending<config>();
}

TEST_P(pending_bits_test_fixture, ClearPendingLineBitsEmptyTest)
{
	clear_pending();
}

TEST_F(exti_strict_test_fixture_base, GetPendingLineBitsTest)
{
	using line19 = mcutl::exti::line_options<
		mcutl::exti::line<19>,
		mcutl::exti::line_mode::event,
		mcutl::exti::line_trigger::software_only
	>;
	using line2 = mcutl::exti::line_options<
		mcutl::exti::line<2>,
		mcutl::exti::line_mode::interrupt,
		mcutl::exti::line_trigger::rising_and_falling
	>;
	using line7 = mcutl::exti::line_options<
		mcutl::exti::line<7>,
		mcutl::exti::line_mode::event_and_interrupt,
		mcutl::exti::line_trigger::rising
	>;
	using line5 = mcutl::exti::line_options<
		mcutl::exti::line<5>,
		mcutl::exti::line_mode::event_and_interrupt,
		mcutl::exti::line_trigger::rising
	>;
	
	memory().set(addr(&EXTI->PR), 0b1100'10101000'11000011);
	memory().allow_reads(addr(&EXTI->PR));
	
	EXPECT_TRUE((mcutl::exti::get_pending_line_bits<line19>()));
	EXPECT_FALSE((mcutl::exti::get_pending_line_bits<line2>()));
	EXPECT_TRUE((mcutl::exti::get_pending_line_bits<line7>()));
	
	EXPECT_TRUE((mcutl::exti::get_pending_line_bits<line19, line2>()));
	EXPECT_TRUE((mcutl::exti::get_pending_line_bits<mcutl::exti::config<line19, line2>>()));
	EXPECT_FALSE((mcutl::exti::get_pending_line_bits<line5, line2>()));
	EXPECT_FALSE((mcutl::exti::get_pending_line_bits<mcutl::exti::config<line5, line2>>()));
	
	EXPECT_EQ((mcutl::exti::get_pending_line_bits<line5, line2, line19, line7>()),
		(mcutl::exti::lines_bit_mask_v<line7, line19>));
	EXPECT_EQ((mcutl::exti::get_pending_line_bits<mcutl::exti::config<line5, line2, line19, line7>>()),
		(mcutl::exti::lines_bit_mask_v<mcutl::exti::config<line7, line19>>));
}

TEST_P(software_trigger_test_fixture, SoftwareTriggerSimpleTest)
{
	expect_software_trigger(1 << 16);
	software_trigger<
		mcutl::exti::line_options<
			mcutl::exti::line<16>,
			mcutl::exti::line_mode::event,
			mcutl::exti::line_trigger::software_only
		>
	>();
}

TEST_P(software_trigger_test_fixture, SoftwareTriggerComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_options<
			mcutl::exti::line<2>,
			mcutl::exti::line_mode::event,
			mcutl::exti::line_trigger::rising
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<4>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::rising_and_falling
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<9>,
			mcutl::exti::line_mode::interrupt,
			mcutl::exti::line_trigger::falling
		>,
		mcutl::exti::line_options<
			mcutl::exti::line<12>,
			mcutl::exti::line_mode::event_and_interrupt,
			mcutl::exti::line_trigger::software_only
		>
	>;
	
	expect_software_trigger(1 << 2 | 1 << 4 | 1 << 9 | 1 << 12);
	software_trigger<config>();
}

TEST_P(software_trigger_test_fixture, SoftwareTriggerEmptyTest)
{
	software_trigger();
}

TEST_F(exti_interrupt_test_fixture, EnableInterruptsSimpleTest)
{
	expect_enable(EXTI15_10_IRQn);
	
	mcutl::exti::enable_interrupts<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<14>
		>
	>();
}

TEST_F(exti_interrupt_test_fixture, DisableInterruptsSimpleTest)
{
	expect_disable(EXTI15_10_IRQn);
	
	mcutl::exti::disable_interrupts<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<11>
		>
	>();
}

TEST_F(exti_interrupt_test_fixture, EnableInterruptsComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<11>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<10>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<7>,
			5,
			3
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<1>,
			10
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<6>,
			5,
			3
		>
	>;
	
	expect_enable(EXTI15_10_IRQn);
	expect_enable(EXTI9_5_IRQn, 5, 3);
	expect_enable(EXTI1_IRQn, 10, mcutl::interrupt::default_priority);
	
	mcutl::exti::enable_interrupts<config>();
}

TEST_F(exti_interrupt_test_fixture, DisableInterruptsComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<12>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<13>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<5>,
			1,
			2
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<0>,
			7
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<8>,
			1,
			2
		>
	>;
	
	expect_disable(EXTI15_10_IRQn);
	expect_disable(EXTI9_5_IRQn);
	expect_disable(EXTI0_IRQn);
	
	mcutl::exti::disable_interrupts<config>();
}

TEST_F(exti_interrupt_test_fixture, SetInterruptProiritiesComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<11>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<10>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<7>,
			5,
			3
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<1>,
			10
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<6>,
			5,
			3
		>
	>;
	
	expect_set_priority(EXTI9_5_IRQn, 5, 3);
	expect_set_priority(EXTI1_IRQn, 10, mcutl::interrupt::default_priority);
	
	mcutl::exti::set_interrupt_prioritites<config>();
}

TEST_F(exti_interrupt_test_fixture, SetInterruptProiritiesComplexConfig2Test)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<11>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<10>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<7>,
			5,
			3
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<1>,
			6
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<6>,
			5,
			3
		>
	>;
	
	expect_set_priority(EXTI9_5_IRQn, 5 << 1, 3);
	expect_set_priority(EXTI1_IRQn, 6 << 1, mcutl::interrupt::default_priority);
	
	mcutl::exti::set_interrupt_prioritites<8, config>();
}

TEST_F(exti_interrupt_test_fixture, ClearPendingInterruptsSimpleTest)
{
	expect_clear_pending(EXTI4_IRQn);
	
	mcutl::exti::clear_pending_interrupts<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<4>
		>
	>();
}

TEST_F(exti_interrupt_test_fixture, ClearPendingInterruptsComplexConfigTest)
{
	using config = mcutl::exti::config<
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<3>
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<5>,
			6
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<6>,
			6
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<7>,
			6
		>,
		mcutl::exti::line_interrupt_options<
			mcutl::exti::line<11>,
			5,
			3
		>
	>;
	
	expect_clear_pending(EXTI9_5_IRQn);
	expect_clear_pending(EXTI15_10_IRQn);
	expect_clear_pending(EXTI3_IRQn);
	
	mcutl::exti::clear_pending_interrupts<config>();
}

TEST_F(exti_strict_test_fixture_base, TraitsTest)
{
	EXPECT_TRUE(mcutl::exti::has_events);
	EXPECT_TRUE(mcutl::exti::has_rising_trigger);
	EXPECT_TRUE(mcutl::exti::has_falling_trigger);
	EXPECT_TRUE(mcutl::exti::has_rising_and_falling_trigger);
	EXPECT_TRUE(mcutl::exti::has_software_trigger);
	EXPECT_TRUE(mcutl::exti::has_pending_line_bits);
	EXPECT_TRUE(mcutl::exti::has_atomic_clear_pending_line_bits);
	EXPECT_TRUE(mcutl::exti::has_atomic_software_trigger);
}

INSTANTIATE_TEST_SUITE_P(enable_disable_tests,
	enable_disable_test_fixture,
	::testing::Values(true, false));

INSTANTIATE_TEST_SUITE_P(pending_bits_tests,
	pending_bits_test_fixture,
	::testing::Values(true, false));

INSTANTIATE_TEST_SUITE_P(software_trigger_tests,
	software_trigger_test_fixture,
	::testing::Values(true, false));
