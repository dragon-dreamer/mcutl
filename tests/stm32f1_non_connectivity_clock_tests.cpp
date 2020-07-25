#define STM32F103xE
#define STM32F1

#include <type_traits>

#include "mcutl/clock/clock.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tests/stm32f1_non_connectivity_clock_test_fixtures.h"

using namespace ::testing;
using namespace mcutl::clock::literals;

using minimum_requirements_clock_configurations = Types<
	mcutl::clock::config<
		mcutl::clock::internal_high_speed_crystal,
		mcutl::clock::base_configuration_is_currently_present
	>,
	mcutl::clock::config<
		mcutl::clock::internal_high_speed_crystal,
		mcutl::clock::set_highest_possible_frequencies,
		mcutl::clock::base_configuration_is_currently_present
	>,
	mcutl::clock::config<
		mcutl::clock::internal_high_speed_crystal,
		mcutl::clock::set_highest_possible_frequencies,
		mcutl::clock::base_configuration_is_currently_present,
		mcutl::clock::force_use_pll
	>
>;
template<typename ClockConfig>
using min_req_clock_test_fixture_templated = clock_test_fixture_templated<ClockConfig>;
TYPED_TEST_SUITE(min_req_clock_test_fixture_templated, minimum_requirements_clock_configurations);

TYPED_TEST(min_req_clock_test_fixture_templated, MinimumRequirementsFromBaseConfig)
{
	InSequence seq;
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL16;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR),
		this->get_cfgr_mask_with_pll(), cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	this->expect_set_pll_as_system_and_wait();
	
	this->configure_clocks();
}

TYPED_TEST(min_req_clock_test_fixture_templated, MinimumRequirementsFromBaseConfigTree)
{
	using clock_id = mcutl::clock::clock_id;
	auto best_tree = this->get_best_clock_tree();
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::sys).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::sys).get_exact_frequency(), 64_MHz);
	
	auto sys_parent_id = best_tree.get_node_parent(clock_id::sys);
	EXPECT_EQ(sys_parent_id, clock_id::pll);
	EXPECT_TRUE(best_tree.get_config_by_id(sys_parent_id).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_exact_frequency(), 64_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_prescaler_value(), 16u);
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::apb1).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_exact_frequency(), 32_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_prescaler_value(), 2u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::apb2).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_exact_frequency(), 64_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_prescaler_value(), 1u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::ahb).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_exact_frequency(), 64_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_prescaler_value(), 1u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::adc).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_exact_frequency(), 10'666'666u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_prescaler_value(), 6u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_prescaler_divider(), 1u);
	
	EXPECT_EQ(best_tree.get_node_parent(clock_id::adc), clock_id::apb2);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::apb2), clock_id::ahb);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::apb1), clock_id::ahb);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::ahb), clock_id::sys);
	
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::usb).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer2_3_4_5_6_7_12_13_14).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer1_8_9_10_11).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer2_3_4_5_6_7_12_13_14_multiplier).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer1_8_9_10_11_multiplier).is_used());
}

using internal_osc_with_usb_clock_config = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal,
	mcutl::clock::set_highest_possible_frequencies,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::provide_usb_frequency
>;

TEST_F(clock_test_fixture, InternalOscillatorWithUsb)
{
	InSequence seq;
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV4 | RCC_CFGR_PLLMULL12 | RCC_CFGR_USBPRE;
	this->expect_reg_bits_set(this->addr(&RCC->CFGR),
		this->get_cfgr_mask_with_pll(), cfgr_values);
	expect_pll_on_and_wait_ready();
	expect_flash_latency_change(FLASH_ACR_LATENCY_0);
	expect_set_pll_as_system_and_wait();
	
	mcutl::clock::configure_clocks<internal_osc_with_usb_clock_config>();
}

TEST_F(clock_test_fixture, InternalOscillatorWithUsbTree)
{
	using clock_id = mcutl::clock::clock_id;
	constexpr auto best_tree = mcutl::clock::get_best_clock_tree<internal_osc_with_usb_clock_config>();
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::sys).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::sys).get_exact_frequency(), 48_MHz);
	
	auto sys_parent_id = best_tree.get_node_parent(clock_id::sys);
	EXPECT_EQ(sys_parent_id, clock_id::pll);
	EXPECT_TRUE(best_tree.get_config_by_id(sys_parent_id).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_exact_frequency(), 48_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_prescaler_value(), 12u);
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::apb1).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_exact_frequency(), 24_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_prescaler_value(), 2u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::apb2).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_exact_frequency(), 48_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_prescaler_value(), 1u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::ahb).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_exact_frequency(), 48_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_prescaler_value(), 1u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::adc).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_exact_frequency(), 12_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_prescaler_value(), 4u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_prescaler_divider(), 1u);
	
	EXPECT_EQ(best_tree.get_node_parent(clock_id::adc), clock_id::apb2);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::apb2), clock_id::ahb);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::apb1), clock_id::ahb);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::ahb), clock_id::sys);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::usb).is_used());
	EXPECT_EQ(best_tree.get_node_parent(clock_id::usb), clock_id::pll);
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer2_3_4_5_6_7_12_13_14).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer1_8_9_10_11).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer2_3_4_5_6_7_12_13_14_multiplier).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer1_8_9_10_11_multiplier).is_used());
}

