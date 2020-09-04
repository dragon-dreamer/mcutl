#define STM32F103xG
#define STM32F1

#include <stdint.h>
#include <type_traits>

#include "mcutl/adc/adc.h"
#include "mcutl/clock/clock.h"
#include "mcutl/dma/dma.h"
#include "mcutl/gpio/gpio.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph.h"
#include "mcutl/systick/systick.h"
#include "mcutl/tests/mcu.h"
#include "mcutl/utils/type_helpers.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "stm32f1_dma_test_fixture.h"
#include "stm32f1_exti_interrupt_test_fixture.h"

class adc_strict_test_fixture_base
	: public dma_strict_test_fixture
	, public exti_interrupt_test_fixture
{
public:
	template<typename Adc, uint8_t... Channels, typename... Gpios>
	void test_gpio_mapping(mcutl::types::list<Gpios...>)
	{
		(... , test_single_gpio_mapping<Adc, Channels, Gpios>());
	}
	
private:
	template<typename Adc, uint8_t Channel, typename Gpio>
	void test_single_gpio_mapping()
	{
		EXPECT_TRUE((std::is_same_v<mcutl::adc::map_adc_channel_to_gpio<Adc, mcutl::adc::channel<Channel>>,
			Gpio>));
		EXPECT_TRUE((std::is_same_v<mcutl::adc::map_gpio_to_adc_channel<Adc, Gpio>,
			mcutl::adc::channel<Channel>>));
	}
};

template<typename Adc>
class adc_strict_test_fixture : public adc_strict_test_fixture_base
{
public:
	using adc = Adc;
	
	auto get_adc() noexcept
	{
		if constexpr (std::is_same_v<Adc, mcutl::adc::adc1>)
			return ADC1;
		else if constexpr (std::is_same_v<Adc, mcutl::adc::adc2>)
			return ADC2;
		else
			return ADC3;
	}
	
	void expect_wait_calibration_winished()
	{
		EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)))
			.Times(2)
			.WillOnce(::testing::DoDefault())
			.WillRepeatedly([] (auto) { return 0u; });
	}
	
	void expect_enable_peripheral()
	{
		EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB2ENR)));
		EXPECT_CALL(this->memory(), write(this->addr(&RCC->APB2ENR),
			this->get_adc_periph_bit()));
		EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB2ENR)));
	}
	
	static uint32_t get_adc_periph_bit() noexcept
	{
		if constexpr (std::is_same_v<adc, mcutl::adc::adc1>)
			return RCC_APB2ENR_ADC1EN;
		else if constexpr (std::is_same_v<adc, mcutl::adc::adc2>)
			return RCC_APB2ENR_ADC2EN;
		else
			return RCC_APB2ENR_ADC3EN;
	}
	
	static uint32_t get_dma_channel() noexcept
	{
		static_assert(!std::is_same_v<adc, mcutl::adc::adc2>);
		return std::is_same_v<adc, mcutl::adc::adc1>
			? DMA1_Channel1_BASE : DMA2_Channel5_BASE;
	}
	
	void setup_systick()
	{
		this->memory().allow_reads(this->addr(&SysTick->LOAD));
		this->memory().allow_reads(this->addr(&SysTick->VAL));
		this->memory().set(this->addr(&SysTick->LOAD), 10u);
	}
	
	void expect_wait_init()
	{
		EXPECT_CALL(this->memory(), read(this->addr(&SysTick->CTRL)))
			.Times(10)
			.WillRepeatedly([] (auto) {
				return SysTick_CTRL_COUNTFLAG_Msk;
			});
	}
};

