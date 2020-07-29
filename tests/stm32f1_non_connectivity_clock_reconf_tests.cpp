#define STM32F103xE
#define STM32F1

#include <type_traits>

#include "mcutl/clock/clock.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tests/stm32f1_non_connectivity_clock_test_fixtures.h"

using namespace ::testing;
using namespace mcutl::clock::literals;


template<typename Source, typename Target>
struct reconfigure_info
{
	using original_config_t = Source;
	using target_config_t = Target;
};

template<typename ClockConfigs>
class reconfigure_test_fixture : public clock_test_fixture
{
public:
	using original_config_t = typename ClockConfigs::original_config_t;
	using target_config_t = typename ClockConfigs::target_config_t;
	
public:
	virtual void SetUp() override
	{
		clock_test_fixture::SetUp();
	}
	
	static void reconfigure_clocks() noexcept
	{
		mcutl::clock::reconfigure_clocks<original_config_t, target_config_t>();
	}
};

template<typename ClockConfig>
class reconfigure_to_same_test_fixture : public clock_test_fixture
{
public:
	virtual void SetUp() override
	{
		mcutl::tests::memory::strict_test_fixture_base::SetUp();
	}
	
	static void reconfigure_clocks() noexcept
	{
		mcutl::clock::reconfigure_clocks<ClockConfig, ClockConfig>();
	}
};

using internal_oscillator_config = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal
>;

using external_oscillator_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>
>;

using internal_to_external_configurations = Types<
	reconfigure_info<internal_oscillator_config,
		external_oscillator_config>,
	reconfigure_info<internal_oscillator_config,
		mcutl::clock::overridden_options_t<internal_oscillator_config, external_oscillator_config>>
>;
template<typename ClockConfig>
using internal_to_external_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(internal_to_external_test_fixture, internal_to_external_configurations);

TYPED_TEST(internal_to_external_test_fixture, SwitchInternalToExternalOscillator)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL16
		| RCC_CFGR_SW_HSI | RCC_CFGR_SWS_HSI);
	
	InSequence seq;
	
	this->expect_set_hsi_as_system_and_wait(false);
	this->expect_pll_off_and_wait_ready();
	this->expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_set_pll_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->reconfigure_clocks();
}

using external_to_internal_configurations = Types<
	reconfigure_info<external_oscillator_config,
		internal_oscillator_config>,
	reconfigure_info<external_oscillator_config,
		mcutl::clock::overridden_options_t<external_oscillator_config, internal_oscillator_config>>
>;
template<typename ClockConfig>
using external_to_internal_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_to_internal_test_fixture, external_to_internal_configurations);

TYPED_TEST(external_to_internal_test_fixture, SwitchExternalToInternalOscillator)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	this->expect_hsi_on_and_wait_ready();
	this->expect_set_hsi_as_system_and_wait();
	this->expect_pll_off_and_wait_ready();
	this->expect_hse_off_and_wait_ready(false);
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL16;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_set_pll_as_system_and_wait();
	
	this->reconfigure_clocks();
}


using internal_oscillator_config_with_usb = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal,
	mcutl::clock::provide_usb_frequency
>;

using external_oscillator_config_with_usb = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::provide_usb_frequency
>;

using external_oscillator_with_usb_diff = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>
>;
using internal_oscillator_with_usb_diff = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal
>;

using internal_to_external_with_usb_configurations = Types<
	reconfigure_info<internal_oscillator_config_with_usb,
		external_oscillator_config_with_usb>,
	reconfigure_info<internal_oscillator_config_with_usb,
		mcutl::clock::overridden_options_t<internal_oscillator_config_with_usb,
			external_oscillator_with_usb_diff>>
>;
template<typename ClockConfig>
using internal_to_external_with_usb_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(internal_to_external_with_usb_test_fixture, internal_to_external_with_usb_configurations);

TYPED_TEST(internal_to_external_with_usb_test_fixture, SwitchInternalToExternalOscillatorWithUsb)
{
	this->memory().allow_reads(this->addr(&RCC->APB1ENR));
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV4 | RCC_CFGR_PLLMULL12
		| RCC_CFGR_USBPRE | RCC_CFGR_SW_HSI | RCC_CFGR_SWS_HSI);
	
	InSequence seq;
	
	this->expect_usb_disable();
	this->expect_set_hsi_as_system_and_wait(false);
	this->expect_pll_off_and_wait_ready();
	this->expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	this->expect_set_pll_as_system_and_wait();
	this->expect_usb_enable();
	this->expect_disable_hsi();
	
	this->reconfigure_clocks();
}

