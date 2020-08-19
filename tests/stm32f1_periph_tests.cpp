#define STM32F103xE
#define STM32F1

#include <type_traits>

#include "mcutl/periph/periph.h"
#include "mcutl/utils/type_helpers.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

enum class bus
{
	ahb,
	apb1,
	apb2
};

template<typename PeriphConfig, bus Bus, uint32_t EnableMask, uint32_t ResetMask = 0>
struct periph_test_data
{
	using periph_config = PeriphConfig;
	static constexpr auto enable_mask = EnableMask;
	static constexpr auto reset_mask = ResetMask;
	
	static void enable() noexcept
	{
		mcutl::periph::configure_peripheral<
			mcutl::periph::enable<periph_config>>();
	}
	
	static void disable() noexcept
	{
		mcutl::periph::configure_peripheral<
			mcutl::periph::disable<periph_config>>();
	}
	
	static void reset() noexcept
	{
		mcutl::periph::configure_peripheral<
			mcutl::periph::reset<periph_config>>();
	}
	
	static void undo_reset() noexcept
	{
		mcutl::periph::configure_peripheral<
			mcutl::periph::undo_reset<periph_config>>();
	}
	
	static mcutl::tests::memory::memory_address_t enr_addr() noexcept
	{
		if constexpr (Bus == bus::ahb)
			return mcutl::tests::memory::addr(&(RCC->AHBENR));
		else if constexpr (Bus == bus::apb1)
			return mcutl::tests::memory::addr(&(RCC->APB1ENR));
		else
			return mcutl::tests::memory::addr(&(RCC->APB2ENR));
	}
	
	static mcutl::tests::memory::memory_address_t rstr_addr() noexcept
	{
		if constexpr (Bus == bus::apb1)
			return mcutl::tests::memory::addr(&(RCC->APB1RSTR));
		else if (Bus == bus::apb2)
			return mcutl::tests::memory::addr(&(RCC->APB2RSTR));
		else
			return 0;
	}
};

template<typename PeriphTestData>
class periph_enable_disable_combine_test {};

template<typename... PeriphTestData>
class periph_enable_disable_combine_test<mcutl::types::list<PeriphTestData...>>
	: public mcutl::tests::memory::strict_test_fixture_base
{
public:
	void enable()
	{
		auto enr_addr = get_enr_addr<PeriphTestData...>();
		memory().set(enr_addr, 0x12345678 & ~mask);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(enr_addr));
		EXPECT_CALL(memory(), write(enr_addr, 0x12345678 | mask));
		EXPECT_CALL(memory(), read(enr_addr));
		
		mcutl::periph::configure_peripheral<
			mcutl::periph::enable<typename PeriphTestData::periph_config>...>();
	}
	
	void disable()
	{
		auto enr_addr = get_enr_addr<PeriphTestData...>();
		memory().set(enr_addr, 0x12345678 | mask);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(enr_addr));
		EXPECT_CALL(memory(), write(enr_addr, 0x12345678 & ~mask));
		EXPECT_CALL(memory(), read(enr_addr));
		
		mcutl::periph::configure_peripheral<
			mcutl::periph::disable<typename PeriphTestData::periph_config>...>();
	}
	
private:
	template<typename Periph, typename...>
	static auto get_enr_addr() noexcept
	{
		return Periph::enr_addr();
	}
	
private:
	static constexpr auto mask = (... | PeriphTestData::enable_mask);
};

template<typename PeriphTestData>
class periph_reset_combine_test {};

template<typename... PeriphTestData>
class periph_reset_combine_test<mcutl::types::list<PeriphTestData...>>
	: public mcutl::tests::memory::strict_test_fixture_base
{
public:
	void reset()
	{
		auto rstr_addr = get_rstr_addr<PeriphTestData...>();
		memory().set(rstr_addr, 0x12345678 & ~mask);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(rstr_addr));
		EXPECT_CALL(memory(), write(rstr_addr, 0x12345678 | mask));
		
		mcutl::periph::configure_peripheral<
			mcutl::periph::reset<typename PeriphTestData::periph_config>...>();
	}
	
	void undo_reset()
	{
		auto rstr_addr = get_rstr_addr<PeriphTestData...>();
		memory().set(rstr_addr, 0x12345678 | mask);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(rstr_addr));
		EXPECT_CALL(memory(), write(rstr_addr, 0x12345678 & ~mask));
		
		mcutl::periph::configure_peripheral<
			mcutl::periph::undo_reset<typename PeriphTestData::periph_config>...>();
	}
	
