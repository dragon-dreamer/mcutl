#define STM32F103xE
#define STM32F1

#include "mcutl/instruction/instruction.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using mcu_test_fixture = mcutl::tests::mcu::test_fixture_base;

struct test_register
{
	uint32_t reg1 = 0;
};

TEST_F(mcu_test_fixture, McuTest)
{
	::testing::InSequence s;
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::wfe>(), ::testing::IsEmpty()));
	EXPECT_CALL(memory(), write(0x12345, 0x67890));
	
	mcutl::instruction::execute<mcutl::device::instruction::type::wfe>();
	mcutl::memory::set_register_value<0x67890, &test_register::reg1, 0x12345>();
}