using external_to_internal_with_usb_configurations = Types<
	reconfigure_info<external_oscillator_config_with_usb,
		internal_oscillator_config_with_usb>,
	reconfigure_info<external_oscillator_config_with_usb,
		mcutl::clock::overridden_options_t<external_oscillator_config_with_usb,
			internal_oscillator_with_usb_diff>
	>
>;
template<typename ClockConfig>
using external_to_internal_with_usb_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_to_internal_with_usb_test_fixture, external_to_internal_with_usb_configurations);

TYPED_TEST(external_to_internal_with_usb_test_fixture, SwitchExternalToInternalOscillatorWithUsb)
{
	this->memory().allow_reads(this->addr(&RCC->APB1ENR));
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	this->expect_usb_disable();
	this->expect_hsi_on_and_wait_ready();
	this->expect_set_hsi_as_system_and_wait();
	this->expect_pll_off_and_wait_ready();
	this->expect_hse_off_and_wait_ready(false);
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV4
		| RCC_CFGR_PLLMULL12 | RCC_CFGR_USBPRE;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_0);
	this->expect_set_pll_as_system_and_wait();
	this->expect_usb_enable();
	
	this->reconfigure_clocks();
}


using external_low_speed_oscillator_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<4_MHz>
>;

using external_low_speed_oscillator_no_pll_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<4_MHz>,
	mcutl::clock::force_skip_pll
>;

using external_low_speed_oscillator_no_pll_diff = mcutl::clock::config<
	mcutl::clock::force_skip_pll
>;
using external_low_speed_oscillator_with_pll_diff = mcutl::clock::config<
	mcutl::clock::force_use_pll
>;

using external_with_pll_to_no_pll_configurations = Types<
	reconfigure_info<external_low_speed_oscillator_config,
		external_low_speed_oscillator_no_pll_config>,
	reconfigure_info<external_low_speed_oscillator_config,
		mcutl::clock::overridden_options_t<external_low_speed_oscillator_config,
			external_low_speed_oscillator_no_pll_diff>
	>
>;
template<typename ClockConfig>
using external_with_pll_to_no_pll_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_with_pll_to_no_pll_test_fixture, external_with_pll_to_no_pll_configurations);

TYPED_TEST(external_with_pll_to_no_pll_test_fixture, TurnOffPllWithExternalOscillator)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL16 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	this->expect_hsi_on_and_wait_ready();
	this->expect_set_hsi_as_system_and_wait();
	this->expect_pll_off_and_wait_ready();
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_ADCPRE_DIV2;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_flash_latency_change(0);
	this->expect_set_hse_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->reconfigure_clocks();
}

using external_no_pll_to_pll_configurations = Types<
	reconfigure_info<external_low_speed_oscillator_no_pll_config,
		external_low_speed_oscillator_config>,
	reconfigure_info<external_low_speed_oscillator_no_pll_config,
		mcutl::clock::overridden_options_t<external_low_speed_oscillator_no_pll_config,
			external_low_speed_oscillator_with_pll_diff>
	>
>;
template<typename ClockConfig>
using external_no_pll_to_pll_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_no_pll_to_pll_test_fixture, external_no_pll_to_pll_configurations);

TYPED_TEST(external_no_pll_to_pll_test_fixture, TurnOnPllWithExternalOscillator)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_ADCPRE_DIV2
		| RCC_CFGR_SW_HSE | RCC_CFGR_SWS_HSE);
	
	InSequence seq;
	
	this->expect_hsi_on_and_wait_ready();
	this->expect_set_hsi_as_system_and_wait();
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL16 | RCC_CFGR_PLLSRC;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	this->expect_set_pll_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->reconfigure_clocks();
}

template<template<typename...> typename Spi>
using spi_config_original = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::apb1<mcutl::clock::required_frequency<36_MHz>>,
	mcutl::clock::apb2<mcutl::clock::required_frequency<36_MHz>>,
	Spi<mcutl::clock::min_frequency<100_KHz>, mcutl::clock::max_frequency<200_KHz>>
>;

template<template<typename...> typename Spi>
using spi_config_modified = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::apb1<mcutl::clock::required_frequency<36_MHz>>,
	mcutl::clock::apb2<mcutl::clock::required_frequency<36_MHz>>,
	Spi<mcutl::clock::min_frequency<400_KHz>, mcutl::clock::max_frequency<800_KHz>>
>;

template<template<typename...> typename Spi>
using spi_config_modified_changes = mcutl::clock::config<
	Spi<mcutl::clock::min_frequency<400_KHz>, mcutl::clock::max_frequency<800_KHz>>
>;