private:
	template<typename Periph, typename...>
	static auto get_rstr_addr() noexcept
	{
		return Periph::rstr_addr();
	}
	
private:
	static constexpr auto mask = (... | PeriphTestData::reset_mask);
};

template<typename PeriphTestData>
class periph_reset_undo_reset_test
	: public periph_reset_combine_test<mcutl::types::list<PeriphTestData>>
{
};

template<typename PeriphTestData>
class periph_enable_disable_test
	: public periph_enable_disable_combine_test<mcutl::types::list<PeriphTestData>>
{
};

using namespace mcutl::periph;

using ahb_peripherals = mcutl::types::pop_front_t<mcutl::types::list<
	void
#ifdef RCC_AHBENR_SDIOEN
	, periph_test_data<sdio, bus::ahb, RCC_AHBENR_SDIOEN>
#endif //RCC_AHBENR_SDIOEN
#ifdef RCC_AHBENR_FSMCEN
	, periph_test_data<fsmc, bus::ahb, RCC_AHBENR_FSMCEN>
#endif //RCC_AHBENR_FSMCEN
#ifdef RCC_AHBENR_CRCEN
	, periph_test_data<crc, bus::ahb, RCC_AHBENR_CRCEN>
#endif //RCC_AHBENR_CRCEN
#ifdef RCC_AHBENR_SRAMEN
	, periph_test_data<sram, bus::ahb, RCC_AHBENR_SRAMEN>
#endif //RCC_AHBENR_SRAMEN
#ifdef RCC_AHBENR_DMA2EN
	, periph_test_data<dma2, bus::ahb, RCC_AHBENR_DMA2EN>
#endif //RCC_AHBENR_DMA2EN
#ifdef RCC_AHBENR_DMA1EN
	, periph_test_data<dma1, bus::ahb, RCC_AHBENR_DMA1EN>
#endif //RCC_AHBENR_DMA1EN
#ifdef RCC_AHBENR_ETHMACRXEN
	, periph_test_data<ethmacrx, bus::ahb, RCC_AHBENR_ETHMACRXEN>
#endif //RCC_AHBENR_ETHMACRXEN
#ifdef RCC_AHBENR_ETHMACTXEN
	, periph_test_data<ethmactx, bus::ahb, RCC_AHBENR_ETHMACTXEN>
#endif //RCC_AHBENR_ETHMACTXEN
#ifdef RCC_AHBENR_ETHMACEN
	, periph_test_data<ethmac, bus::ahb, RCC_AHBENR_ETHMACEN>
#endif //RCC_AHBENR_ETHMACEN
#ifdef RCC_AHBENR_OTGFSEN
	, periph_test_data<otgfs, bus::ahb, RCC_AHBENR_OTGFSEN>
#endif //RCC_AHBENR_OTGFSEN
#ifdef RCC_AHBENR_FLITFEN
	, periph_test_data<flitf, bus::ahb, RCC_AHBENR_FLITFEN>
#endif //RCC_AHBENR_FLITFEN
>>;

using apb1_peripherals = mcutl::types::pop_front_t<mcutl::types::list<
	void
#ifdef RCC_APB1ENR_DACEN
	, periph_test_data<dac, bus::apb1, RCC_APB1ENR_DACEN, RCC_APB1RSTR_DACRST>
#endif //RCC_APB1ENR_DACEN
#ifdef RCC_APB1ENR_PWREN
	, periph_test_data<pwr, bus::apb1, RCC_APB1ENR_PWREN, RCC_APB1RSTR_PWRRST>
#endif //RCC_APB1ENR_PWREN
#ifdef RCC_APB1ENR_BKPEN
	, periph_test_data<bkp, bus::apb1, RCC_APB1ENR_BKPEN, RCC_APB1RSTR_BKPRST>
#endif //RCC_APB1ENR_BKPEN
#ifdef RCC_APB1ENR_CAN1EN
	, periph_test_data<can1, bus::apb1, RCC_APB1ENR_CAN1EN, RCC_APB1RSTR_CAN1RST>
