#define STM32F107xC
#define STM32F1

#include <stdint.h>
#include <type_traits>

#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph_defs.h"
#include "mcutl/rtc/rtc.h"
#include "mcutl/tests/mcu.h"
#include "mcutl/utils/duration.h"
#include "mcutl/utils/type_helpers.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tests/stm32f1_backup_test_fixture.h"
#include "tests/stm32f1_exti_interrupt_test_fixture.h"

class rtc_strict_test_fixture : public backup_strict_test_fixture,
	public exti_interrupt_test_fixture
{
public:
	void expect_single_crl_read(uint32_t crl)
	{
		::testing::InSequence s;
		expect_enable_rtc_config(false);
		EXPECT_CALL(memory(), read(addr(&RTC->CRL)))
			.WillOnce([crl] (auto) { return crl; });
	}
	
	void enable_interrupt(uint32_t irqn)
	{
		memory().allow_reads(addr(&(NVIC->ISER[RTC_IRQn / 32])));
		memory().allow_reads(addr(&(NVIC->ISER[RTC_Alarm_IRQn / 32])));
		memory().set(addr(&(NVIC->ISER[irqn / 32])), 1 << (irqn % 32));
	}
	
	void expect_reset_backup_domain(uint32_t bdcr)
	{
		::testing::InSequence s;
		EXPECT_CALL(memory(), write(addr(&RCC->BDCR), bdcr | RCC_BDCR_BDRST));
		EXPECT_CALL(memory(), read(addr(&RCC->BDCR)))
			.Times(2)
			.WillOnce([this] (auto) { return memory().get(addr(&RCC->BDCR)); })
			.WillOnce([this] (auto) { return memory().get(addr(&RCC->BDCR)) & ~RCC_BDCR_BDRST; });
	}
	
	void expect_enable_disable_alarm_line(bool enable)
	{
		memory().set(addr(&EXTI->FTSR), 0x12345678);
		memory().set(addr(&EXTI->RTSR), 0x90abcdef);
		memory().set(addr(&EXTI->EMR), 0x12345678);
		memory().set(addr(&EXTI->IMR), 0x90abcdef);
	
		uint32_t ft = 0, rt = 0, em = 0, im = 0;
		uint32_t mask = 1u << 17;
		if (enable)
		{
			rt = EXTI_RTSR_RT17;
			im = EXTI_IMR_IM17;
		}
		
		EXPECT_CALL(memory(), read(addr(&EXTI->FTSR)));
		EXPECT_CALL(memory(), write(addr(&EXTI->FTSR),
			(0x12345678 & ~mask) | ft));
		EXPECT_CALL(memory(), read(addr(&EXTI->RTSR)));
		EXPECT_CALL(memory(), write(addr(&EXTI->RTSR),
			(0x90abcdef & ~mask) | rt));
		EXPECT_CALL(memory(), read(addr(&EXTI->EMR)));
		EXPECT_CALL(memory(), write(addr(&EXTI->EMR),
			(0x12345678 & ~mask) | em));
		EXPECT_CALL(memory(), read(addr(&EXTI->IMR)));
		EXPECT_CALL(memory(), write(addr(&EXTI->IMR),
			(0x90abcdef & ~mask) | im));
	}
	
	void expect_set_rtc_prescaler(uint32_t prescaler)
	{
		EXPECT_CALL(memory(), write(addr(&RTC->PRLL), prescaler & 0xffff));
		EXPECT_CALL(memory(), write(addr(&RTC->PRLH), prescaler >> 16));
	}
	
	void expect_enable_lse(uint32_t bdcr)
	{
		::testing::InSequence s;
		EXPECT_CALL(memory(), write(addr(&RCC->BDCR), bdcr));
		EXPECT_CALL(memory(), read(addr(&RCC->BDCR)))
			.Times(2)
			.WillOnce([this] (auto) { return memory().get(addr(&RCC->BDCR)) & ~RCC_BDCR_LSERDY; })
			.WillOnce([this] (auto) { return memory().get(addr(&RCC->BDCR)) | RCC_BDCR_LSERDY; });
	}
	
	void expect_disable_lse(uint32_t bdcr)
	{
		::testing::InSequence s;
		EXPECT_CALL(memory(), write(addr(&RCC->BDCR), bdcr));
		EXPECT_CALL(memory(), read(addr(&RCC->BDCR)))
			.Times(2)
			.WillOnce([this] (auto) { return memory().get(addr(&RCC->BDCR)) | RCC_BDCR_LSERDY; })
			.WillOnce([this] (auto) { return memory().get(addr(&RCC->BDCR)) & ~RCC_BDCR_LSERDY; });
	}
	
	void expect_enable_lsi()
	{
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(addr(&RCC->CSR)))
			.WillOnce([] (auto) { return 0xffffffff & ~(RCC_CSR_LSION | RCC_CSR_LSIRDY); });
		EXPECT_CALL(memory(), write(addr(&RCC->CSR), 0xffffffff & ~RCC_CSR_LSIRDY));
		EXPECT_CALL(memory(), read(addr(&RCC->CSR)))
			.Times(2)
			.WillOnce([] (auto) { return RCC_CSR_LSION; })
			.WillOnce([] (auto) { return RCC_CSR_LSION | RCC_CSR_LSIRDY; });
	}
	
	void expect_set_bdcr(uint32_t bdcr)
	{
		EXPECT_CALL(memory(), write(addr(&RCC->BDCR), bdcr));
	}
	
	void expect_set_crh(uint32_t crh)
	{
		EXPECT_CALL(memory(), write(addr(&RTC->CRH), crh));
	}
	
	void expect_set_rtc_value()
	{
		::testing::InSequence s;
		expect_enable_backup_writes(false);
		expect_enable_rtc_config(true);
	
		EXPECT_CALL(memory(), write(addr(&RTC->CNTL), 0));
		EXPECT_CALL(memory(), write(addr(&RTC->CNTH), test_timestamp >> 16));
		EXPECT_CALL(memory(), write(addr(&RTC->CNTL), test_timestamp & 0xffff));
	
		expect_disable_rtc_config(true);
		expect_disable_backup_writes(true, false);
	}
	
	void expect_set_alarm_value()
	{
		::testing::InSequence s;
		
		EXPECT_CALL(memory(), write(addr(&EXTI->PR), 1 << mcutl::exti::rtc_alarm_line::value));
		
		expect_enable_backup_writes(false);
		expect_enable_rtc_config(true);
	
		EXPECT_CALL(memory(), write(addr(&RTC->ALRH), test_timestamp >> 16));
		EXPECT_CALL(memory(), write(addr(&RTC->ALRL), test_timestamp & 0xffff));
		
		EXPECT_CALL(memory(), write(addr(&RTC->CRL),
			RTC_CRL_CNF | RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF));
	
		expect_disable_rtc_config(true);
		expect_disable_backup_writes(true, false);
	}
	
	void expect_rtc_value_read()
	{
		::testing::Sequence s1, s2;
		expect_enable_rtc_config(false, s1, s2);
		EXPECT_CALL(memory(), read(addr(&RTC->CNTH)))
			.Times(::testing::AtLeast(1))
			.InSequence(s1);
		EXPECT_CALL(memory(), read(addr(&RTC->CNTL)))
			.Times(::testing::AtLeast(1))
			.InSequence(s2);
	}
	
	void expect_enable_rtc_config(bool requires_config_mode,
		const ::testing::Sequence& s1 = {},
		const ::testing::Sequence& s2 = {})
	{
		EXPECT_CALL(memory(), read(addr(&RTC->CRL)))
			.Times(2)
			.InSequence(s1, s2)
			.WillRepeatedly([this] (auto)
			{
				rtoff_ = !rtoff_;
				return rtoff_ ? RTC_CRL_RTOFF : 0;
			});
		
		EXPECT_CALL(memory(), write(addr(&RTC->CRL),
			RTC_CRL_ALRF | RTC_CRL_SECF | RTC_CRL_OWF))
			.InSequence(s1, s2);
		
		EXPECT_CALL(memory(), read(addr(&RTC->CRL)))
			.Times(2)
			.InSequence(s1, s2)
			.WillRepeatedly([this] (auto)
			{
				rsf_ = !rsf_;
				return rsf_ ? (RTC_CRL_RSF | RTC_CRL_RTOFF) : RTC_CRL_RTOFF;
			});
		
		if (requires_config_mode)
		{
			EXPECT_CALL(memory(), write(addr(&RTC->CRL),
				RTC_CRL_CNF | RTC_CRL_ALRF | RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF))
			.InSequence(s1, s2);
		}
	}
	
	void expect_disable_rtc_config(bool requires_config_mode)
	{
		::testing::InSequence s;
		
		if (requires_config_mode)
		{
			EXPECT_CALL(memory(), write(addr(&RTC->CRL),
				RTC_CRL_ALRF | RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF));
		}
		
		EXPECT_CALL(memory(), read(addr(&RTC->CRL)))
			.Times(2)
			.WillRepeatedly([this] (auto)
			{
				rtoff_ = !rtoff_;
				return rtoff_ ? RTC_CRL_RTOFF : 0;
			});
	}
	