using spi_configurations = Types<
	reconfigure_info<spi_config_original<mcutl::clock::spi1>,
		spi_config_modified<mcutl::clock::spi1>>,
	reconfigure_info<spi_config_original<mcutl::clock::spi1>,
		mcutl::clock::overridden_options_t<spi_config_original<mcutl::clock::spi1>,
			spi_config_modified_changes<mcutl::clock::spi1>>
	>
#ifdef RCC_APB1ENR_SPI2EN
	, reconfigure_info<spi_config_original<mcutl::clock::spi2>,
		spi_config_modified<mcutl::clock::spi2>>
	, reconfigure_info<spi_config_original<mcutl::clock::spi2>,
		mcutl::clock::overridden_options_t<spi_config_original<mcutl::clock::spi2>,
			spi_config_modified_changes<mcutl::clock::spi2>>
	>
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	, reconfigure_info<spi_config_original<mcutl::clock::spi3>,
		spi_config_modified<mcutl::clock::spi3>>
	, reconfigure_info<spi_config_original<mcutl::clock::spi3>,
		mcutl::clock::overridden_options_t<spi_config_original<mcutl::clock::spi3>,
			spi_config_modified_changes<mcutl::clock::spi3>>
	>
#endif //RCC_APB1ENR_SPI3EN
>;
template<typename ClockConfig>
using reconfigure_spi_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(reconfigure_spi_test_fixture, spi_configurations);

TYPED_TEST(reconfigure_spi_test_fixture, SpiOnly)
{
	mcutl::tests::memory::memory_address_t spi_cr{};
	
	if constexpr (std::is_same_v<typename TestFixture::original_config_t,
		spi_config_original<mcutl::clock::spi1>>)
	{
		spi_cr = this->addr(&SPI1->CR1);
	}
#ifdef RCC_APB1ENR_SPI2EN
	else if constexpr (std::is_same_v<typename TestFixture::original_config_t,
		spi_config_original<mcutl::clock::spi2>>)
	{
		spi_cr = this->addr(&SPI2->CR1);
	}
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	else if constexpr (std::is_same_v<typename TestFixture::original_config_t,
		spi_config_original<mcutl::clock::spi3>>)
	{
		spi_cr = this->addr(&SPI3->CR1);
	}
#endif //RCC_APB1ENR_SPI3EN
	
	this->memory().allow_reads(spi_cr);
	
	this->expect_spi_prescaler_change(spi_cr, SPI_CR1_BR_2 | SPI_CR1_BR_0);
	
	this->reconfigure_clocks();
}


using external_crystal_with_fast_core_req_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<12_MHz>,
	mcutl::clock::core<mcutl::clock::required_frequency<72_MHz>>
>;

using external_crystal_with_slow_core_req_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<12_MHz>,
	mcutl::clock::core<mcutl::clock::required_frequency<48_MHz>>
>;

using external_crystal_with_slow_core_req_diff = mcutl::clock::config<
	mcutl::clock::core<mcutl::clock::required_frequency<48_MHz>>
>;
using external_crystal_with_fast_core_req_diff = mcutl::clock::config<
	mcutl::clock::core<mcutl::clock::required_frequency<72_MHz>>
>;

using external_crystal_fast_to_slow_core_configurations = Types<
	reconfigure_info<external_crystal_with_fast_core_req_config,
		external_crystal_with_slow_core_req_config>,
	reconfigure_info<external_crystal_with_fast_core_req_config,
		mcutl::clock::overridden_options_t<external_crystal_with_fast_core_req_config,
			external_crystal_with_slow_core_req_diff>
	>
>;
template<typename ClockConfig>
using external_crystal_fast_to_slow_core_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_crystal_fast_to_slow_core_test_fixture, external_crystal_fast_to_slow_core_configurations);

TYPED_TEST(external_crystal_fast_to_slow_core_test_fixture, ExternalCrystalFastToSlowCore)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL6 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	this->expect_hsi_on_and_wait_ready();
	this->expect_set_hsi_as_system_and_wait();
	this->expect_pll_off_and_wait_ready();
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV4
		| RCC_CFGR_PLLMULL4 | RCC_CFGR_PLLSRC;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_0);
	this->expect_set_pll_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->reconfigure_clocks();
}

using external_crystal_slow_to_fast_core_configurations = Types<
	reconfigure_info<external_crystal_with_slow_core_req_config,
		external_crystal_with_fast_core_req_config>,
	reconfigure_info<external_crystal_with_slow_core_req_config,
		mcutl::clock::overridden_options_t<external_crystal_with_slow_core_req_config,
			external_crystal_with_fast_core_req_diff>
	>
>;
template<typename ClockConfig>
using external_crystal_slow_to_fast_core_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_crystal_slow_to_fast_core_test_fixture, external_crystal_slow_to_fast_core_configurations);