TEST_F(adc_strict_test_fixture_base, TraitsTest)
{
	EXPECT_TRUE(mcutl::adc::supports_calibration<mcutl::adc::adc1>);
	EXPECT_TRUE(mcutl::adc::supports_calibration<mcutl::adc::adc2>);
	EXPECT_TRUE(mcutl::adc::supports_calibration<mcutl::adc::adc3>);
	EXPECT_TRUE(mcutl::adc::supports_input_impedance_option<mcutl::adc::adc1>);
	EXPECT_TRUE(mcutl::adc::supports_input_impedance_option<mcutl::adc::adc2>);
	EXPECT_TRUE(mcutl::adc::supports_input_impedance_option<mcutl::adc::adc3>);
	EXPECT_TRUE(mcutl::adc::supports_atomic_clear_pending_flags<mcutl::adc::adc1>);
	EXPECT_TRUE(mcutl::adc::supports_atomic_clear_pending_flags<mcutl::adc::adc2>);
	EXPECT_TRUE(mcutl::adc::supports_atomic_clear_pending_flags<mcutl::adc::adc3>);
	EXPECT_TRUE(mcutl::adc::supports_data_alignment<mcutl::adc::adc1>);
	EXPECT_TRUE(mcutl::adc::supports_data_alignment<mcutl::adc::adc2>);
	EXPECT_TRUE(mcutl::adc::supports_data_alignment<mcutl::adc::adc3>);
	
	EXPECT_TRUE((std::is_same_v<mcutl::adc::peripheral_type<mcutl::adc::adc1>, mcutl::periph::adc1>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::peripheral_type<mcutl::adc::adc2>, mcutl::periph::adc2>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::peripheral_type<mcutl::adc::adc3>, mcutl::periph::adc3>));
	
	EXPECT_TRUE((std::is_same_v<mcutl::adc::interrupt_type<mcutl::adc::adc1,
		mcutl::adc::init::interrupt::conversion_complete>, mcutl::interrupt::type::adc1_2>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::interrupt_type<mcutl::adc::adc2,
		mcutl::adc::init::interrupt::conversion_complete>, mcutl::interrupt::type::adc1_2>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::interrupt_type<mcutl::adc::adc3,
		mcutl::adc::init::interrupt::conversion_complete>, mcutl::interrupt::type::adc3>));
	
	EXPECT_TRUE((std::is_same_v<mcutl::adc::conversion_result_type<mcutl::adc::adc1>, uint16_t>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::conversion_result_type<mcutl::adc::adc2>, uint16_t>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::conversion_result_type<mcutl::adc::adc3>, uint16_t>));
	
	EXPECT_EQ(mcutl::adc::resolution_bits<mcutl::adc::adc1>, 12);
	EXPECT_EQ(mcutl::adc::resolution_bits<mcutl::adc::adc2>, 12);
	EXPECT_EQ(mcutl::adc::resolution_bits<mcutl::adc::adc3>, 12);
	
	EXPECT_EQ(mcutl::adc::max_result_value<mcutl::adc::adc1>, 0xfffu);
	EXPECT_EQ(mcutl::adc::max_result_value<mcutl::adc::adc2>, 0xfffu);
	EXPECT_EQ(mcutl::adc::max_result_value<mcutl::adc::adc3>, 0xfffu);
	
	EXPECT_TRUE((std::is_same_v<mcutl::adc::gpio_config, mcutl::gpio::in::analog>));
}

TEST_F(adc_strict_test_fixture_base, MappingsTest)
{
	test_gpio_mapping<mcutl::adc::adc1,
		10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 14, 15, 8, 9>
	(mcutl::types::list<
		mcutl::gpio::gpioc<0>,
		mcutl::gpio::gpioc<1>,
		mcutl::gpio::gpioc<2>,
		mcutl::gpio::gpioc<3>,
		mcutl::gpio::gpioa<0>,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<2>,
		mcutl::gpio::gpioa<3>,
		mcutl::gpio::gpioa<4>,
		mcutl::gpio::gpioa<5>,
		mcutl::gpio::gpioa<6>,
		mcutl::gpio::gpioa<7>,
		mcutl::gpio::gpioc<4>,
		mcutl::gpio::gpioc<5>,
		mcutl::gpio::gpiob<0>,
		mcutl::gpio::gpiob<1>
	>{});
	
	test_gpio_mapping<mcutl::adc::adc2,
		10, 11, 12, 13, 0, 1, 2, 3, 4, 5, 6, 7, 14, 15, 8, 9>
	(mcutl::types::list<
		mcutl::gpio::gpioc<0>,
		mcutl::gpio::gpioc<1>,
		mcutl::gpio::gpioc<2>,
		mcutl::gpio::gpioc<3>,
		mcutl::gpio::gpioa<0>,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<2>,
		mcutl::gpio::gpioa<3>,
		mcutl::gpio::gpioa<4>,
		mcutl::gpio::gpioa<5>,
		mcutl::gpio::gpioa<6>,
		mcutl::gpio::gpioa<7>,
		mcutl::gpio::gpioc<4>,
		mcutl::gpio::gpioc<5>,
		mcutl::gpio::gpiob<0>,
		mcutl::gpio::gpiob<1>
	>{});
	
	test_gpio_mapping<mcutl::adc::adc3,
		4, 5, 6, 7, 8, 10, 11, 12, 13, 0, 1, 2, 3>
	(mcutl::types::list<
		mcutl::gpio::gpiof<6>,
		mcutl::gpio::gpiof<7>,
		mcutl::gpio::gpiof<8>,
		mcutl::gpio::gpiof<9>,
		mcutl::gpio::gpiof<10>,
		mcutl::gpio::gpioc<0>,
		mcutl::gpio::gpioc<1>,
		mcutl::gpio::gpioc<2>,
		mcutl::gpio::gpioc<3>,
		mcutl::gpio::gpioa<0>,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<2>,
		mcutl::gpio::gpioa<3>
	>{});
}