using external_oscillator_with_usb_clock = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::set_highest_possible_frequencies,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::provide_usb_frequency
>;
using external_bypass_with_usb_clock = mcutl::clock::config<
	mcutl::clock::external_high_speed_bypass<16_MHz>,
	mcutl::clock::set_highest_possible_frequencies,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::provide_usb_frequency
>;
using external_oscillator_with_usb_clock_configurations = Types<
	external_oscillator_with_usb_clock,
	external_bypass_with_usb_clock
>;
template<typename ClockConfig>
using external_oscillator_with_usb_test_fixture_templated = clock_test_fixture_templated<ClockConfig>;
TYPED_TEST_SUITE(external_oscillator_with_usb_test_fixture_templated, external_oscillator_with_usb_clock_configurations);

TYPED_TEST(external_oscillator_with_usb_test_fixture_templated, ExternalOscillatorWithUsb)
{
	InSequence seq;
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC;
	this->expect_hse_on_and_wait_ready(
		std::is_same_v<typename TestFixture::clock_config_t, external_oscillator_with_usb_clock>
		? RCC_CR_HSEON : (RCC_CR_HSEON | RCC_CR_HSEBYP)
	);
	this->expect_reg_bits_set(this->addr(&RCC->CFGR),
		this->get_cfgr_mask_with_pll(), cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	this->expect_set_pll_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->configure_clocks();
}

TYPED_TEST(external_oscillator_with_usb_test_fixture_templated, ExternalOscillatorWithUsbTree)
{
	using clock_id = mcutl::clock::clock_id;
	auto best_tree = this->get_best_clock_tree();
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::sys).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::sys).get_exact_frequency(), 72_MHz);
	
	auto sys_parent_id = best_tree.get_node_parent(clock_id::sys);
	EXPECT_EQ(sys_parent_id, clock_id::pll);
	EXPECT_TRUE(best_tree.get_config_by_id(sys_parent_id).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_exact_frequency(), 72_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_prescaler_value(), 9u);
	EXPECT_EQ(best_tree.get_config_by_id(sys_parent_id).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::hse_prediv).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::hse_prediv).get_exact_frequency(), 8_MHz);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::pll), clock_id::hse_prediv);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::apb1).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_exact_frequency(), 36_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_prescaler_value(), 2u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb1).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::apb2).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_exact_frequency(), 72_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_prescaler_value(), 1u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::apb2).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::ahb).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_exact_frequency(), 72_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_prescaler_value(), 1u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::ahb).get_prescaler_divider(), 1u);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::adc).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_exact_frequency(), 12_MHz);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_prescaler_value(), 6u);
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::adc).get_prescaler_divider(), 1u);
	
	EXPECT_EQ(best_tree.get_node_parent(clock_id::adc), clock_id::apb2);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::apb2), clock_id::ahb);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::apb1), clock_id::ahb);
	EXPECT_EQ(best_tree.get_node_parent(clock_id::ahb), clock_id::sys);
	
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::usb).is_used());
	EXPECT_EQ(best_tree.get_node_parent(clock_id::usb), clock_id::pll);
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer2_3_4_5_6_7_12_13_14).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer1_8_9_10_11).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer2_3_4_5_6_7_12_13_14_multiplier).is_used());
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::timer1_8_9_10_11_multiplier).is_used());
}

using external_oscillator_core_limit_with_usb = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::set_highest_possible_frequencies,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::provide_usb_frequency,
	mcutl::clock::core<mcutl::clock::required_frequency<48_MHz>>
>;