#endif //RCC_APB1ENR_CAN1EN
#ifdef RCC_APB1ENR_CAN2EN
	, periph_test_data<can2, bus::apb1, RCC_APB1ENR_CAN2EN, RCC_APB1RSTR_CAN2RST>
#endif //RCC_APB1ENR_CAN2EN
#ifdef RCC_APB1ENR_USBEN
	, periph_test_data<usb, bus::apb1, RCC_APB1ENR_USBEN, RCC_APB1RSTR_USBRST>
#endif //RCC_APB1ENR_USBEN
#ifdef RCC_APB1ENR_I2C1EN
	, periph_test_data<i2c1, bus::apb1, RCC_APB1ENR_I2C1EN, RCC_APB1RSTR_I2C1RST>
#endif //RCC_APB1ENR_I2C1EN
#ifdef RCC_APB1ENR_I2C2EN
	, periph_test_data<i2c2, bus::apb1, RCC_APB1ENR_I2C2EN, RCC_APB1RSTR_I2C2RST>
#endif //RCC_APB1ENR_I2C2EN
#ifdef RCC_APB1ENR_UART5EN
	, periph_test_data<uart5, bus::apb1, RCC_APB1ENR_UART5EN, RCC_APB1RSTR_UART5RST>
#endif //RCC_APB1ENR_UART5EN
#ifdef RCC_APB1ENR_UART4EN
	, periph_test_data<uart4, bus::apb1, RCC_APB1ENR_UART4EN, RCC_APB1RSTR_UART4RST>
#endif //RCC_APB1ENR_UART4EN
#ifdef RCC_APB1ENR_USART3EN
	, periph_test_data<usart3, bus::apb1, RCC_APB1ENR_USART3EN, RCC_APB1RSTR_USART3RST>
#endif //RCC_APB1ENR_USART3EN
#ifdef RCC_APB1ENR_USART2EN
	, periph_test_data<usart2, bus::apb1, RCC_APB1ENR_USART2EN, RCC_APB1RSTR_USART2RST>
#endif //RCC_APB1ENR_USART2EN
#ifdef RCC_APB1ENR_SPI3EN
	, periph_test_data<spi3, bus::apb1, RCC_APB1ENR_SPI3EN, RCC_APB1RSTR_SPI3RST>
#endif //RCC_APB1ENR_SPI3EN
#ifdef RCC_APB1ENR_SPI2EN
	, periph_test_data<spi2, bus::apb1, RCC_APB1ENR_SPI2EN, RCC_APB1RSTR_SPI2RST>
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_WWDGEN
	, periph_test_data<wwdg, bus::apb1, RCC_APB1ENR_WWDGEN, RCC_APB1RSTR_WWDGRST>
#endif //RCC_APB1ENR_WWDGEN
#ifdef RCC_APB1ENR_TIM14EN
	, periph_test_data<timer14, bus::apb1, RCC_APB1ENR_TIM14EN, RCC_APB1RSTR_TIM14RST>
#endif //RCC_APB1ENR_TIM14EN
#ifdef RCC_APB1ENR_TIM13EN
	, periph_test_data<timer13, bus::apb1, RCC_APB1ENR_TIM13EN, RCC_APB1RSTR_TIM13RST>
#endif //RCC_APB1ENR_TIM13EN
#ifdef RCC_APB1ENR_TIM12EN
	, periph_test_data<timer12, bus::apb1, RCC_APB1ENR_TIM12EN, RCC_APB1RSTR_TIM12RST>
#endif //RCC_APB1ENR_TIM12EN
#ifdef RCC_APB1ENR_TIM7EN
	, periph_test_data<timer7, bus::apb1, RCC_APB1ENR_TIM7EN, RCC_APB1RSTR_TIM7RST>
#endif //RCC_APB1ENR_TIM7EN
#ifdef RCC_APB1ENR_TIM6EN
	, periph_test_data<timer6, bus::apb1, RCC_APB1ENR_TIM6EN, RCC_APB1RSTR_TIM6RST>
