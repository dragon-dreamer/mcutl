#define STM32F107xC
#define STM32F1

#include <type_traits>

#include "mcutl/low_power/low_power.h"
#include "mcutl/periph/periph.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class low_power_strict_test_fixture
	: public mcutl::tests::mcu::strict_test_fixture_base
{
public:
	void expect_sleep(uint32_t pwr_cr_mask, uint32_t pwr_cr_value,
		uint32_t scb_scr_value, uint32_t dbgmcu_cr_flag, bool sleep_on_exit, bool wait_event)
	{
		memory().allow_reads(addr(&PWR->CR));
		memory().allow_reads(addr(&SCB->SCR));
		if (dbgmcu_cr_flag)
			memory().allow_reads(addr(&DBGMCU->CR));
		
		::testing::InSequence s;
		
		memory().set(addr(&PWR->CR), pwr_cr_initial_value);
		EXPECT_CALL(memory(), write(addr(&PWR->CR),
			(pwr_cr_initial_value & ~pwr_cr_mask) | pwr_cr_value));
		
		if (dbgmcu_cr_flag)
		{
			memory().set(addr(&DBGMCU->CR), dbgmcu_cr_initial_value);
			EXPECT_CALL(memory(), write(addr(&DBGMCU->CR),
				dbgmcu_cr_initial_value | dbgmcu_cr_flag));
		}
		
		memory().set(addr(&SCB->SCR), scb_scr_initial_value);
		EXPECT_CALL(memory(), write(addr(&SCB->SCR),
			(scb_scr_initial_value & ~scb_scr_mask) | scb_scr_value));
		
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dsb>(),
			::testing::IsEmpty()));
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
			::testing::IsEmpty()));
		
		if (sleep_on_exit)
			return;
		
		if (wait_event)
		{
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::wfe>(),
				::testing::IsEmpty()));
		}
		else
		{
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::wfi>(),
				::testing::IsEmpty()));
		}
			
		if (scb_scr_value)
		{
			EXPECT_CALL(memory(), write(addr(&SCB->SCR),
				scb_scr_initial_value & ~scb_scr_mask));
		}
			
		if (dbgmcu_cr_flag)
		{
			EXPECT_CALL(memory(), write(addr(&DBGMCU->CR),
				dbgmcu_cr_initial_value & ~dbgmcu_cr_flag));
		}
	}
	
	void expect_reset_sleep_flags(uint32_t scb_scr_value, uint32_t dbgmcu_cr_flag)
	{
		static constexpr uint32_t dbgmcu_cr_all_ones = 0xffffffffu;
		
		if (scb_scr_value)
		{
			memory().allow_reads(addr(&SCB->SCR));
			memory().set(addr(&SCB->SCR), scb_scr_initial_value | scb_scr_mask);
		}
		if (dbgmcu_cr_flag)
		{
			memory().allow_reads(addr(&DBGMCU->CR));
			memory().set(addr(&DBGMCU->CR), dbgmcu_cr_all_ones);
		}
		
		if (scb_scr_value)
		{
			EXPECT_CALL(memory(), write(addr(&SCB->SCR),
				(scb_scr_initial_value | scb_scr_mask) & ~scb_scr_value));
		}
			
		if (dbgmcu_cr_flag)
		{
			EXPECT_CALL(memory(), write(addr(&DBGMCU->CR),
				dbgmcu_cr_all_ones & ~dbgmcu_cr_flag));
		}
	}
	
private:
	static constexpr uint32_t pwr_cr_initial_value = 0x12345678u;
	static constexpr uint32_t dbgmcu_cr_initial_value = 0xffffffffu &
		~(DBGMCU_CR_DBG_STOP_Msk | DBGMCU_CR_DBG_SLEEP_Msk | DBGMCU_CR_DBG_STANDBY_Msk);
	static constexpr uint32_t scb_scr_mask = SCB_SCR_SLEEPDEEP_Msk
		| SCB_SCR_SLEEPONEXIT_Msk | SCB_SCR_SEVONPEND_Msk;
	static constexpr uint32_t scb_scr_initial_value = 0x89abcdefu;
};

TEST_F(low_power_strict_test_fixture, SleepWfiTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, 0, 0, false, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_interrupt>();
}

TEST_F(low_power_strict_test_fixture, StopWfiTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, SCB_SCR_SLEEPDEEP_Msk, 0, false, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::stop,
		mcutl::low_power::wakeup_mode::wait_for_interrupt>();
}

TEST_F(low_power_strict_test_fixture, StandbyWfiTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk | PWR_CR_CWUF_Msk,
		PWR_CR_PDDS | PWR_CR_CWUF, SCB_SCR_SLEEPDEEP_Msk, 0, false, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::standby,
		mcutl::low_power::wakeup_mode::wait_for_interrupt>();
}

TEST_F(low_power_strict_test_fixture, SleepWfeTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, 0, 0, false, true);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_event>();
}

TEST_F(low_power_strict_test_fixture, StopWfeTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, SCB_SCR_SLEEPDEEP_Msk, 0, false, true);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::stop,
		mcutl::low_power::wakeup_mode::wait_for_event>();
}

TEST_F(low_power_strict_test_fixture, StandbyWfeTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk | PWR_CR_CWUF_Msk,
		PWR_CR_PDDS | PWR_CR_CWUF, SCB_SCR_SLEEPDEEP_Msk, 0, false, true);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::standby,
		mcutl::low_power::wakeup_mode::wait_for_event>();
}