public:
	static constexpr mcutl::datetime::timestamp_t test_timestamp = 0x12345678u;
	
private:
	bool rtoff_ = true;
	bool rsf_ = true;
};

TEST_F(rtc_strict_test_fixture, TraitsTest)
{
	EXPECT_TRUE(mcutl::rtc::supports_alarm);
	EXPECT_TRUE(mcutl::rtc::supports_second_interrupt);
	EXPECT_TRUE(mcutl::rtc::supports_overflow_interrupt);
	EXPECT_TRUE(mcutl::rtc::supports_internal_clock_source);
	EXPECT_FALSE(mcutl::rtc::supports_atomic_clear_pending_flags);
	EXPECT_FALSE(mcutl::rtc::supports_clock_source_reconfiguration);
	EXPECT_TRUE(mcutl::rtc::supports_prescalers);
	EXPECT_EQ(mcutl::rtc::min_prescaler, 1u);
	EXPECT_EQ(mcutl::rtc::max_prescaler, 0xfffffu);
	EXPECT_FALSE(mcutl::rtc::prescaler_supported<0>);
	EXPECT_FALSE(mcutl::rtc::prescaler_supported<0x100000u>);
	EXPECT_TRUE(mcutl::rtc::prescaler_supported<123>);
	
	EXPECT_TRUE((std::is_same_v<mcutl::rtc::peripheral_type,
		mcutl::types::list<mcutl::periph::pwr, mcutl::periph::bkp>>));
	EXPECT_TRUE((std::is_same_v<mcutl::rtc::interrupt_type<mcutl::rtc::interrupt::alarm>,
		mcutl::interrupt::type::rtc>));
	EXPECT_TRUE((std::is_same_v<mcutl::rtc::interrupt_type<mcutl::rtc::interrupt::overflow>,
		mcutl::interrupt::type::rtc>));
	EXPECT_TRUE((std::is_same_v<mcutl::rtc::interrupt_type<mcutl::rtc::interrupt::second>,
		mcutl::interrupt::type::rtc>));
	EXPECT_TRUE((std::is_same_v<mcutl::rtc::interrupt_type<mcutl::rtc::interrupt::external_alarm>,
		mcutl::interrupt::type::rtc_alarm>));
}