#endif //RCC_APB1ENR_TIM6EN
#ifdef RCC_APB1ENR_TIM5EN
	, periph_test_data<timer5, bus::apb1, RCC_APB1ENR_TIM5EN, RCC_APB1RSTR_TIM5RST>
#endif //RCC_APB1ENR_TIM5EN
#ifdef RCC_APB1ENR_TIM4EN
	, periph_test_data<timer4, bus::apb1, RCC_APB1ENR_TIM4EN, RCC_APB1RSTR_TIM4RST>
#endif //RCC_APB1ENR_TIM4EN
#ifdef RCC_APB1ENR_TIM3EN
	, periph_test_data<timer3, bus::apb1, RCC_APB1ENR_TIM3EN, RCC_APB1RSTR_TIM3RST>
#endif //RCC_APB1ENR_TIM3EN
#ifdef RCC_APB1ENR_TIM2EN
	, periph_test_data<timer2, bus::apb1, RCC_APB1ENR_TIM2EN, RCC_APB1RSTR_TIM2RST>
#endif //RCC_APB1ENR_TIM2EN
>>;

using apb2_peripherals = mcutl::types::pop_front_t<mcutl::types::list<
	void
#ifdef RCC_APB2ENR_TIM11EN
	, periph_test_data<timer11, bus::apb2, RCC_APB2ENR_TIM11EN, RCC_APB2RSTR_TIM11RST>
#endif //RCC_APB2ENR_TIM11EN
#ifdef RCC_APB2ENR_TIM10EN
	, periph_test_data<timer10, bus::apb2, RCC_APB2ENR_TIM10EN, RCC_APB2RSTR_TIM10RST>
#endif //RCC_APB2ENR_TIM10EN
#ifdef RCC_APB2ENR_TIM9EN
	, periph_test_data<timer9, bus::apb2, RCC_APB2ENR_TIM9EN, RCC_APB2RSTR_TIM9RST>
#endif //RCC_APB2ENR_TIM9EN
#ifdef RCC_APB2ENR_ADC3EN
	, periph_test_data<adc3, bus::apb2, RCC_APB2ENR_ADC3EN, RCC_APB2RSTR_ADC3RST>
#endif //RCC_APB2ENR_ADC3EN
#ifdef RCC_APB2ENR_USART1EN
	, periph_test_data<usart1, bus::apb2, RCC_APB2ENR_USART1EN, RCC_APB2RSTR_USART1RST>
#endif //RCC_APB2ENR_USART1EN
#ifdef RCC_APB2ENR_TIM8EN
	, periph_test_data<timer8, bus::apb2, RCC_APB2ENR_TIM8EN, RCC_APB2RSTR_TIM8RST>
#endif //RCC_APB2ENR_TIM8EN
#ifdef RCC_APB2ENR_SPI1EN
	, periph_test_data<spi1, bus::apb2, RCC_APB2ENR_SPI1EN, RCC_APB2RSTR_SPI1RST>
#endif //RCC_APB2ENR_SPI1EN
#ifdef RCC_APB2ENR_TIM1EN
	, periph_test_data<timer1, bus::apb2, RCC_APB2ENR_TIM1EN, RCC_APB2RSTR_TIM1RST>
#endif //RCC_APB2ENR_TIM1EN
#ifdef RCC_APB2ENR_ADC2EN
	, periph_test_data<adc2, bus::apb2, RCC_APB2ENR_ADC2EN, RCC_APB2RSTR_ADC2RST>
#endif //RCC_APB2ENR_ADC2EN
#ifdef RCC_APB2ENR_ADC1EN
	, periph_test_data<adc1, bus::apb2, RCC_APB2ENR_ADC1EN, RCC_APB2RSTR_ADC1RST>
#endif //RCC_APB2ENR_ADC1EN
#ifdef RCC_APB2ENR_IOPAEN
	, periph_test_data<gpioa, bus::apb2, RCC_APB2ENR_IOPAEN, RCC_APB2RSTR_IOPARST>
#endif //RCC_APB2ENR_IOPAEN
#ifdef RCC_APB2ENR_IOPBEN
	, periph_test_data<gpiob, bus::apb2, RCC_APB2ENR_IOPBEN, RCC_APB2RSTR_IOPBRST>