using simple_clock_config = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal,
	mcutl::clock::adc<mcutl::clock::required_frequency<14'000'000>>>;

TEST_F(adc_strict_test_fixture_base, TimesTest)
{
	EXPECT_TRUE((std::is_same_v<mcutl::adc::initialization_time<mcutl::adc::adc1, simple_clock_config>,
		mcutl::types::microseconds<1>>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::initialization_time<mcutl::adc::adc2, simple_clock_config>,
		mcutl::types::microseconds<1>>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::initialization_time<mcutl::adc::adc3, simple_clock_config>,
		mcutl::types::microseconds<1>>));
	
	EXPECT_TRUE((std::is_same_v<mcutl::adc::calibration_time<mcutl::adc::adc1, simple_clock_config>,
		mcutl::types::microseconds<6>>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::calibration_time<mcutl::adc::adc2, simple_clock_config>,
		mcutl::types::microseconds<6>>));
	EXPECT_TRUE((std::is_same_v<mcutl::adc::calibration_time<mcutl::adc::adc3, simple_clock_config>,
		mcutl::types::microseconds<6>>));
}

using adc_list = ::testing::Types<mcutl::adc::adc1, mcutl::adc::adc2, mcutl::adc::adc3>;

template<typename Adc> using adc_list_test_fixture = adc_strict_test_fixture<Adc>;
TYPED_TEST_SUITE(adc_list_test_fixture, adc_list);

TYPED_TEST(adc_list_test_fixture, PendingFlagsTest)
{
	using adc = typename TestFixture::adc;
	EXPECT_EQ((mcutl::adc::pending_flags_v<adc,
		mcutl::adc::init::interrupt::conversion_complete>), ADC_SR_EOS);
	EXPECT_EQ(mcutl::adc::pending_flags_v<adc>, 0u);
}

TYPED_TEST(adc_list_test_fixture, GetPendingFlagsTest)
{
	using adc = typename TestFixture::adc;
	this->memory().allow_reads(this->addr(&this->get_adc()->SR));
	
	EXPECT_EQ((mcutl::adc::get_pending_flags<adc,
		mcutl::adc::init::interrupt::conversion_complete>()), 0u);
	EXPECT_EQ((mcutl::adc::get_pending_flags<adc>()), 0u);
	
	this->memory().set(this->addr(&this->get_adc()->SR), ADC_SR_EOS);
	
	EXPECT_EQ((mcutl::adc::get_pending_flags<adc,
		mcutl::adc::init::interrupt::conversion_complete>()), ADC_SR_EOS);
	EXPECT_EQ((mcutl::adc::get_pending_flags<adc>()), 0u);
}