TEST_F(rtc_strict_test_fixture, ClearPendingFlagsTest)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(false);
	
	EXPECT_CALL(memory(), write(addr(&RTC->CRL), RTC_CRL_OWF | RTC_CRL_RSF));
	
	expect_disable_rtc_config(false);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::clear_pending_flags<mcutl::rtc::interrupt::alarm,
		mcutl::rtc::interrupt::second>();
}

TEST_F(rtc_strict_test_fixture, ClearPendingFlagsWithExternalAlarmTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), write(addr(&EXTI->PR), 1 << mcutl::exti::rtc_alarm_line::value));
	
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(false);
	
	EXPECT_CALL(memory(), write(addr(&RTC->CRL), RTC_CRL_RSF));
	
	expect_disable_rtc_config(false);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::clear_pending_flags<mcutl::rtc::interrupt::overflow,
		mcutl::rtc::interrupt::second,
		mcutl::rtc::interrupt::external_alarm>();
}

TEST_F(rtc_strict_test_fixture, IsEnabledTest)
{
	memory().allow_reads(addr(&RCC->BDCR));
	EXPECT_FALSE(mcutl::rtc::is_enabled());
	
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCEN);
	EXPECT_FALSE(mcutl::rtc::is_enabled());
	
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCSEL_HSE);
	EXPECT_FALSE(mcutl::rtc::is_enabled());
	
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_HSE);
	EXPECT_TRUE(mcutl::rtc::is_enabled());
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE);
	EXPECT_TRUE(mcutl::rtc::is_enabled());
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSI);
	EXPECT_TRUE(mcutl::rtc::is_enabled());
}