#endif //RCC_APB2ENR_IOPBEN
#ifdef RCC_APB2ENR_IOPCEN
	, periph_test_data<gpioc, bus::apb2, RCC_APB2ENR_IOPCEN, RCC_APB2RSTR_IOPCRST>
#endif //RCC_APB2ENR_IOPCEN
#ifdef RCC_APB2ENR_IOPDEN
	, periph_test_data<gpiod, bus::apb2, RCC_APB2ENR_IOPDEN, RCC_APB2RSTR_IOPDRST>
#endif //RCC_APB2ENR_IOPDEN
#ifdef RCC_APB2ENR_IOPEEN
	, periph_test_data<gpioe, bus::apb2, RCC_APB2ENR_IOPEEN, RCC_APB2RSTR_IOPERST>
#endif //RCC_APB2ENR_IOPEEN
#ifdef RCC_APB2ENR_IOPFEN
	, periph_test_data<gpiof, bus::apb2, RCC_APB2ENR_IOPFEN, RCC_APB2RSTR_IOPFRST>
#endif //RCC_APB2ENR_IOPFEN
#ifdef RCC_APB2ENR_IOPGEN
	, periph_test_data<gpiog, bus::apb2, RCC_APB2ENR_IOPGEN, RCC_APB2RSTR_IOPGRST>
#endif //RCC_APB2ENR_IOPGEN
#ifdef RCC_APB2ENR_AFIOEN
	, periph_test_data<afio, bus::apb2, RCC_APB2ENR_AFIOEN, RCC_APB2RSTR_AFIORST>
#endif //RCC_APB2ENR_AFIOEN
>>;

using enable_disable_test_peripherals_type_list = mcutl::types::merge_containers_t<
	mcutl::types::list, ahb_peripherals, apb1_peripherals, apb2_peripherals>;
using enable_disable_test_peripherals = mcutl::types::merge_containers_t<
	::testing::Types, enable_disable_test_peripherals_type_list>;
using reset_test_peripherals_type_list = mcutl::types::merge_containers_t<
	::testing::Types, apb1_peripherals, apb2_peripherals>;
TYPED_TEST_SUITE(periph_enable_disable_test, enable_disable_test_peripherals);
TYPED_TEST_SUITE(periph_reset_undo_reset_test, reset_test_peripherals_type_list);

using periph_enable_disable_combine_test_peripherals = ::testing::Types<
	ahb_peripherals, apb1_peripherals, apb2_peripherals>;
TYPED_TEST_SUITE(periph_enable_disable_combine_test, periph_enable_disable_combine_test_peripherals);

using periph_reset_combine_test_peripherals = ::testing::Types<apb1_peripherals, apb2_peripherals>;
TYPED_TEST_SUITE(periph_reset_combine_test, periph_reset_combine_test_peripherals);

TYPED_TEST(periph_enable_disable_test, PeriphEnableDisableTest)
{
	this->enable();
	this->disable();
}

TYPED_TEST(periph_reset_undo_reset_test, PeriphResetUndoResetTest)
{
	this->reset();
	this->undo_reset();
}

TYPED_TEST(periph_enable_disable_combine_test, PeriphEnableDisableCombineTest)
{
	this->enable();
	this->disable();
}

TYPED_TEST(periph_reset_combine_test, PeriphResetCombineTest)
{
	this->reset();
	this->undo_reset();
}

class periph_strict_test_fixture
	: public mcutl::tests::memory::strict_test_fixture_base
{
};

TEST_F(periph_strict_test_fixture, ListPeriphTest)
{
	auto enr_addr = addr(&(RCC->APB1ENR));
	auto mask = RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN | RCC_APB1ENR_WWDGEN;
	memory().set(enr_addr, 0x12345678 & ~mask);
		
	::testing::InSequence s;
	EXPECT_CALL(memory(), read(enr_addr));
	EXPECT_CALL(memory(), write(enr_addr, 0x12345678 | mask));
	EXPECT_CALL(memory(), read(enr_addr));
		
	mcutl::periph::configure_peripheral<
		mcutl::periph::enable<mcutl::types::list<
			mcutl::periph::pwr,
			mcutl::periph::bkp,
			mcutl::periph::wwdg
		>>,
		mcutl::periph::enable<mcutl::periph::bkp>
	>();
}