TYPED_TEST(adc_list_test_fixture, ClearPendingFlagsTest)
{
	static constexpr uint32_t initial_sr = 0xffffffffu;
	
	using adc = typename TestFixture::adc;
	this->memory().set(this->addr(&this->get_adc()->SR), initial_sr);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SR),
		initial_sr & ~ADC_SR_EOS));
	
	mcutl::adc::clear_pending_flags<adc,
		mcutl::adc::init::interrupt::conversion_complete>();
	
	this->memory().set(this->addr(&this->get_adc()->SR), initial_sr);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SR),
		initial_sr & ~ADC_SR_EOS));
	
	mcutl::adc::clear_pending_flags_atomic<adc,
		mcutl::adc::init::interrupt::conversion_complete>();
}

TYPED_TEST(adc_list_test_fixture, CalibrationFinishedTest)
{
	using adc = typename TestFixture::adc;
	
	this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
	
	EXPECT_TRUE(mcutl::adc::is_calibration_finished<adc>());
	
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_CAL);
	
	EXPECT_FALSE(mcutl::adc::is_calibration_finished<adc>());
}

TYPED_TEST(adc_list_test_fixture, CalibrationAlreadyInProgressTest)
{
	using adc = typename TestFixture::adc;
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_CAL);
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	mcutl::adc::calibrate<adc>();
}

TYPED_TEST(adc_list_test_fixture, CalibrationAlreadyInProgressWaitTest)
{
	using adc = typename TestFixture::adc;
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_CAL);
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	this->expect_wait_calibration_winished();
	
	mcutl::adc::calibrate<adc, mcutl::adc::cal::wait_finished<72'000'000, simple_clock_config>>();
}

TYPED_TEST(adc_list_test_fixture, CalibrationTest)
{
	using adc = typename TestFixture::adc;
	constexpr uint32_t initial_cr2 = 0xffffffffu & ~ADC_CR2_CAL;
	
	this->memory().set(this->addr(&this->get_adc()->CR2), initial_cr2);
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
		initial_cr2 | ADC_CR2_CAL));
	
	mcutl::adc::calibrate<adc>();
}

TYPED_TEST(adc_list_test_fixture, CalibrationWaitTest)
{
	using adc = typename TestFixture::adc;
	constexpr uint32_t initial_cr2 = 0xffffffffu & ~ADC_CR2_CAL;
	
	this->memory().set(this->addr(&this->get_adc()->CR2), initial_cr2);
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
		initial_cr2 | ADC_CR2_CAL));
	this->expect_wait_calibration_winished();
	
	mcutl::adc::calibrate<adc, mcutl::adc::cal::wait_finished<72'000'000, simple_clock_config>>();
}

TYPED_TEST(adc_list_test_fixture, SingleConversionTest)
{
	using adc = typename TestFixture::adc;
	
	constexpr uint32_t initial_cr2 = 0xffffffffu & ~ADC_CR2_ADON;
	this->memory().set(this->addr(&this->get_adc()->CR2), initial_cr2);
	this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
	
	auto conv = mcutl::adc::prepare_conversion<adc, mcutl::adc::channel<3>>();
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
		initial_cr2 | ADC_CR2_ADON));
	conv.start();
	
	this->memory().allow_reads(this->addr(&this->get_adc()->DR));
	this->memory().set(this->addr(&this->get_adc()->DR), 12345u);
	EXPECT_EQ(conv.get_conversion_result(), 12345u);
	
	this->memory().allow_reads(this->addr(&this->get_adc()->SR));
	EXPECT_FALSE(conv.is_finished());
	this->memory().set(this->addr(&this->get_adc()->SR), ADC_SR_EOS);
	EXPECT_TRUE(conv.is_finished());
}