TEST_F(clock_test_fixture, ExternalOscillatorCoreLimitWithUsb)
{
	InSequence seq;
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV4 | RCC_CFGR_PLLMULL3
		| RCC_CFGR_PLLSRC | RCC_CFGR_USBPRE;
	expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	expect_reg_bits_set(addr(&RCC->CFGR),
		get_cfgr_mask_with_pll(), cfgr_values);
	expect_pll_on_and_wait_ready();
	expect_flash_latency_change(FLASH_ACR_LATENCY_0);
	expect_set_pll_as_system_and_wait();
	expect_disable_hsi();
	
	using clock_id = mcutl::clock::clock_id;
	constexpr auto best_tree = mcutl::clock::get_best_clock_tree<external_oscillator_core_limit_with_usb>();
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::sys).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::sys).get_exact_frequency(), 48_MHz);
	
	mcutl::clock::configure_clocks<external_oscillator_core_limit_with_usb>();
}

using no_pll_with_core_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::force_skip_pll,
	mcutl::clock::core<mcutl::clock::min_frequency<10_MHz>>
>;

TEST_F(clock_test_fixture, NoPllWithCore)
{
	InSequence seq;
	
	expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	expect_reg_bits_set(addr(&RCC->CFGR),
		get_cfgr_mask_without_pll(), 0);
	expect_flash_latency_change(0);
	expect_set_hse_as_system_and_wait();
	expect_disable_hsi();
	
	using clock_id = mcutl::clock::clock_id;
	constexpr auto best_tree = mcutl::clock::get_best_clock_tree<no_pll_with_core_config>();
	EXPECT_TRUE(best_tree.get_config_by_id(clock_id::sys).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(clock_id::sys).get_exact_frequency(), 16_MHz);
	EXPECT_FALSE(best_tree.get_config_by_id(clock_id::pll).is_used());
	EXPECT_EQ(best_tree.get_node_parent(clock_id::sys), clock_id::hse);
	
	mcutl::clock::configure_clocks<no_pll_with_core_config>();
}

using timer1_config_apb2_prescaler1 = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::apb2<mcutl::clock::required_frequency<72_MHz>>,
	mcutl::clock::timer1_8_9_10_11<>
>;

TEST_F(clock_test_fixture, Timer1ConfigWithApb2Prescaler1)
{
	InSequence seq;
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC;
	expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	expect_reg_bits_set(addr(&RCC->CFGR),
		get_cfgr_mask_with_pll(), cfgr_values);
	expect_pll_on_and_wait_ready();
	expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	expect_set_pll_as_system_and_wait();
	expect_disable_hsi();
	
	mcutl::clock::configure_clocks<timer1_config_apb2_prescaler1>();
	
	constexpr auto timer1_info = mcutl::clock::get_clock_info<timer1_config_apb2_prescaler1,
		mcutl::clock::clock_id::timer1_8_9_10_11>();
	EXPECT_EQ(timer1_info.get_exact_frequency(), 72_MHz);
}

using timer1_config_apb2_prescaler4 = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::base_configuration_is_currently_present,
	mcutl::clock::apb2<mcutl::clock::required_frequency<18_MHz>>,
	mcutl::clock::timer1_8_9_10_11<>
>;

TEST_F(clock_test_fixture, Timer1ConfigWithApb2Prescaler4)
{
	InSequence seq;
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV4
		| RCC_CFGR_ADCPRE_DIV2 | RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC;
	expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	expect_reg_bits_set(addr(&RCC->CFGR),
		get_cfgr_mask_with_pll(), cfgr_values);
	expect_pll_on_and_wait_ready();
	expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	expect_set_pll_as_system_and_wait();
	expect_disable_hsi();
	
	mcutl::clock::configure_clocks<timer1_config_apb2_prescaler4>();
	
	constexpr auto timer1_info = mcutl::clock::get_clock_info<timer1_config_apb2_prescaler4,
		mcutl::clock::clock_id::timer1_8_9_10_11>();
	EXPECT_EQ(timer1_info.get_exact_frequency(), 36_MHz);
}

template<template<typename...> typename Spi>
using spi_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::base_configuration_is_currently_present,
	Spi<mcutl::clock::min_frequency<100_KHz>, mcutl::clock::max_frequency<200_KHz>>
>;

using spi_configurations = Types<
	spi_config<mcutl::clock::spi1>
#ifdef RCC_APB1ENR_SPI2EN
	, spi_config<mcutl::clock::spi2>
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	, spi_config<mcutl::clock::spi3>
#endif //RCC_APB1ENR_SPI3EN
>;
template<typename ClockConfig>
using spi_test_fixture_templated = clock_test_fixture_templated<ClockConfig>;
TYPED_TEST_SUITE(spi_test_fixture_templated, spi_configurations);

