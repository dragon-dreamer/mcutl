#define STM32F103xE
#define STM32F1

#include "mcutl/backup/backup.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class backup_strict_test_fixture :
	public mcutl::tests::memory::strict_test_fixture_base,
	public ::testing::WithParamInterface<bool>
{
public:
	void expect_enable(bool was_enabled)
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
	
	void expect_disable(bool was_enabled, bool rtc_hse_used)
	{
		if (!was_enabled)
			return;
		
		if (rtc_hse_used)
			memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCSEL_HSE);
		memory().allow_reads(addr(&PWR->CR));
		memory().allow_reads(addr(&RCC->BDCR));
	
		if (!rtc_hse_used)
		{
			EXPECT_CALL(memory(), write(addr(&PWR->CR), ::testing::_))
				.WillOnce([this] (auto address, auto value) {
					EXPECT_EQ(value, memory().get(address) & ~PWR_CR_DBP);
					memory().set(address, value);
				});
		}
	}
	
private:
	static constexpr uint32_t initial_pwr_cr_value_ = 0x89abcdefu;
};

TEST_P(backup_strict_test_fixture, EnableWritesTest)
{
	bool was_enabled = GetParam();
	expect_enable(was_enabled);
	if (was_enabled)
		EXPECT_FALSE(mcutl::backup::enable_backup_writes());
	else
		EXPECT_TRUE(mcutl::backup::enable_backup_writes());
}

TEST_P(backup_strict_test_fixture, DisableWritesTest)
{
	bool hse_rtc_used = GetParam();
	expect_disable(true, hse_rtc_used);
	if (hse_rtc_used)
		EXPECT_FALSE(mcutl::backup::disable_backup_writes());
	else
		EXPECT_TRUE(mcutl::backup::disable_backup_writes());
}

TEST_F(backup_strict_test_fixture, BackupReadTest)
{
	memory().set(addr(&BKP->DR1), 111u);
	memory().set(addr(&BKP->DR3), 333u);
	memory().set(addr(&BKP->DR12), 1212u);
	memory().set(addr(&BKP->DR35), 3535u);
	memory().allow_reads(addr(&BKP->DR1));
	memory().allow_reads(addr(&BKP->DR3));
	memory().allow_reads(addr(&BKP->DR12));
	memory().allow_reads(addr(&BKP->DR35));
	memory().allow_reads(addr(&BKP->DR5));
	
	EXPECT_EQ(mcutl::backup::read_backup<1>(), memory().get(addr(&BKP->DR1)));
	EXPECT_EQ(mcutl::backup::read_backup<3>(), memory().get(addr(&BKP->DR3)));
	EXPECT_EQ(mcutl::backup::read_backup<5>(), memory().get(addr(&BKP->DR5)));
	EXPECT_EQ(mcutl::backup::read_backup<12>(), memory().get(addr(&BKP->DR12)));
	EXPECT_EQ(mcutl::backup::read_backup<35>(), memory().get(addr(&BKP->DR35)));
}

TEST_F(backup_strict_test_fixture, BackupWriteNoEnableTest)
{
	constexpr mcutl::backup::backup_register_type
		dr1_value = 111u,
		dr3_value = 333u,
		dr15_value = 1515u;
	
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&BKP->DR1), dr1_value));
	EXPECT_CALL(memory(), write(addr(&BKP->DR3), dr3_value));
	EXPECT_CALL(memory(), write(addr(&BKP->DR15), dr15_value));
	
	mcutl::backup::write_backup<1, false>(dr1_value);
	mcutl::backup::write_backup<3, false>(dr3_value);
	mcutl::backup::write_backup<15, false>(dr15_value);
}

TEST_P(backup_strict_test_fixture, BackupWriteEnableDisableAlwaysTest)
{
	constexpr mcutl::backup::backup_register_type
		dr7_value = 12345u;
	
	memory().allow_reads(addr(&PWR->CR));
	::testing::InSequence s;
	expect_enable(GetParam());
	EXPECT_CALL(memory(), write(addr(&BKP->DR7), dr7_value));
	expect_disable(true, false);
	
	mcutl::backup::write_backup<7, true,
		mcutl::backup::disable_policy::disable_always>(dr7_value);
}

TEST_P(backup_strict_test_fixture, BackupWriteEnableDisableIfEnabledTest)
{
	constexpr mcutl::backup::backup_register_type
		dr7_value = 12345u;
	
	memory().allow_reads(addr(&PWR->CR));
	::testing::InSequence s;
	bool was_enabled = GetParam();
	expect_enable(was_enabled);
	EXPECT_CALL(memory(), write(addr(&BKP->DR7), dr7_value));
	expect_disable(!was_enabled, false);
	
	mcutl::backup::write_backup<7, true,
		mcutl::backup::disable_policy::disable_only_if_enabled>(dr7_value);
}

TEST_P(backup_strict_test_fixture, BackupWriteEnablerDisableAlwaysTest)
{
	constexpr mcutl::backup::backup_register_type
		dr7_value = 12345u,
		dr25_value = 2525u;
	
	memory().allow_reads(addr(&PWR->CR));
	::testing::InSequence s;
	expect_enable(GetParam());
	EXPECT_CALL(memory(), write(addr(&BKP->DR7), dr7_value));
	EXPECT_CALL(memory(), write(addr(&BKP->DR25), dr25_value));
	expect_disable(true, false);
	
	mcutl::backup::backup_write_enabler<
		mcutl::backup::disable_policy::disable_always> bkp;
	bkp.write<7>(dr7_value);
	bkp.write<25>(dr25_value);
}

TEST_P(backup_strict_test_fixture, BackupWriteEnablerDisableIfEnabledTest)
{
	constexpr mcutl::backup::backup_register_type
		dr7_value = 12345u,
		dr25_value = 2525u;
	
	memory().allow_reads(addr(&PWR->CR));
	::testing::InSequence s;
	bool was_enabled = GetParam();
	expect_enable(was_enabled);
	EXPECT_CALL(memory(), write(addr(&BKP->DR7), dr7_value));
	EXPECT_CALL(memory(), write(addr(&BKP->DR25), dr25_value));
	expect_disable(!was_enabled, false);
	
	mcutl::backup::backup_write_enabler<
		mcutl::backup::disable_policy::disable_only_if_enabled> bkp;
	bkp.write<7>(dr7_value);
	bkp.write<25>(dr25_value);
}

INSTANTIATE_TEST_SUITE_P(backup_tests,
	backup_strict_test_fixture,
	::testing::Values(true, false));
