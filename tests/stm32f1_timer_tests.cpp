#define STM32F103xG
#define STM32F1

#include <ratio>
#include <stdint.h>
#include <type_traits>

#include "mcutl/clock/clock.h"
#include "mcutl/timer/timer.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph.h"
#include "mcutl/tests/mcu.h"
#include "mcutl/utils/type_helpers.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "stm32f1_exti_interrupt_test_fixture.h"

class timer_strict_test_fixture_base
	: public exti_interrupt_test_fixture
{
};

template<typename Timer> struct timer_map {};
template<typename Periph, typename OverflowInterrupt, uint32_t Reg, uint32_t PeripheralBit>
struct timer_info
{
	using periph = Periph;
	using overflow = OverflowInterrupt;
	static constexpr auto reg = Reg;
	static constexpr auto periph_bit = PeripheralBit;
};
template<> struct timer_map<mcutl::timer::timer2>
	: timer_info<mcutl::periph::timer2, mcutl::interrupt::type::tim2, TIM2_BASE, RCC_APB1ENR_TIM2EN> {};
template<> struct timer_map<mcutl::timer::timer3>
	: timer_info<mcutl::periph::timer3, mcutl::interrupt::type::tim3, TIM3_BASE, RCC_APB1ENR_TIM3EN> {};
template<> struct timer_map<mcutl::timer::timer4>
	: timer_info<mcutl::periph::timer4, mcutl::interrupt::type::tim4, TIM4_BASE, RCC_APB1ENR_TIM4EN> {};
template<> struct timer_map<mcutl::timer::timer5>
	: timer_info<mcutl::periph::timer5, mcutl::interrupt::type::tim5, TIM5_BASE, RCC_APB1ENR_TIM5EN> {};

using timer_clock_config = mcutl::clock::config<mcutl::clock::external_high_speed_crystal<8'000'000>,
	mcutl::clock::timer2_3_4_5_6_7_12_13_14<mcutl::clock::required_frequency<72'000'000>>>;

template<typename Timer>
class timer_strict_test_fixture : public timer_strict_test_fixture_base
{
public:
	using timer = Timer;
	
public:
	static auto timer_reg() noexcept
	{
		return mcutl::memory::volatile_memory<TIM_TypeDef>(timer_map<timer>::reg);
	}
	
	template<typename... Sequences>
	void expect_enable_peripheral(const ::testing::Sequence& s, const Sequences&... seqs)
	{
		EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB1ENR)))
			.InSequence(s, seqs...);
		EXPECT_CALL(this->memory(), write(this->addr(&RCC->APB1ENR),
			timer_map<timer>::periph_bit))
			.InSequence(s, seqs...);
		EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB1ENR)))
			.InSequence(s, seqs...);
	}
	
	void expect_enable_peripheral()
	{
		expect_enable_peripheral({});
	}
	
	void expect_disable_peripheral()
	{
		this->memory().set(this->addr(&RCC->APB1ENR), timer_map<timer>::periph_bit);
		EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB1ENR)));
		EXPECT_CALL(this->memory(), write(this->addr(&RCC->APB1ENR), 0u));
		EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB1ENR)));
	}
	
	void expect_configure_prescaler_and_reload_value(const::testing::Sequence& s1 = {},
		const ::testing::Sequence& s2 = {})
	{
		EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->PSC),
			59u)).InSequence(s1);
		EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->ARR),
			59999u)).InSequence(s2);
		EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->EGR),
			TIM_EGR_UG)).InSequence(s1, s2);
	}
};

using timer_list = ::testing::Types<mcutl::timer::timer2,
	mcutl::timer::timer3, mcutl::timer::timer4, mcutl::timer::timer5>;

template<typename Timer> using timer_list_test_fixture = timer_strict_test_fixture<Timer>;
TYPED_TEST_SUITE(timer_list_test_fixture, timer_list);