TYPED_TEST(adc_list_test_fixture, ScanConversionTest)
{
	using adc = typename TestFixture::adc;
	constexpr uint32_t initial_cr2 = 0xffffffffu & ~ADC_CR2_ADON;
	this->memory().set(this->addr(&this->get_adc()->CR2), initial_cr2);
	
	if constexpr (!std::is_same_v<adc, mcutl::adc::adc2>)
	{
		static constexpr uint32_t channel_count = 3;
		auto conv = mcutl::adc::prepare_conversion<adc, mcutl::adc::conv::scan_channels<
			mcutl::adc::dma_channel_config<mcutl::adc::dma_channel<adc>>,
			mcutl::adc::channel<1>,
			mcutl::adc::channel<4>,
			mcutl::adc::channel<7>>
		>();
		
		{
			::testing::Sequence s1, s2, s3;
			
			this->expect_start_transfer(this->get_dma_channel(), 0,
				&(this->get_adc()->DR),
				this->to_address, channel_count,
				s1, s2, s3);
			
			EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)))
				.InSequence(s1, s2, s3);
			EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
				initial_cr2 | ADC_CR2_ADON))
				.InSequence(s1, s2, s3);
			
			conv.start(static_cast<uint16_t*>(this->to_address));
		}
		
		this->memory().allow_reads(this->addr(&this->get_adc()->SR));
		EXPECT_FALSE(conv.is_finished());
		this->memory().set(this->addr(&this->get_adc()->SR), ADC_SR_EOS);
		EXPECT_TRUE(conv.is_finished());
	}
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest1)
{
	using adc = typename TestFixture::adc;
	
	::testing::InSequence s;
	this->expect_enable_peripheral();
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR1),
		(1u << ADC_SQR1_L_Pos)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR3),
		5u));
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR1),
		ADC_CR1_EOSIE));
	
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
		ADC_CR2_ADON | ADC_CR2_ALIGN));
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::enable_peripheral<true>,
		mcutl::adc::channel<5>,
		mcutl::adc::init::enable<true>,
		mcutl::adc::init::data_alignment::left,
		mcutl::adc::init::interrupt::conversion_complete,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest2)
{
	using adc = typename TestFixture::adc;
	
	this->memory().set(this->addr(&RCC->APB2ENR), this->get_adc_periph_bit());
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB2ENR)));
	EXPECT_CALL(this->memory(), write(this->addr(&RCC->APB2ENR), 0u));
	EXPECT_CALL(this->memory(), read(this->addr(&RCC->APB2ENR)));
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::enable_peripheral<false>,
		mcutl::adc::init::enable<false>,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest3)
{
	using adc = typename TestFixture::adc;
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SMPR2),
		0b011u << ADC_SMPR2_SMP7_Pos));
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::input_impedance<mcutl::adc::channel<7>, 25'000, simple_clock_config>,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest4)
{
	using adc = typename TestFixture::adc;
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SMPR1),
		0b101u << ADC_SMPR1_SMP10_Pos));
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::sample_time<mcutl::adc::channel<10>,
			mcutl::adc::init::sample_time_cycles::cycles_55_5>,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest5)
{
	using adc = typename TestFixture::adc;
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::wait_finished<72'000'000, simple_clock_config>,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest6)
{
	using adc = typename TestFixture::adc;
	
	if constexpr (!std::is_same_v<adc, mcutl::adc::adc2>)
	{
		using dma_config = mcutl::adc::dma_channel_config<mcutl::adc::dma_channel<adc>>;
		
		::testing::InSequence s;
		
		this->expect_configure(this->get_dma_channel(),
			DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_0,
			0, mcutl::interrupt::default_priority, 0,
			0x12345678u & ~DMA_CCR_EN);
		
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR1),
			0u << ADC_SQR1_SQ13_Pos
			| 12u << ADC_SQR1_SQ14_Pos
			| 14u << ADC_SQR1_L_Pos));
	
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR2),
			6u << ADC_SQR2_SQ7_Pos
			| 11u << ADC_SQR2_SQ8_Pos
			| 8u << ADC_SQR2_SQ9_Pos
			| 2u << ADC_SQR2_SQ10_Pos
			| 1u << ADC_SQR2_SQ11_Pos
			| 9u << ADC_SQR2_SQ12_Pos));
	
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR3),
			1u << ADC_SQR3_SQ1_Pos
			| 0u << ADC_SQR3_SQ2_Pos
			| 3u << ADC_SQR3_SQ3_Pos
			| 4u << ADC_SQR3_SQ4_Pos
			| 7u << ADC_SQR3_SQ5_Pos
			| 5u << ADC_SQR3_SQ6_Pos));
	
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR1),
			ADC_CR1_SCAN));
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
			ADC_CR2_DMA));
		
		mcutl::adc::configure<adc,
			mcutl::adc::init::scan_channels<dma_config,
				mcutl::adc::channel<1>,
				mcutl::adc::channel<0>,
				mcutl::adc::channel<3>,
				mcutl::adc::channel<4>,
				mcutl::adc::channel<7>,
				mcutl::adc::channel<5>,
		
				mcutl::adc::channel<6>,
				mcutl::adc::channel<11>,
				mcutl::adc::channel<8>,
				mcutl::adc::channel<2>,
				mcutl::adc::channel<1>,
				mcutl::adc::channel<9>,
		
				mcutl::adc::channel<0>,
				mcutl::adc::channel<12>>,
			mcutl::adc::init::base_configuration_is_currently_present
		>();
	}
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest7)
{
	using adc = typename TestFixture::adc;
	
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_ADON);
	this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::wait_finished<72'000'000, simple_clock_config>,
		mcutl::adc::init::enable<true>,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureBaseConfigPresentTest8)
{
	using adc = typename TestFixture::adc;
	
	this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
	this->setup_systick();
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2), ADC_CR2_ADON));
	this->expect_wait_init();
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::wait_finished<100'000'000, simple_clock_config>,
		mcutl::adc::init::enable<true>,
		mcutl::adc::init::base_configuration_is_currently_present
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureTest1)
{
	using adc = typename TestFixture::adc;
	
	::testing::InSequence s;
	this->expect_enable_peripheral();
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SMPR1), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SMPR2), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR1),
		(1u << ADC_SQR1_L_Pos)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR2), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR3), 11u));
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR1),
		ADC_CR1_EOSIE));
	
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
		ADC_CR2_ADON));
	
	this->expect_enable_interrupt(std::is_same_v<adc, mcutl::adc::adc3> ? ADC3_IRQn : ADC1_2_IRQn,
		3u << 1, 1u);
	
	mcutl::adc::configure<adc,
		mcutl::adc::init::enable_peripheral<true>,
		mcutl::adc::channel<11>,
		mcutl::adc::init::enable<true>,
		mcutl::adc::init::data_alignment::right,
		mcutl::interrupt::interrupt<mcutl::adc::init::interrupt::conversion_complete, 3, 1>,
		mcutl::adc::init::interrupt::enable_controller_interrupts,
		mcutl::interrupt::priority_count<8>
	>();
}

