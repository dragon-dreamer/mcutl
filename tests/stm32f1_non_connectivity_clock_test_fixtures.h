#pragma once

#include <memory>

#include "mcutl/clock/clock.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;
using namespace mcutl::clock::literals;

class clock_test_fixture : public mcutl::tests::strict_test_fixture_base
{
public:
	virtual void SetUp() override
	{
		mcutl::tests::strict_test_fixture_base::SetUp();
		memory().set(addr(&RCC->CR), 0xff83);
		memory().allow_reads(addr(&RCC->CR));
		memory().allow_reads(addr(&RCC->CFGR));
		memory().allow_reads(addr(&FLASH->ACR));
	}
	
	void expect_reg_bits_set(mcutl::tests::memory_address_t address,
		uint32_t mask, uint32_t bits)
	{
		bits &= mask;
		EXPECT_CALL(memory(), write(address, _))
			.WillOnce([this, mask, bits] (auto address, auto value) {
				EXPECT_EQ(value, (memory().get(address) & ~mask) | bits);
				memory().set(address, value);
			});
	}
	
	void expect_flash_latency_change(uint32_t flash_acr_value)
	{
		expect_reg_bits_set(addr(&FLASH->ACR), FLASH_ACR_LATENCY, flash_acr_value);
	}
	
	void expect_set_source_as_system_and_wait(uint32_t source_enable_mask,
		uint32_t source_ready_mask, bool wait)
	{
		EXPECT_CALL(memory(), write(addr(&RCC->CFGR), _))
			.WillOnce([this, source_enable_mask] (auto address, auto value) {
				auto old_value = memory().get(address);
				EXPECT_PRED3([](auto new_value, auto old_value, auto enable_mask) {
					return new_value == ((old_value & ~RCC_CFGR_SW) | enable_mask)
						|| new_value == ((old_value & ~(RCC_CFGR_SW | RCC_CFGR_SWS)) | enable_mask);
					}, value, old_value, source_enable_mask);
				memory().set(address, value | (old_value & RCC_CFGR_SWS));
			});
		
		EXPECT_CALL(memory(), read(addr(&RCC->CFGR)))
			.WillOnce([this, source_ready_mask] (auto address) {
			auto old_value = memory().get(address);
			memory().set(address, (old_value & ~RCC_CFGR_SWS) | source_ready_mask);
			return old_value;
		});
		if (wait)
		{
			EXPECT_CALL(memory(), read(addr(&RCC->CFGR))).Times(AtLeast(1));
		}
	}
	
	void expect_set_pll_as_system_and_wait(bool wait = true)
	{
		expect_set_source_as_system_and_wait(RCC_CFGR_SW_PLL, RCC_CFGR_SWS_PLL, wait);
	}
	
	void expect_set_hse_as_system_and_wait(bool wait = true)
	{
		expect_set_source_as_system_and_wait(RCC_CFGR_SW_HSE, RCC_CFGR_SWS_HSE, wait);
	}
	
	void expect_set_hsi_as_system_and_wait(bool wait = true)
	{
		expect_set_source_as_system_and_wait(RCC_CFGR_SW_HSI, RCC_CFGR_SWS_HSI, wait);
	}
	
	void expect_source_on_and_wait_ready(uint32_t on_bits, uint32_t rdy_bits)
	{
		expect_reg_bits_set(addr(&RCC->CR), on_bits, on_bits);
		EXPECT_CALL(memory(), read(addr(&RCC->CR)))
			.WillOnce([this, rdy_bits] (auto address) {
			auto old_value = memory().get(address) & ~rdy_bits;
			memory().get(address) |= rdy_bits;
			return old_value;
		});
		EXPECT_CALL(memory(), read(addr(&RCC->CR))).Times(AtLeast(1));
	}
	
	void expect_hse_on_and_wait_ready(uint32_t hse_bits)
	{
		expect_source_on_and_wait_ready(hse_bits, RCC_CR_HSERDY);
	}
	
	void expect_pll_on_and_wait_ready()
	{
		expect_source_on_and_wait_ready(RCC_CR_PLLON, RCC_CR_PLLRDY);
	}
	