TYPED_TEST(timer_list_test_fixture, TraitsTest)
{
	using timer = typename TestFixture::timer;
	
	EXPECT_TRUE(mcutl::timer::supports_stop_on_overflow<timer>);
	EXPECT_TRUE(mcutl::timer::supports_prescalers<timer>);
	EXPECT_TRUE(mcutl::timer::supports_reload_value<timer>);
	EXPECT_TRUE(mcutl::timer::supports_reload_value_buffer<timer>);
	EXPECT_TRUE(mcutl::timer::supports_atomic_clear_pending_flags<timer>);
	EXPECT_EQ(mcutl::timer::default_count_direction<timer>, mcutl::timer::count_direction::up);
	EXPECT_TRUE((std::is_same_v<mcutl::timer::counter_type<timer>, uint16_t>));
	EXPECT_EQ(mcutl::timer::max_value<timer>, 0xffffu);
	EXPECT_EQ(mcutl::timer::default_reload_value<timer>, 0xffffu + 1u);
	EXPECT_TRUE((std::is_same_v<mcutl::timer::peripheral_type<timer>, typename timer_map<timer>::periph>));
	EXPECT_TRUE((std::is_same_v<mcutl::timer::interrupt_type<timer, mcutl::timer::interrupt::overflow>,
		typename timer_map<timer>::overflow>));
	EXPECT_TRUE((mcutl::timer::prescaler_supported<timer, 123>));
	EXPECT_FALSE((mcutl::timer::prescaler_supported<timer, 0>));
	EXPECT_FALSE((mcutl::timer::prescaler_supported<timer, 0xfffff>));
	EXPECT_TRUE((std::is_same_v<mcutl::timer::reload_value_limits<timer>, mcutl::types::limits<2, 65536>>));
}

TYPED_TEST(timer_list_test_fixture, GetCountTest)
{
	using timer = typename TestFixture::timer;
	static constexpr uint16_t count = 1234u;
	
	this->memory().allow_reads(this->addr(&this->timer_reg()->CNT));
	this->memory().set(this->addr(&this->timer_reg()->CNT), count);
	
	EXPECT_EQ(mcutl::timer::get_timer_count<timer>(), count);
}

TYPED_TEST(timer_list_test_fixture, SetCountTest)
{
	using timer = typename TestFixture::timer;
	static constexpr uint16_t count = 1234u;
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CNT), count));
	
	mcutl::timer::set_timer_count<timer>(count);
}

TYPED_TEST(timer_list_test_fixture, PendingFlagsTest)
{
	using timer = typename TestFixture::timer;
	EXPECT_EQ((mcutl::timer::pending_flags_v<timer,
		mcutl::timer::interrupt::overflow>), TIM_SR_UIF);
	EXPECT_EQ(mcutl::timer::pending_flags_v<timer>, 0u);
}

TYPED_TEST(timer_list_test_fixture, GetPendingFlagsTest)
{
	using timer = typename TestFixture::timer;
	this->memory().allow_reads(this->addr(&this->timer_reg()->SR));
	
	EXPECT_EQ((mcutl::timer::get_pending_flags<timer,
		mcutl::timer::interrupt::overflow>()), 0u);
	EXPECT_EQ((mcutl::timer::get_pending_flags<timer>()), 0u);
	
	this->memory().set(this->addr(&this->timer_reg()->SR), TIM_SR_UIF);
	
	EXPECT_EQ((mcutl::timer::get_pending_flags<timer,
		mcutl::timer::interrupt::overflow>()), TIM_SR_UIF);
	EXPECT_EQ((mcutl::timer::get_pending_flags<timer>()), 0u);
}