TYPED_TEST(adc_list_test_fixture, ConfigureTest2)
{
	using adc = typename TestFixture::adc;
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SMPR1), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SMPR2), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR1), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR2), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR3), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR1), 0u));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2), 0u));
	
	this->expect_disable_interrupt(std::is_same_v<adc, mcutl::adc::adc3> ? ADC3_IRQn : ADC1_2_IRQn);
	
	mcutl::adc::configure<adc,
		mcutl::interrupt::disabled<mcutl::adc::init::interrupt::conversion_complete>,
		mcutl::adc::init::interrupt::disable_controller_interrupts
	>();
}

TYPED_TEST(adc_list_test_fixture, ReConfigureTest1)
{
	using adc = typename TestFixture::adc;
	
	this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
	this->memory().set(this->addr(&this->get_adc()->CR2), 0xffffffffu & ~ADC_CR2_ADON);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2), 0xffffffffu));
	
	mcutl::adc::reconfigure<adc, mcutl::adc::init::enable<true>>();
	
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_ADON);
	mcutl::adc::reconfigure<adc, mcutl::adc::init::enable<true>>();
}

TYPED_TEST(adc_list_test_fixture, ReConfigureTest2)
{
	using adc = typename TestFixture::adc;
	
	this->memory().allow_reads(this->addr(&this->get_adc()->CR1));
	this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
	this->memory().set(this->addr(&this->get_adc()->CR1), ADC_CR1_SCAN | ADC_CR1_EOSIE);
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_DMA | ADC_CR2_ADON);
	this->memory().set(this->addr(&this->get_adc()->SQR1), 12u << ADC_SQR1_L_Pos);
	this->memory().set(this->addr(&this->get_adc()->SQR3), 10u << ADC_SQR3_SQ1_Pos);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR1), ADC_CR1_EOSIE));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2), ADC_CR2_ADON));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR1), 1u << ADC_SQR1_L_Pos));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR3), 2u << ADC_SQR3_SQ1_Pos));
	
	mcutl::adc::reconfigure<adc, mcutl::adc::channel<2>>();
}