TYPED_TEST(spi_test_fixture_templated, Spi)
{
	using clock_id = mcutl::clock::clock_id;
	clock_id spi_id {};
	mcutl::tests::memory_address_t spi_cr{};
	
	constexpr bool is_spi1 = std::is_same_v<typename TestFixture::clock_config_t,
		spi_config<mcutl::clock::spi1>>;
	
	if constexpr (is_spi1)
	{
		spi_id = clock_id::spi1;
		spi_cr = this->addr(&SPI1->CR1);
	}
#ifdef RCC_APB1ENR_SPI2EN
	else if constexpr (std::is_same_v<typename TestFixture::clock_config_t,
		spi_config<mcutl::clock::spi2>>)
	{
		spi_id = clock_id::spi2;
		spi_cr = this->addr(&SPI2->CR1);
	}
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	else if constexpr (std::is_same_v<typename TestFixture::clock_config_t,
		spi_config<mcutl::clock::spi3>>)
	{
		spi_id = clock_id::spi3;
		spi_cr = this->addr(&SPI3->CR1);
	}
#endif //RCC_APB1ENR_SPI3EN
	
	this->memory().allow_reads(spi_cr);
	
	InSequence seq;
	
	auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2 | RCC_CFGR_PLLSRC;
	if constexpr (is_spi1)
		cfgr_values |= RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_ADCPRE_DIV4;
	else
		cfgr_values |= RCC_CFGR_ADCPRE_DIV6;
	this->expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	this->expect_reg_bits_set(this->addr(&RCC->CFGR),
		this->get_cfgr_mask_with_pll(), cfgr_values);
	this->expect_pll_on_and_wait_ready();
	this->expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	this->expect_set_pll_as_system_and_wait();
	this->expect_disable_hsi();
	
	this->expect_spi_prescaler_change(spi_cr, SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0);
	
	this->configure_clocks();
	
	constexpr auto best_tree = mcutl::clock::get_best_clock_tree<
		typename TestFixture::clock_config_t>();
	EXPECT_TRUE(best_tree.get_config_by_id(spi_id).is_used());
	EXPECT_EQ(best_tree.get_config_by_id(spi_id).get_exact_frequency(), 140625u);
}

using unknown_initial_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::provide_usb_frequency
>;

TEST_P(unknown_initial_config_test_fixture, UnknownInitialConfig)
{
	InSequence seq;
	
	if (GetParam().apb1enr & RCC_APB1ENR_USBEN)
		expect_usb_disable();
	
	if ((GetParam().cr & (RCC_CR_HSION | RCC_CR_HSIRDY))
		!= (RCC_CR_HSION | RCC_CR_HSIRDY))
	{
		expect_hsi_on_and_wait_ready();
	}
	
	if ((GetParam().cfgr & (RCC_CFGR_SW | RCC_CFGR_SWS))
		!= (RCC_CFGR_SW_HSI | RCC_CFGR_SWS_HSI))
	{
		expect_set_hsi_as_system_and_wait();
	}
	
	if (GetParam().cr & (RCC_CR_PLLON | RCC_CR_PLLRDY))
		expect_pll_off_and_wait_ready();
	
	if (GetParam().cr & (RCC_CR_HSEON | RCC_CR_HSERDY))
		expect_hse_off_and_wait_ready();
	
	constexpr auto cfgr_values = RCC_CFGR_PPRE1_DIV2
		| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC;
	expect_hse_on_and_wait_ready(RCC_CR_HSEON);
	expect_reg_bits_set(addr(&RCC->CFGR),
		get_cfgr_mask_with_pll(), cfgr_values);
	expect_pll_on_and_wait_ready();
	expect_flash_latency_change(FLASH_ACR_LATENCY_1);
	expect_set_pll_as_system_and_wait();
	
	if (GetParam().apb1enr & RCC_APB1ENR_USBEN)
		expect_usb_enable();
	
	expect_disable_hsi();
	
	mcutl::clock::configure_clocks<unknown_initial_config>();
}

INSTANTIATE_TEST_SUITE_P(unknown_initial_config_test_fixture,
	unknown_initial_config_test_fixture,
	Values(
		initial_config{ RCC_CFGR_SW_HSI | RCC_CFGR_SWS_HSI,
				RCC_CR_HSION | RCC_CR_HSIRDY, RCC_APB1ENR_USBEN },
		initial_config{ RCC_CFGR_SW_HSE | RCC_CFGR_SWS_HSE,
				RCC_CR_HSEON | RCC_CR_HSERDY, 0 },
		initial_config{ RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL,
				RCC_CR_PLLON | RCC_CR_PLLRDY, 0 },
		initial_config{ RCC_CFGR_SW_PLL | RCC_CFGR_SWS_PLL,
				RCC_CR_HSEON | RCC_CR_HSERDY | RCC_CR_PLLON | RCC_CR_PLLRDY, 0 }
	)
);