	void expect_hsi_on_and_wait_ready()
	{
		expect_source_on_and_wait_ready(RCC_CR_HSION, RCC_CR_HSIRDY);
	}
	
	void expect_disable_hsi()
	{
		EXPECT_CALL(memory(), write(addr(&RCC->CR), _))
			.WillOnce([this] (auto address, auto value) {
				EXPECT_EQ(value, memory().get(address) & ~RCC_CR_HSION);
				memory().set(address, value);
			});
	}
	
	void expect_spi_prescaler_change(mcutl::tests::memory_address_t address,
		uint32_t prescaler_bits)
	{
		expect_reg_bits_set(address, SPI_CR1_BR, prescaler_bits);
	}
	
	void expect_usb_disable()
	{
		expect_reg_bits_set(addr(&RCC->APB1ENR), RCC_APB1ENR_USBEN,
			~static_cast<uint32_t>(RCC_APB1ENR_USBEN));
		EXPECT_CALL(memory(), read(addr(&RCC->APB1ENR))).Times(AtLeast(1));
	}
	
	void expect_usb_enable()
	{
		expect_reg_bits_set(addr(&RCC->APB1ENR), RCC_APB1ENR_USBEN, RCC_APB1ENR_USBEN);
		EXPECT_CALL(memory(), read(addr(&RCC->APB1ENR))).Times(AtLeast(1));
	}
	
	void expect_source_off_and_wait_ready(uint32_t on_bits, uint32_t rdy_bits)
	{
		expect_reg_bits_set(addr(&RCC->CR), on_bits, ~on_bits);
		EXPECT_CALL(memory(), read(addr(&RCC->CR)))
			.WillOnce([this, rdy_bits] (auto address) {
			auto old_value = memory().get(address) | rdy_bits;
			memory().get(address) &= ~rdy_bits;
			return old_value;
		});
		EXPECT_CALL(memory(), read(addr(&RCC->CR))).Times(AtLeast(1));
	}
	
	void expect_hse_off_and_wait_ready(bool disable_bypass = true)
	{
		expect_source_off_and_wait_ready(RCC_CR_HSEON, RCC_CR_HSERDY);
		if (disable_bypass)
		{
			expect_reg_bits_set(addr(&RCC->CR), RCC_CR_HSEBYP,
				~static_cast<uint32_t>(RCC_CR_HSEBYP));
		}
	}
	
	void expect_pll_off_and_wait_ready()
	{
		expect_source_off_and_wait_ready(RCC_CR_PLLON, RCC_CR_PLLRDY);
	}
	
	static uint32_t get_cfgr_mask_with_pll() noexcept
	{
		return get_cfgr_mask_without_pll()
			| RCC_CFGR_PLLMULL | RCC_CFGR_PLLXTPRE_HSE_DIV2 | RCC_CFGR_PLLSRC | RCC_CFGR_USBPRE;
	}
	
	static uint32_t get_cfgr_mask_without_pll() noexcept
	{
		return RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 | RCC_CFGR_ADCPRE | RCC_CFGR_HPRE | RCC_CFGR_SWS;
	}
};

template<typename ClockConfig>
class clock_test_fixture_templated : public clock_test_fixture
{
public:
	using clock_config_t = ClockConfig;
	
public:
	static void configure_clocks() noexcept
	{
		mcutl::clock::configure_clocks<ClockConfig>();
	}
	
	static constexpr auto get_best_clock_tree() noexcept
	{
		return mcutl::clock::get_best_clock_tree<ClockConfig>();
	}
};

struct initial_config
{
	uint32_t cfgr = 0;
	uint32_t cr = 0;
	uint32_t apb1enr = 0;
};

class unknown_initial_config_test_fixture :
	public clock_test_fixture,
	public WithParamInterface<initial_config>
{
public:
	virtual void SetUp() override
	{
		clock_test_fixture::SetUp();
		memory().allow_reads(addr(&RCC->APB1ENR));
		memory().set(addr(&RCC->CFGR), GetParam().cfgr);
		memory().set(addr(&RCC->CR), GetParam().cr);
		memory().set(addr(&RCC->APB1ENR), GetParam().apb1enr);
	}
};