TYPED_TEST(adc_list_test_fixture, ReConfigureTest3)
{
	using adc = typename TestFixture::adc;
	
	this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_DMA);
	this->setup_systick();
	
	::testing::InSequence s;
	EXPECT_CALL(this->memory(), read(this->addr(&this->get_adc()->CR2)));
	EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
		ADC_CR2_DMA | ADC_CR2_ADON));
	this->expect_wait_init();
	
	mcutl::adc::reconfigure<adc,
		mcutl::adc::init::enable<true>,
		mcutl::adc::init::wait_finished<100'000'000, simple_clock_config>>();
}

TYPED_TEST(adc_list_test_fixture, ReConfigureTest4)
{
	using adc = typename TestFixture::adc;
	
	this->memory().allow_reads(this->addr(&this->get_adc()->CR1));
	
	mcutl::adc::reconfigure<adc,
		mcutl::adc::init::interrupt::enable_controller_interrupts>();
	
	this->expect_enable_interrupt(
		std::is_same_v<adc, mcutl::adc::adc3> ? ADC3_IRQn : ADC1_2_IRQn);
	
	this->memory().set(this->addr(&this->get_adc()->CR1), ADC_CR1_EOSIE);
	
	mcutl::adc::reconfigure<adc,
		mcutl::adc::init::interrupt::enable_controller_interrupts>();
}

TYPED_TEST(adc_list_test_fixture, ReConfigureTest5)
{
	using adc = typename TestFixture::adc;
	
	this->memory().allow_reads(this->addr(&this->get_adc()->CR1));
	this->memory().set(this->addr(&this->get_adc()->CR1), ADC_CR1_EOSIE);
	
	mcutl::adc::reconfigure<adc,
		mcutl::adc::init::interrupt::disable_controller_interrupts>();
	
	this->memory().set(this->addr(&this->get_adc()->CR1), 0u);
	
	this->expect_disable_interrupt(
		std::is_same_v<adc, mcutl::adc::adc3> ? ADC3_IRQn : ADC1_2_IRQn);
	
	mcutl::adc::reconfigure<adc,
		mcutl::adc::init::interrupt::disable_controller_interrupts>();
}

TYPED_TEST(adc_list_test_fixture, ReConfigureTest6)
{
	using adc = typename TestFixture::adc;
	
	if constexpr (!std::is_same_v<adc, mcutl::adc::adc2>)
	{
		using dma_config = mcutl::adc::dma_channel_config<
			mcutl::adc::dma_channel<adc>,
			mcutl::dma::priority::high
		>;
		
		this->memory().allow_reads(this->addr(&this->get_adc()->CR1));
		this->memory().allow_reads(this->addr(&this->get_adc()->CR2));
		this->memory().set(this->addr(&this->get_adc()->CR1), ADC_CR1_EOSIE);
		this->memory().set(this->addr(&this->get_adc()->CR2), ADC_CR2_ADON);
		
		::testing::InSequence s;
		
		this->expect_configure(this->get_dma_channel(),
			DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_0 | DMA_CCR_PL_1,
			0, mcutl::interrupt::default_priority, 0,
			0x12345678u & ~DMA_CCR_EN);
		
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR1),
			3u << ADC_SQR1_L_Pos));
	
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->SQR3),
			5u << ADC_SQR3_SQ1_Pos
			| 7u << ADC_SQR3_SQ2_Pos
			| 11u << ADC_SQR3_SQ3_Pos));
	
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR1),
			ADC_CR1_EOSIE | ADC_CR1_SCAN));
		EXPECT_CALL(this->memory(), write(this->addr(&this->get_adc()->CR2),
			ADC_CR2_ADON | ADC_CR2_DMA));
		
		mcutl::adc::reconfigure<adc,
			mcutl::adc::init::scan_channels<dma_config,
				mcutl::adc::channel<5>,
				mcutl::adc::channel<7>,
				mcutl::adc::channel<11>>
		>();
	}
}