TEST_F(rtc_strict_test_fixture, HasAlarmedTest)
{
	expect_single_crl_read(0);
	EXPECT_FALSE(mcutl::rtc::has_alarmed());
	
	expect_single_crl_read(RTC_CRL_ALRF);
	EXPECT_TRUE(mcutl::rtc::has_alarmed());
}

struct timestamp_only
{
	mcutl::datetime::timestamp_t timestamp = 0;
};

TEST_F(rtc_strict_test_fixture, GetTimestampTest)
{
	memory().set(addr(&RTC->CNTH), 0x1234);
	memory().set(addr(&RTC->CNTL), 0x5678);
	
	expect_rtc_value_read();
	
	timestamp_only result;
	mcutl::rtc::get_date_time(result);
	EXPECT_EQ(result.timestamp, 0x12345678u);
}

TEST_F(rtc_strict_test_fixture, TimestampChangedWhenReatingTest)
{
	::testing::Sequence s1, s2;
	expect_enable_rtc_config(false, s1, s2);
	EXPECT_CALL(memory(), read(addr(&RTC->CNTH)))
		.Times(2)
		.InSequence(s1)
		.WillOnce([] (auto) { return 0x1234u; })
		.WillRepeatedly([] (auto) { return 0x1235u; });
	EXPECT_CALL(memory(), read(addr(&RTC->CNTL)))
		.Times(2)
		.InSequence(s2)
		.WillOnce([] (auto) { return 0xffff; })
		.WillRepeatedly([] (auto) { return 0x1u; });
	
	timestamp_only result;
	mcutl::rtc::get_date_time(result);
	EXPECT_EQ(result.timestamp, 0x12350001u);
}

TEST_F(rtc_strict_test_fixture, GetDateTest)
{
	memory().set(addr(&RTC->CNTH), test_timestamp >> 16);
	memory().set(addr(&RTC->CNTL), test_timestamp & 0xffff);
	
	expect_rtc_value_read();
	
	mcutl::datetime::date_value result;
	mcutl::rtc::get_date_time(result);
	EXPECT_EQ(result.day, mcutl::datetime::date_from_timestamp(test_timestamp).day);
	EXPECT_EQ(result.month, mcutl::datetime::date_from_timestamp(test_timestamp).month);
	EXPECT_EQ(result.year, mcutl::datetime::date_from_timestamp(test_timestamp).year);
}

TEST_F(rtc_strict_test_fixture, GetTimeTest)
{
	memory().set(addr(&RTC->CNTH), test_timestamp >> 16);
	memory().set(addr(&RTC->CNTL), test_timestamp & 0xffff);
	
	expect_rtc_value_read();
	
	mcutl::datetime::time_value result;
	mcutl::rtc::get_date_time(result);
	EXPECT_EQ(result.second, mcutl::datetime::time_from_timestamp(test_timestamp).second);
	EXPECT_EQ(result.minute, mcutl::datetime::time_from_timestamp(test_timestamp).minute);
	EXPECT_EQ(result.hour, mcutl::datetime::time_from_timestamp(test_timestamp).hour);
}

TEST_F(rtc_strict_test_fixture, GetDateTimeWithTimestampTest)
{
	memory().set(addr(&RTC->CNTH), test_timestamp >> 16);
	memory().set(addr(&RTC->CNTL), test_timestamp & 0xffff);
	
	expect_rtc_value_read();
	
	mcutl::datetime::date_time_with_timestamp result;
	mcutl::rtc::get_date_time(result);
	EXPECT_EQ(result.timestamp, test_timestamp);
	EXPECT_EQ(mcutl::datetime::timestamp_from_date_time(result), test_timestamp);
}