TYPED_TEST(timer_list_test_fixture, ClearPendingFlagsTest)
{
	static constexpr uint32_t initial_sr = 0xffffffffu;
	static constexpr uint32_t all_flags
		= TIM_SR_CC4OF | TIM_SR_CC3OF | TIM_SR_CC2OF | TIM_SR_CC1OF
		| TIM_SR_TIF | TIM_SR_CC4IF | TIM_SR_CC3IF | TIM_SR_CC2IF | TIM_SR_CC1IF
		| TIM_SR_UIF;
	
	using timer = typename TestFixture::timer;
	this->memory().set(this->addr(&this->timer_reg()->SR), initial_sr);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->SR),
		all_flags & ~TIM_SR_UIF));
	
	mcutl::timer::clear_pending_flags<timer,
		mcutl::timer::interrupt::overflow>();
	
	this->memory().set(this->addr(&this->timer_reg()->SR), initial_sr);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->SR),
		all_flags & ~TIM_SR_UIF));
	
	mcutl::timer::clear_pending_flags_atomic<timer,
		mcutl::timer::interrupt::overflow>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest1)
{
	using timer = typename TestFixture::timer;
	
	this->expect_enable_peripheral();
	
	mcutl::timer::configure<timer,
		mcutl::timer::enable_peripheral<true>,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest2)
{
	using timer = typename TestFixture::timer;
	
	::testing::Sequence s1, s2;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_UDIS))
		.InSequence(s1, s2);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR2),
		TIM_CR2_MMS_1))
		.InSequence(s1);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->DIER),
		TIM_DIER_UIE))
		.InSequence(s2);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_UDIS | TIM_CR1_CEN))
		.InSequence(s1, s2);
	
	mcutl::timer::configure<timer,
		mcutl::timer::direction::down,
		mcutl::timer::stop_on_overflow<true>,
		mcutl::timer::buffer_reload_value<true>,
		mcutl::timer::update_request_source::overflow,
		mcutl::timer::disable_update<true>,
		mcutl::timer::master_mode::output_update,
		mcutl::timer::interrupt::overflow,
		mcutl::timer::enable<true>,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest3)
{
	using timer = typename TestFixture::timer;
	mcutl::timer::configure<timer,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest4)
{
	using timer = typename TestFixture::timer;
	
	::testing::InSequence s;
	this->expect_disable_peripheral();
	
	mcutl::timer::configure<timer,
		mcutl::timer::enable<false>,
		mcutl::timer::enable_peripheral<false>,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest5)
{
	using timer = typename TestFixture::timer;
	
	::testing::InSequence s;
	this->expect_enable_peripheral();
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->DIER),
		TIM_DIER_UIE));
	this->expect_enable_interrupt(timer_map<timer>::overflow::irqn, 5u << 1, 2u);
	
	mcutl::timer::configure<timer,
		mcutl::interrupt::interrupt<mcutl::timer::interrupt::overflow, 5, 2>,
		mcutl::interrupt::priority_count<8>,
		mcutl::timer::interrupt::enable_controller_interrupts,
		mcutl::timer::enable_peripheral<true>,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest6)
{
	using timer = typename TestFixture::timer;
	
	::testing::InSequence s;
	this->expect_enable_peripheral();
	this->expect_disable_interrupt(timer_map<timer>::overflow::irqn);
	
	mcutl::timer::configure<timer,
		mcutl::interrupt::disabled<mcutl::timer::interrupt::overflow>,
		mcutl::interrupt::priority_count<8>,
		mcutl::timer::interrupt::disable_controller_interrupts,
		mcutl::timer::enable_peripheral<true>,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest7)
{
	using timer = typename TestFixture::timer;
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_UDIS));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR2),
		TIM_CR2_MMS_0));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->EGR),
		TIM_EGR_UG));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_UDIS | TIM_CR1_CEN));
	
	mcutl::timer::configure<timer,
		mcutl::timer::direction::down,
		mcutl::timer::stop_on_overflow<false>,
		mcutl::timer::buffer_reload_value<true>,
		mcutl::timer::disable_update<true>,
		mcutl::timer::master_mode::output_enable,
		mcutl::timer::enable<true>,
		mcutl::timer::trigger_registers_update,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest8)
{
	using timer = typename TestFixture::timer;
	
	constexpr uint32_t reload_value = 0x1234u;
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->ARR),
		reload_value - 1u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->EGR),
		TIM_EGR_UG));
	
	mcutl::timer::configure<timer,
		mcutl::timer::reload_value<reload_value>,
		mcutl::timer::trigger_registers_update,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest9)
{
	using timer = typename TestFixture::timer;
	
	constexpr uint32_t prescaler = 0x5678;
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->PSC),
		prescaler - 1u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->EGR),
		TIM_EGR_UG));
	
	mcutl::timer::configure<timer,
		mcutl::timer::prescaler<prescaler>,
		mcutl::timer::trigger_registers_update,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest10)
{
	using timer = typename TestFixture::timer;
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->PSC),
		10285u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->EGR),
		TIM_EGR_UG));
	
	mcutl::timer::configure<timer,
		mcutl::timer::timer_frequency<timer_clock_config, std::ratio<7'000>, std::ratio<1, 2>>,
		mcutl::timer::trigger_registers_update,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureWithBaseConfigurationTest11)
{
	using timer = typename TestFixture::timer;
	
	this->expect_configure_prescaler_and_reload_value();
	
	mcutl::timer::configure<timer,
		mcutl::timer::overflow_frequency<timer_clock_config, std::ratio<20>, std::ratio<1, 5>>,
		mcutl::timer::trigger_registers_update,
		mcutl::timer::base_configuration_is_currently_present>();
	
	this->expect_configure_prescaler_and_reload_value();
	
	mcutl::timer::configure<timer,
		mcutl::timer::exact_overflow_frequency<timer_clock_config, std::ratio<20>, true>,
		mcutl::timer::trigger_registers_update,
		mcutl::timer::base_configuration_is_currently_present>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureTest1)
{
	using timer = typename TestFixture::timer;
	
	::testing::Sequence s1, s2, s3, s4, s5;
	this->expect_enable_peripheral(s1, s2, s3, s4, s5);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1), 0u))
		.InSequence(s1);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR2), 0u))
		.InSequence(s2);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->PSC), 0u))
		.InSequence(s3);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->ARR), 0xffffu))
		.InSequence(s4);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->DIER), 0u))
		.InSequence(s5);
	
	mcutl::timer::configure<timer,
		mcutl::timer::enable_peripheral<true>>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureTest2)
{
	using timer = typename TestFixture::timer;
	
	::testing::Sequence s1, s2, s3, s4;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_UDIS))
		.InSequence(s1, s2, s3, s4);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR2),
		TIM_CR2_MMS_1))
		.InSequence(s1);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->PSC), 0u))
		.InSequence(s2);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->ARR), 0xffffu))
		.InSequence(s3);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->DIER),
		TIM_DIER_UIE))
		.InSequence(s4);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_UDIS | TIM_CR1_CEN))
		.InSequence(s1, s2, s3, s4);
	
	mcutl::timer::configure<timer,
		mcutl::timer::direction::down,
		mcutl::timer::stop_on_overflow<true>,
		mcutl::timer::buffer_reload_value<true>,
		mcutl::timer::update_request_source::overflow,
		mcutl::timer::disable_update<true>,
		mcutl::timer::master_mode::output_update,
		mcutl::timer::interrupt::overflow,
		mcutl::timer::enable<true>>();
}

