#define STM32F107xC
#define STM32F1

#include <stdint.h>
#include <type_traits>

#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph_defs.h"
#include "mcutl/systick/systick.h"
#include "mcutl/systick/systick_wait.h"
#include "mcutl/tests/mcu.h"
#include "mcutl/utils/duration.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class systick_strict_test_fixture
	: public mcutl::tests::mcu::strict_test_fixture_base
{
};

TEST_F(systick_strict_test_fixture, TraitsTest)
{
	EXPECT_TRUE((std::is_same_v<mcutl::systick::interrupt_type<
		mcutl::systick::interrupt::tick>, mcutl::interrupt::type::systick>));
	EXPECT_TRUE((std::is_same_v<mcutl::systick::value_type, uint32_t>));
	EXPECT_TRUE((std::is_same_v<mcutl::systick::peripheral_type, mcutl::periph::no_periph>));
	
	EXPECT_EQ(mcutl::systick::min_value, 0u);
	EXPECT_EQ(mcutl::systick::max_value, 0xffffffu);
	EXPECT_EQ(mcutl::systick::count_direction, mcutl::systick::direction_type::top_down);
	
	EXPECT_TRUE(mcutl::systick::supports_overflow_detection);
	EXPECT_TRUE(mcutl::systick::supports_value_request);
	EXPECT_TRUE(mcutl::systick::supports_reload_value);
	EXPECT_TRUE(mcutl::systick::supports_reset_value);
	EXPECT_TRUE(mcutl::systick::supports_reload_value_change);
	EXPECT_TRUE(mcutl::systick::supports_atomic_clear_pending_flags);
	EXPECT_TRUE(mcutl::systick::overflow_flag_cleared_on_read);
}

TEST_F(systick_strict_test_fixture, ClearPendingFlagTest)
{
	EXPECT_CALL(memory(), write(addr(&SCB->ICSR), SCB_ICSR_PENDSTCLR_Msk));
	mcutl::systick::clear_pending_flags<mcutl::systick::interrupt::tick>();
}

TEST_F(systick_strict_test_fixture, AtomicClearPendingFlagTest)
{
	EXPECT_CALL(memory(), write(addr(&SCB->ICSR), SCB_ICSR_PENDSTCLR_Msk));
	mcutl::systick::clear_pending_flags_atomic<mcutl::systick::interrupt::tick>();
}

TEST_F(systick_strict_test_fixture, ClearPendingFlagEmptyTest)
{
	mcutl::systick::clear_pending_flags_atomic<>();
}

TEST_F(systick_strict_test_fixture, GetValueTest)
{
	memory().set(addr(&SysTick->VAL), 0x123456u);
	EXPECT_CALL(memory(), read(addr(&SysTick->VAL)));
	EXPECT_EQ(mcutl::systick::get_value(), 0x123456u);
}

TEST_F(systick_strict_test_fixture, HasOverflownTest)
{
	memory().allow_reads(addr(&SysTick->CTRL));
	
	EXPECT_FALSE(mcutl::systick::has_overflown());
	
	memory().set(addr(&SysTick->CTRL), SysTick_CTRL_COUNTFLAG_Msk);
	EXPECT_TRUE(mcutl::systick::has_overflown());
}

TEST_F(systick_strict_test_fixture, GetReloadValueTest)
{
	memory().set(addr(&SysTick->LOAD), 0x123456u);
	EXPECT_CALL(memory(), read(addr(&SysTick->LOAD)));
	EXPECT_EQ(mcutl::systick::get_reload_value(), 0x123456u);
}

TEST_F(systick_strict_test_fixture, SetReloadValueTest)
{
	EXPECT_CALL(memory(), write(addr(&SysTick->LOAD), 0x123456u));
	mcutl::systick::set_reload_value(0x123456u);
}

TEST_F(systick_strict_test_fixture, ResetReloadValueTest)
{
	EXPECT_CALL(memory(), write(addr(&SysTick->LOAD),
		mcutl::systick::get_default_reload_value()));
	mcutl::systick::reset_reload_value();
}

TEST_F(systick_strict_test_fixture, ResetValueTest)
{
	EXPECT_CALL(memory(), write(addr(&SysTick->VAL), ::testing::_));
	mcutl::systick::reset_value();
}

TEST_F(systick_strict_test_fixture, GetDefaultReloadValueTest)
{
	EXPECT_EQ(mcutl::systick::get_default_reload_value(), 0xffffffu);
}

TEST_F(systick_strict_test_fixture, ConfigureTest1)
{
	memory().set(addr(&SysTick->CTRL), 0xffffffffu);
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL), SysTick_CTRL_ENABLE_Msk));
	mcutl::systick::configure<
		mcutl::systick::clock_source::external,
		mcutl::systick::enable<true>
	>();
}

TEST_F(systick_strict_test_fixture, ConfigureTest2)
{
	memory().set(addr(&SysTick->CTRL), 0xffffffffu);
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL), SysTick_CTRL_CLKSOURCE_Msk));
	mcutl::systick::configure<
		mcutl::systick::clock_source::processor,
		mcutl::systick::enable<false>,
		mcutl::interrupt::disabled<mcutl::systick::interrupt::tick>
	>();
}