TEST_F(rtc_strict_test_fixture, GetDateTimeTest)
{
	memory().set(addr(&RTC->CNTH), test_timestamp >> 16);
	memory().set(addr(&RTC->CNTL), test_timestamp & 0xffff);
	
	expect_rtc_value_read();
	
	mcutl::datetime::date_time result;
	mcutl::rtc::get_date_time(result);
	EXPECT_EQ(mcutl::datetime::timestamp_from_date_time(result), test_timestamp);
}

TEST_F(rtc_strict_test_fixture, SetTimestampTest)
{
	expect_set_rtc_value();
	mcutl::rtc::set_date_time(test_timestamp);
}

TEST_F(rtc_strict_test_fixture, SetDateTimeTest1)
{
	expect_set_rtc_value();
	mcutl::rtc::set_date_time(mcutl::datetime::date_time_from_timestamp(test_timestamp));
}

TEST_F(rtc_strict_test_fixture, SetDateTimeTest2)
{
	expect_set_rtc_value();
	mcutl::datetime::date_time_with_timestamp result {};
	static_cast<mcutl::datetime::date_time&>(result)
		= mcutl::datetime::date_time_from_timestamp(test_timestamp);
	mcutl::rtc::set_date_time(result);
}

TEST_F(rtc_strict_test_fixture, DisableAlarmTest)
{
	memory().set(addr(&RTC->CRH), 0xffffffffu);
	
	::testing::InSequence s;
	
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(false);
	
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	EXPECT_CALL(memory(), write(addr(&RTC->CRH), 0xffffffffu & ~RTC_CRH_ALRIE));
	EXPECT_CALL(memory(), write(addr(&RTC->CRL), RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF));
	
	expect_disable_rtc_config(false);
	expect_disable_backup_writes(true, false);
	
	EXPECT_CALL(memory(), write(addr(&EXTI->PR), 1 << mcutl::exti::rtc_alarm_line::value));
	
	mcutl::rtc::disable_alarm();
}

TEST_F(rtc_strict_test_fixture, SetAlarmTimestampTest)
{
	expect_set_alarm_value();
	mcutl::rtc::set_alarm(test_timestamp);
}

TEST_F(rtc_strict_test_fixture, SetAlarmDateTimeTest1)
{
	expect_set_alarm_value();
	auto result = mcutl::datetime::date_time_from_timestamp(test_timestamp);
	mcutl::rtc::set_alarm(result);
}

