#pragma once

#include <memory>

#include "mcutl/backup/backup.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class backup_strict_test_fixture :
	virtual public mcutl::tests::mcu::strict_test_fixture_base,
	virtual public::testing::WithParamInterface<bool>
{
public:
	void expect_enable_backup_writes(bool was_enabled)
	{
		uint32_t initial_pwr_cr_value = initial_pwr_cr_value_;
		if (was_enabled)
			initial_pwr_cr_value |= PWR_CR_DBP;
		else
			initial_pwr_cr_value &= ~PWR_CR_DBP;
		
		memory().set(addr(&PWR->CR), initial_pwr_cr_value);
		memory().allow_reads(addr(&PWR->CR));
		if (!was_enabled)
		{
			EXPECT_CALL(memory(), write(addr(&PWR->CR),
				(initial_pwr_cr_value & ~PWR_CR_DBP_Msk) | PWR_CR_DBP));
		}
	}
	
	void expect_disable_backup_writes(bool was_enabled, bool rtc_hse_used)
	{
		if (!was_enabled)
			return;
		
		if (rtc_hse_used)
			memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCSEL_HSE);
		memory().allow_reads(addr(&RCC->BDCR));
		memory().allow_reads(addr(&PWR->CR));
	
		if (!rtc_hse_used)
		{
			EXPECT_CALL(memory(), write(addr(&PWR->CR),::testing::_))
				.WillOnce([this] (auto address, auto value) {
					EXPECT_EQ(value, memory().get(address) & ~PWR_CR_DBP);
					memory().set(address, value);
				});
		}
	}
	
private:
	static constexpr uint32_t initial_pwr_cr_value_ = 0x89abcdefu;
};