TYPED_TEST(external_crystal_slow_to_fast_core_test_fixture, ExternalCrystalSlowToFastCore)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV4
		| RCC_CFGR_PLLMULL4 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	this->expect_hsi_on_and_wait_ready();
	this->expect_set_hsi_as_system_and_wait();
	this->expect_pll_off_and_wait_ready();
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL6 | RCC_CFGR_PLLSRC;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	this->expect_set_pll_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->reconfigure_clocks();
}


using external_crystal_with_fast_apb2_req_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<12_MHz>,
	mcutl::clock::apb2<mcutl::clock::required_frequency<72_MHz>>
>;

using external_crystal_with_slow_apb2_req_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<12_MHz>,
	mcutl::clock::apb2<mcutl::clock::required_frequency<18_MHz>>
>;

using external_crystal_with_slow_apb2_req_diff = mcutl::clock::config<
	mcutl::clock::apb2<mcutl::clock::required_frequency<18_MHz>>
>;
using external_crystal_with_fast_apb2_req_diff = mcutl::clock::config<
	mcutl::clock::apb2<mcutl::clock::required_frequency<72_MHz>>
>;

using external_crystal_fast_to_slow_apb2_configurations = Types<
	reconfigure_info<external_crystal_with_fast_apb2_req_config,
		external_crystal_with_slow_apb2_req_config>,
	reconfigure_info<external_crystal_with_fast_apb2_req_config,
		mcutl::clock::overridden_options_t<external_crystal_with_fast_apb2_req_config,
			external_crystal_with_slow_apb2_req_diff>
	>
>;
template<typename ClockConfig>
using external_crystal_fast_to_slow_apb2_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_crystal_fast_to_slow_apb2_test_fixture, external_crystal_fast_to_slow_apb2_configurations);

TYPED_TEST(external_crystal_fast_to_slow_apb2_test_fixture, ExternalCrystalFastToSlowApb2)
{
	this->memory().set(this->addr(&RCC->CFGR),
		RCC_CFGR_PPRE2_DIV1
		| RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL6 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	constexpr auto cfgr_values = 
		RCC_CFGR_PPRE2_DIV4
		| RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV2
		| RCC_CFGR_PLLMULL6 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	
	this->reconfigure_clocks();
}

using external_crystal_slow_to_fast_apb2_configurations = Types<
	reconfigure_info<external_crystal_with_slow_apb2_req_config,
		external_crystal_with_fast_apb2_req_config>,
	reconfigure_info<external_crystal_with_slow_apb2_req_config,
		mcutl::clock::overridden_options_t<external_crystal_with_slow_apb2_req_config,
			external_crystal_with_fast_apb2_req_diff>
	>
>;
template<typename ClockConfig>
using external_crystal_slow_to_fast_apb2_test_fixture = reconfigure_test_fixture<ClockConfig>;
TYPED_TEST_SUITE(external_crystal_slow_to_fast_apb2_test_fixture, external_crystal_slow_to_fast_apb2_configurations);

TYPED_TEST(external_crystal_slow_to_fast_apb2_test_fixture, ExternalCrystalSlowToFastApb2)
{
	this->memory().set(this->addr(&RCC->CFGR), 
		RCC_CFGR_PPRE2_DIV4
		| RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV2
		| RCC_CFGR_PLLMULL6 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL);
	
	InSequence seq;
	
	constexpr auto cfgr_values =
		RCC_CFGR_PPRE2_DIV1
		| RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6
		| RCC_CFGR_PLLMULL6 | RCC_CFGR_PLLSRC
		| RCC_CFGR_SW_PLL;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR), 0xffffffff, cfgr_values);
	
	this->reconfigure_clocks();
}


using test_configurations = Types<
	internal_oscillator_config,
	external_oscillator_config,
	internal_oscillator_config_with_usb,
	external_oscillator_config_with_usb,
	external_low_speed_oscillator_config,
	external_low_speed_oscillator_no_pll_config,
	external_crystal_with_fast_core_req_config,
	external_crystal_with_fast_apb2_req_config,
	spi_config_original<mcutl::clock::spi1>,
	spi_config_modified<mcutl::clock::spi1>
#ifdef RCC_APB1ENR_SPI2EN
	, spi_config_original<mcutl::clock::spi2>
	, spi_config_modified<mcutl::clock::spi2>
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	, spi_config_original<mcutl::clock::spi3>
	, spi_config_modified<mcutl::clock::spi3>
#endif //RCC_APB1ENR_SPI3EN
>;
TYPED_TEST_SUITE(reconfigure_to_same_test_fixture, test_configurations);
TYPED_TEST(reconfigure_to_same_test_fixture, ReconfigureToSameConfig)
{
	this->reconfigure_clocks();
}