TEST_F(rtc_strict_test_fixture, SetAlarmDateTimeTest2)
{
	expect_set_alarm_value();
	mcutl::datetime::date_time_with_timestamp result {};
	static_cast<mcutl::datetime::date_time&>(result)
		= mcutl::datetime::date_time_from_timestamp(test_timestamp);
	mcutl::rtc::set_alarm(result);
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigComplexTest1)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_lsi();
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSI);
	expect_enable_rtc_config(true);
	expect_set_rtc_prescaler(40000);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSI | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::internal,
		mcutl::rtc::clock::one_second_prescaler,
		mcutl::rtc::enable<true>,
		mcutl::rtc::base_configuration_is_currently_present
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigComplexTest2)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_lse(RCC_BDCR_LSEON);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON);
	expect_enable_rtc_config(true);
	expect_set_rtc_prescaler(32767);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::external_crystal,
		mcutl::rtc::clock::one_second_prescaler,
		mcutl::rtc::base_configuration_is_currently_present
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigComplexTest3)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_lse(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
	expect_enable_rtc_config(true);
	expect_set_rtc_prescaler(32767);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::lse_bypass,
		mcutl::rtc::clock::one_second_prescaler,
		mcutl::rtc::base_configuration_is_currently_present,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::overflow>,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::second>
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigComplexTest4)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE);
	expect_enable_rtc_config(true);
	expect_set_crh(RTC_CRH_ALRIE | RTC_CRH_OWIE);
	expect_set_rtc_prescaler(12345);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, true);
	memory().set(addr(&RCC->BDCR), 0);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::hse_div_128,
		mcutl::rtc::clock::prescaler<12345>,
		mcutl::rtc::enable<true>,
		mcutl::rtc::base_configuration_is_currently_present,
		mcutl::rtc::interrupt::external_alarm,
		mcutl::rtc::interrupt::overflow
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigAndInterruptsComplexTest1)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE);
	expect_enable_rtc_config(true);
	expect_set_crh(RTC_CRH_ALRIE | RTC_CRH_OWIE);
	expect_set_rtc_prescaler(12345);
	expect_disable_rtc_config(true);
	expect_enable_disable_alarm_line(false);
	expect_disable_interrupt(RTC_Alarm_IRQn);
	expect_enable_interrupt(RTC_IRQn);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, true);
	memory().set(addr(&RCC->BDCR), 0);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::hse_div_128,
		mcutl::rtc::clock::prescaler<12345>,
		mcutl::rtc::enable<true>,
		mcutl::rtc::base_configuration_is_currently_present,
		mcutl::rtc::interrupt::alarm,
		mcutl::rtc::interrupt::overflow,
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::rtc::interrupt::disable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigAndInterruptsComplexTest2)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE);
	expect_enable_rtc_config(true);
	expect_set_crh(RTC_CRH_ALRIE | RTC_CRH_OWIE);
	expect_set_rtc_prescaler(12345);
	expect_disable_rtc_config(true);
	expect_enable_interrupt(RTC_IRQn, 5, 4);
	expect_enable_disable_alarm_line(true);
	expect_enable_interrupt(RTC_Alarm_IRQn, 3, 2);
	expect_disable_backup_writes(true, true);
	memory().set(addr(&RCC->BDCR), 0);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::hse_div_128,
		mcutl::rtc::clock::prescaler<12345>,
		mcutl::rtc::base_configuration_is_currently_present,
		mcutl::interrupt::interrupt<mcutl::rtc::interrupt::external_alarm, 3, 2>,
		mcutl::interrupt::interrupt<mcutl::rtc::interrupt::overflow, 5, 4>,
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::rtc::interrupt::disable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureWithBaseConfigAndInterruptsComplexTest3)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE);
	expect_enable_rtc_config(true);
	expect_set_crh(RTC_CRH_ALRIE);
	expect_set_rtc_prescaler(12345);
	expect_disable_rtc_config(true);
	expect_disable_interrupt(RTC_IRQn);
	expect_enable_disable_alarm_line(true);
	expect_enable_interrupt(RTC_Alarm_IRQn, 3, 2);
	expect_disable_backup_writes(true, true);
	memory().set(addr(&RCC->BDCR), 0);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::hse_div_128,
		mcutl::rtc::clock::prescaler<12345>,
		mcutl::rtc::base_configuration_is_currently_present,
		mcutl::interrupt::interrupt<mcutl::rtc::interrupt::external_alarm, 3, 2>,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>,
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::rtc::interrupt::disable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureComplexTest1)
{
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCSEL_LSI);
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	EXPECT_CALL(memory(), read(addr(&RCC->BDCR)));
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSI);
	expect_enable_rtc_config(true);
	expect_set_crh(0);
	expect_set_rtc_prescaler(40000);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSI | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::internal,
		mcutl::rtc::clock::one_second_prescaler,
		mcutl::rtc::enable<true>
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureComplexTest2)
{
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCEN | RCC_BDCR_LSEON);
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	EXPECT_CALL(memory(), read(addr(&RCC->BDCR)));
	expect_disable_lse(0);
	expect_enable_lse(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
	expect_enable_rtc_config(true);
	expect_set_crh(0);
	expect_set_rtc_prescaler(32767);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSEBYP | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::lse_bypass,
		mcutl::rtc::clock::one_second_prescaler,
		mcutl::rtc::enable<true>
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureComplexTest3)
{
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCEN);
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	EXPECT_CALL(memory(), read(addr(&RCC->BDCR)));
	expect_disable_lse(0);
	expect_enable_lsi();
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSI);
	expect_enable_rtc_config(true);
	expect_set_crh(0);
	expect_set_rtc_prescaler(123);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSI | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::lsi,
		mcutl::rtc::clock::prescaler<123>,
		mcutl::rtc::enable<true>
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureComplexTest4)
{
	memory().set(addr(&RCC->BDCR), RCC_BDCR_LSEON | RCC_BDCR_LSERDY);
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	EXPECT_CALL(memory(), read(addr(&RCC->BDCR)));
	expect_enable_lse(RCC_BDCR_LSEON | RCC_BDCR_LSERDY);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON);
	expect_enable_rtc_config(true);
	expect_set_crh(0);
	expect_set_rtc_prescaler(123);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::configure<
		mcutl::rtc::clock::lse_crystal,
		mcutl::rtc::clock::prescaler<123>,
		mcutl::rtc::enable<true>
	>();
}