TEST_F(systick_strict_test_fixture, ConfigureWithInterruptTest)
{
	memory().set(addr(&SysTick->CTRL), 0xffffffffu);
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL),
		SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk));
	mcutl::systick::configure<
		mcutl::systick::clock_source::processor,
		mcutl::systick::enable<true>,
		mcutl::systick::interrupt::tick
	>();
}

TEST_F(systick_strict_test_fixture, ConfigureWithInterruptPriorityTest)
{
	memory().set(addr(&SysTick->CTRL), 0xffffffffu);
	
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL),
		SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk));
	EXPECT_CALL(memory(), write(addr(&(SCB->SHP[11])), (3u << 5) | 5u));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
		::testing::IsEmpty()));
	
	mcutl::systick::configure<
		mcutl::interrupt::interrupt<mcutl::systick::interrupt::tick, 3, 5>,
		mcutl::interrupt::priority_count<8>
	>();
}

TEST_F(systick_strict_test_fixture, ReconfigureTest1)
{
	constexpr uint32_t initial_ctrl = 0xffffffffu & ~SysTick_CTRL_ENABLE_Msk;
	memory().allow_reads(addr(&SysTick->CTRL));
	memory().set(addr(&SysTick->CTRL), initial_ctrl);
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL),
		(initial_ctrl & ~SysTick_CTRL_CLKSOURCE_Msk) | SysTick_CTRL_ENABLE_Msk));
	mcutl::systick::reconfigure<
		mcutl::systick::clock_source::external,
		mcutl::systick::enable<true>
	>();
}

TEST_F(systick_strict_test_fixture, ReconfigureTest2)
{
	constexpr uint32_t initial_ctrl = 0xffffffffu;
	memory().allow_reads(addr(&SysTick->CTRL));
	memory().set(addr(&SysTick->CTRL), initial_ctrl);
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL),
		(initial_ctrl & ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk))));
	mcutl::systick::reconfigure<
		mcutl::systick::enable<false>,
		mcutl::interrupt::disabled<mcutl::systick::interrupt::tick>
	>();
}

TEST_F(systick_strict_test_fixture, ReconfigureWithInterruptTest)
{
	constexpr uint32_t initial_ctrl = 0xffffffffu & ~SysTick_CTRL_TICKINT_Msk;
	memory().allow_reads(addr(&SysTick->CTRL));
	memory().set(addr(&SysTick->CTRL), initial_ctrl);
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL),
		initial_ctrl | SysTick_CTRL_TICKINT_Msk));
	mcutl::systick::reconfigure<
		mcutl::systick::clock_source::processor,
		mcutl::systick::enable<true>,
		mcutl::systick::interrupt::tick
	>();
}

TEST_F(systick_strict_test_fixture, ReconfigureWithInterruptPriorityTest)
{
	constexpr uint32_t initial_ctrl = 0xffffffffu & ~SysTick_CTRL_TICKINT_Msk;
	memory().allow_reads(addr(&SysTick->CTRL));
	memory().set(addr(&SysTick->CTRL), initial_ctrl);
	
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&SysTick->CTRL),
		initial_ctrl | SysTick_CTRL_TICKINT_Msk));
	EXPECT_CALL(memory(), write(addr(&(SCB->SHP[11])), (3u << 5) | 5u));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
		::testing::IsEmpty()));
	
	mcutl::systick::reconfigure<
		mcutl::interrupt::interrupt<mcutl::systick::interrupt::tick, 3, 5>,
		mcutl::interrupt::priority_count<8>
	>();
}

TEST_F(systick_strict_test_fixture, ClearOverflowFlagTest)
{
	EXPECT_CALL(memory(), read(addr(&SysTick->CTRL)));
	mcutl::systick::clear_overflow_flag();
}

TEST_F(systick_strict_test_fixture, WaitIntervalsTests)
{
	EXPECT_EQ((mcutl::systick::detail::get_ticks_to_wait<
		72'000'000u, mcutl::types::milliseconds<12>, uint32_t>()), 864000u);
	EXPECT_EQ((mcutl::systick::detail::get_ticks_to_wait<
		72'000'000u, mcutl::types::microseconds<25>, uint32_t>()), 1800u);
	EXPECT_EQ((mcutl::systick::detail::get_ticks_to_wait<
		72'000'000u, mcutl::types::nanoseconds<123>, uint32_t>()), 9u);
	EXPECT_EQ((mcutl::systick::detail::get_ticks_to_wait<
		72'000'000u, mcutl::types::seconds<3>, uint32_t>()), 216000000u);
}

TEST_F(systick_strict_test_fixture, WaitTest)
{
	memory().allow_reads(addr(&SysTick->LOAD));
	memory().allow_reads(addr(&SysTick->VAL));
	memory().set(addr(&SysTick->VAL), 10'000'000u);
	memory().set(addr(&SysTick->LOAD),
		mcutl::systick::get_default_reload_value());
	
	EXPECT_CALL(memory(), read(addr(&SysTick->CTRL)))
		.Times(3)
		.WillRepeatedly([] (auto) {
			return SysTick_CTRL_COUNTFLAG_Msk;
		});
	
	mcutl::systick::wait_msec<72'000'000u, 500u>(); //36'000'000 systicks
}