TYPED_TEST(timer_list_test_fixture, ConfigureTest3)
{
	using timer = typename TestFixture::timer;
	
	::testing::Sequence s1, s2;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1), 0u))
		.InSequence(s1);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR2), 0u))
		.InSequence(s2);
	this->expect_configure_prescaler_and_reload_value(s1, s2);
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->DIER), 0u))
		.InSequence(s1, s2);
	
	mcutl::timer::configure<timer,
		mcutl::timer::overflow_frequency<timer_clock_config, std::ratio<20>, std::ratio<1, 5>>,
		mcutl::timer::trigger_registers_update>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest1)
{
	using timer = typename TestFixture::timer;
	mcutl::timer::reconfigure<timer>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest2)
{
	using timer = typename TestFixture::timer;
	
	static constexpr uint32_t initial_cr1 = 0xffffffffu & ~TIM_CR1_CEN;
	
	this->memory().set(this->addr(&this->timer_reg()->CR1), initial_cr1);
	this->memory().allow_reads(this->addr(&this->timer_reg()->CR1));
	
	::testing::InSequence s;
	this->expect_enable_peripheral();
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		initial_cr1 | TIM_CR1_CEN));
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::enable_peripheral<true>,
		mcutl::timer::enable<true>>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest3)
{
	using timer = typename TestFixture::timer;
	
	static constexpr uint32_t initial_cr1 = 0xffffffffu;
	
	this->memory().set(this->addr(&this->timer_reg()->CR1), initial_cr1);
	this->memory().allow_reads(this->addr(&this->timer_reg()->CR1));
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		initial_cr1 & ~TIM_CR1_CEN));
	this->expect_disable_peripheral();
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::enable_peripheral<false>,
		mcutl::timer::enable<false>>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest4)
{
	using timer = typename TestFixture::timer;
	
	static constexpr uint32_t initial_cr1 = 0xffffffffu;
	static constexpr uint32_t initial_cr2 = 0xffffffffu;
	
	this->memory().set(this->addr(&this->timer_reg()->CR1), initial_cr1);
	this->memory().set(this->addr(&this->timer_reg()->CR2), initial_cr2);
	this->memory().allow_reads(this->addr(&this->timer_reg()->CR1));
	this->memory().allow_reads(this->addr(&this->timer_reg()->CR2));
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		initial_cr1 & ~(TIM_CR1_CEN | TIM_CR1_OPM | TIM_CR1_DIR)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR2),
		initial_cr2 & ~TIM_CR2_MMS_Msk));
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->CR1),
		initial_cr1 & ~(TIM_CR1_OPM | TIM_CR1_DIR)));
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::direction::up,
		mcutl::timer::stop_on_overflow<false>,
		mcutl::timer::master_mode::none>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest5)
{
	using timer = typename TestFixture::timer;
	
	this->expect_configure_prescaler_and_reload_value();
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::exact_overflow_frequency<timer_clock_config, std::ratio<20>>,
		mcutl::timer::trigger_registers_update>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest6)
{
	using timer = typename TestFixture::timer;
	
	static constexpr uint32_t initial_dier = 0xffffffffu & ~TIM_DIER_UIE;
	
	this->memory().allow_reads(this->addr(&this->timer_reg()->DIER));
	this->memory().set(this->addr(&this->timer_reg()->DIER), initial_dier);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->timer_reg()->DIER),
		initial_dier | TIM_DIER_UIE));
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::interrupt::overflow>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest7)
{
	using timer = typename TestFixture::timer;
	
	static constexpr uint32_t initial_dier = 0xffffffffu & ~TIM_DIER_UIE;
	
	this->memory().allow_reads(this->addr(&this->timer_reg()->DIER));
	this->memory().set(this->addr(&this->timer_reg()->DIER), initial_dier);
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::interrupt::enable_controller_interrupts>();
	
	this->memory().set(this->addr(&this->timer_reg()->DIER),
		initial_dier | TIM_DIER_UIE);
	
	this->expect_enable_interrupt(timer_map<timer>::overflow::irqn);
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::interrupt::enable_controller_interrupts>();
}

TYPED_TEST(timer_list_test_fixture, ReconfigureTest8)
{
	using timer = typename TestFixture::timer;
	
	static constexpr uint32_t initial_dier = 0xffffffffu;
	
	this->memory().allow_reads(this->addr(&this->timer_reg()->DIER));
	this->memory().set(this->addr(&this->timer_reg()->DIER), initial_dier);
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::interrupt::disable_controller_interrupts>();
	
	this->memory().set(this->addr(&this->timer_reg()->DIER),
		initial_dier & ~TIM_DIER_UIE);
	
	this->expect_disable_interrupt(timer_map<timer>::overflow::irqn);
	
	mcutl::timer::reconfigure<timer,
		mcutl::timer::interrupt::disable_controller_interrupts>();
}