TEST_F(rtc_strict_test_fixture, ConfigureComplexTest5)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	EXPECT_CALL(memory(), read(addr(&RCC->BDCR)));
	expect_reset_backup_domain(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE);
	expect_enable_rtc_config(true);
	expect_set_crh(0);
	expect_set_rtc_prescaler(123);
	expect_disable_rtc_config(true);
	expect_set_bdcr(RCC_BDCR_RTCSEL_HSE | RCC_BDCR_RTCEN);
	expect_disable_backup_writes(true, true);
	
	memory().set(addr(&RCC->BDCR), RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY);
	mcutl::rtc::configure<
		mcutl::rtc::clock::hse_div_128,
		mcutl::rtc::clock::prescaler<123>,
		mcutl::rtc::enable<true>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureEmptyTest)
{
	mcutl::rtc::reconfigure();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest1)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	EXPECT_CALL(memory(), read(addr(&RCC->BDCR)));
	expect_set_bdcr(RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	mcutl::rtc::reconfigure<
		mcutl::rtc::enable<false>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest2)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	expect_set_rtc_prescaler(123);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	mcutl::rtc::reconfigure<
		mcutl::rtc::clock::prescaler<123>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest3)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_SECIE | RTC_CRH_OWIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_SECIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::overflow
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest4)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_ALRIE | RTC_CRH_OWIE);
	mcutl::rtc::reconfigure<
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest5)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE | RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_OWIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::external_alarm
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest6)
{
	enable_interrupt(0);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_OWIE | RTC_CRH_ALRIE);
	mcutl::rtc::reconfigure<
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest7)
{
	enable_interrupt(0);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_OWIE | RTC_CRH_ALRIE);
	mcutl::rtc::reconfigure<
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest8)
{
	enable_interrupt(RTC_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE | RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_OWIE | RTC_CRH_ALRIE);
	mcutl::rtc::reconfigure<
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest9)
{
	enable_interrupt(RTC_Alarm_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE | RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_OWIE | RTC_CRH_ALRIE);
	mcutl::rtc::reconfigure<
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureComplexTest10)
{
	enable_interrupt(RTC_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_OWIE);
	expect_disable_rtc_config(true);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RCC->BDCR),
		RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN);
	memory().set(addr(&RTC->CRH), RTC_CRH_OWIE);
	mcutl::rtc::reconfigure<
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest1)
{
	memory().allow_reads(addr(&RTC->CRH));
	
	expect_disable_interrupt(RTC_IRQn);
	expect_enable_disable_alarm_line(false);
	expect_disable_interrupt(RTC_Alarm_IRQn);
	
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::disable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest2)
{
	memory().allow_reads(addr(&RTC->CRH));
	memory().set(addr(&RTC->CRH), RTC_CRH_ALRIE);
	
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::disable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest3)
{
	enable_interrupt(RTC_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_SECIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_disable_alarm_line(false);
	expect_disable_interrupt(RTC_Alarm_IRQn);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RTC->CRH), RTC_CRH_SECIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::disable_controller_interrupts,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest4)
{
	memory().allow_reads(addr(&RTC->CRH));
	expect_enable_interrupt(RTC_IRQn);
	
	memory().set(addr(&RTC->CRH), RTC_CRH_SECIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest5)
{
	memory().allow_reads(addr(&RTC->CRH));
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest6)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_disable_alarm_line(true);
	expect_enable_interrupt(RTC_Alarm_IRQn);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::rtc::interrupt::external_alarm
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest7)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_interrupt(RTC_IRQn);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::rtc::interrupt::alarm
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest8)
{
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_interrupt(RTC_IRQn);
	expect_enable_disable_alarm_line(true);
	expect_enable_interrupt(RTC_Alarm_IRQn);
	expect_disable_backup_writes(true, false);
	
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::rtc::interrupt::alarm,
		mcutl::rtc::interrupt::external_alarm
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest9)
{
	memory().allow_reads(addr(&RTC->CRH));
	expect_enable_interrupt(RTC_IRQn);
	
	memory().set(addr(&RTC->CRH), RTC_CRH_ALRIE);
	
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest10)
{
	enable_interrupt(RTC_Alarm_IRQn);
	enable_interrupt(RTC_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_SECIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_disable_alarm_line(false);
	expect_disable_interrupt(RTC_Alarm_IRQn);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RTC->CRH), RTC_CRH_ALRIE | RTC_CRH_SECIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::disable_controller_interrupts,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest11)
{
	enable_interrupt(RTC_Alarm_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_SECIE | RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_disable_alarm_line(false);
	expect_disable_interrupt(RTC_Alarm_IRQn);
	expect_enable_interrupt(RTC_IRQn);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RTC->CRH), RTC_CRH_ALRIE | RTC_CRH_SECIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::disable_controller_interrupts,
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::interrupt::disabled<mcutl::rtc::interrupt::external_alarm>,
		mcutl::rtc::interrupt::alarm
	>();
}