TEST_F(low_power_strict_test_fixture, SleepWfeWithPendingInterruptsTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, SCB_SCR_SEVONPEND_Msk, 0, false, true);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_event,
		mcutl::low_power::options::event_on_pending_interrupt>();
}

TEST_F(low_power_strict_test_fixture, SleepWfeWithPendingInterruptsAndDebugSupportTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, SCB_SCR_SEVONPEND_Msk,
		DBGMCU_CR_DBG_SLEEP, false, true);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_event,
		mcutl::low_power::options::debug_support,
		mcutl::low_power::options::event_on_pending_interrupt>();
}

TEST_F(low_power_strict_test_fixture, StopWfiWithLowPowerVoltageRegAndDebugSupportTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, PWR_CR_LPDS, SCB_SCR_SLEEPDEEP_Msk,
		DBGMCU_CR_DBG_STOP, false, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::stop,
		mcutl::low_power::wakeup_mode::wait_for_interrupt,
		mcutl::low_power::options::debug_support,
		mcutl::low_power::options::voltage_regulator_low_power_stop>();
}

TEST_F(low_power_strict_test_fixture, StandbyWithDebugSupportTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, PWR_CR_PDDS | PWR_CR_CWUF,
		SCB_SCR_SLEEPDEEP_Msk, DBGMCU_CR_DBG_STANDBY, false, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::standby,
		mcutl::low_power::wakeup_mode::wait_for_interrupt,
		mcutl::low_power::options::debug_support>();
}

TEST_F(low_power_strict_test_fixture, SleepOnExitWfiTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0,
		SCB_SCR_SLEEPONEXIT_Msk, 0, true, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_interrupt,
		mcutl::low_power::options::sleep_on_exit>();
}

TEST_F(low_power_strict_test_fixture, SleepOnExitWfeAndDebugSupportTest)
{
	expect_sleep(PWR_CR_PDDS_Msk | PWR_CR_LPDS_Msk, 0, SCB_SCR_SLEEPONEXIT_Msk,
		DBGMCU_CR_DBG_SLEEP, true, false);
	mcutl::low_power::sleep<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_interrupt,
		mcutl::low_power::options::debug_support,
		mcutl::low_power::options::sleep_on_exit>();
}

TEST_F(low_power_strict_test_fixture, ResetSleepFlagsNoOpsTest)
{
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_interrupt>();
	
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_event>();
}

TEST_F(low_power_strict_test_fixture, ResetStopFlagsTest)
{
	expect_reset_sleep_flags(SCB_SCR_SLEEPDEEP_Msk, 0);
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::stop,
		mcutl::low_power::wakeup_mode::wait_for_interrupt>();
}

TEST_F(low_power_strict_test_fixture, ResetStandbyFlagsTest)
{
	expect_reset_sleep_flags(SCB_SCR_SLEEPDEEP_Msk, 0);
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::standby,
		mcutl::low_power::wakeup_mode::wait_for_interrupt>();
}

TEST_F(low_power_strict_test_fixture, ResetWfeWithDebugSupportAndEventOnPendingInterruptTest)
{
	expect_reset_sleep_flags(SCB_SCR_SEVONPEND_Msk, DBGMCU_CR_DBG_SLEEP);
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_event,
		mcutl::low_power::options::event_on_pending_interrupt,
		mcutl::low_power::options::debug_support>();
}

TEST_F(low_power_strict_test_fixture, ResetWfiWithSleepOnExitAndDebugModeTest)
{
	expect_reset_sleep_flags(SCB_SCR_SLEEPONEXIT_Msk, DBGMCU_CR_DBG_SLEEP);
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::sleep,
		mcutl::low_power::wakeup_mode::wait_for_interrupt,
		mcutl::low_power::options::sleep_on_exit,
		mcutl::low_power::options::debug_support>();
}

TEST_F(low_power_strict_test_fixture, ResetStopFlagsWithDebugSupportTest)
{
	expect_reset_sleep_flags(SCB_SCR_SLEEPDEEP_Msk, DBGMCU_CR_DBG_STOP);
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::stop,
		mcutl::low_power::wakeup_mode::wait_for_event,
		mcutl::low_power::options::debug_support>();
}

TEST_F(low_power_strict_test_fixture, ResetStandbyFlagsWithDebugSupportTest)
{
	expect_reset_sleep_flags(SCB_SCR_SLEEPDEEP_Msk, DBGMCU_CR_DBG_STANDBY);
	mcutl::low_power::reset_sleep_flags<mcutl::low_power::sleep_mode::standby,
		mcutl::low_power::wakeup_mode::wait_for_event,
		mcutl::low_power::options::debug_support>();
}

TEST_F(low_power_strict_test_fixture, TraitsTest)
{
	EXPECT_TRUE(mcutl::low_power::supports_wait_for_event);
	EXPECT_TRUE(mcutl::low_power::supports_wait_for_interrupt);
	EXPECT_TRUE(mcutl::low_power::supports_core_stop);
	EXPECT_FALSE(mcutl::low_power::supports_adc_noise_reduction);
	EXPECT_TRUE(mcutl::low_power::supports_stop_all_clocks);
	EXPECT_TRUE(mcutl::low_power::supports_power_off);
	EXPECT_TRUE((std::is_same_v<mcutl::low_power::peripheral_type, mcutl::periph::pwr>));
}