TEST_F(rtc_strict_test_fixture, ReConfigureInterruptsTest12)
{
	enable_interrupt(RTC_IRQn);
	
	::testing::InSequence s;
	expect_enable_backup_writes(false);
	expect_enable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_set_crh(RTC_CRH_SECIE | RTC_CRH_ALRIE);
	expect_disable_rtc_config(true);
	EXPECT_CALL(memory(), read(addr(&RTC->CRH)));
	expect_enable_interrupt(RTC_IRQn);
	expect_enable_disable_alarm_line(true);
	expect_enable_interrupt(RTC_Alarm_IRQn, 2, 3);
	expect_disable_backup_writes(true, false);
	
	memory().set(addr(&RTC->CRH), RTC_CRH_SECIE);
	mcutl::rtc::reconfigure<
		mcutl::rtc::interrupt::enable_controller_interrupts,
		mcutl::interrupt::interrupt<mcutl::rtc::interrupt::external_alarm, 2, 3>,
		mcutl::rtc::interrupt::alarm
	>();
}

TEST_F(rtc_strict_test_fixture, GetPendingFlagsTest)
{
	expect_single_crl_read(RTC_CRL_ALRF | RTC_CRL_SECF);
	EXPECT_EQ(mcutl::rtc::get_pending_flags<mcutl::rtc::interrupt::second>(),
		mcutl::rtc::pending_flags_v<mcutl::rtc::interrupt::second>);
	
	expect_single_crl_read(RTC_CRL_ALRF | RTC_CRL_SECF);
	EXPECT_EQ(mcutl::rtc::get_pending_flags<mcutl::rtc::interrupt::overflow>(), 0u);
	
	expect_single_crl_read(RTC_CRL_ALRF | RTC_CRL_SECF);
	EXPECT_EQ(mcutl::rtc::get_pending_flags<mcutl::rtc::interrupt::alarm>(),
		mcutl::rtc::pending_flags_v<mcutl::rtc::interrupt::alarm>);
	
	expect_single_crl_read(RTC_CRL_ALRF | RTC_CRL_SECF);
	EXPECT_EQ((mcutl::rtc::get_pending_flags<
		mcutl::rtc::interrupt::alarm,
		mcutl::rtc::interrupt::overflow,
		mcutl::rtc::interrupt::second>()),
		(mcutl::rtc::pending_flags_v<
		mcutl::rtc::interrupt::alarm,
		mcutl::rtc::interrupt::second>));
}
