#define STM32F103xE
#define STM32F1

#include "mcutl/instruction/instruction.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using instruction_test_fixture = mcutl::tests::instruction::test_fixture_base;

TEST_F(instruction_test_fixture, InstructionTest)
{
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::wfe>(), ::testing::IsEmpty()));
	mcutl::instruction::execute<mcutl::device::instruction::type::wfe>();
}

namespace mcutl::device::instruction::type
{

struct fake {};

} //namespace mcutl::device::instruction::type

namespace mcutl::instruction::detail
{

template<>
struct return_type<mcutl::device::instruction::type::fake> { using type = unsigned int; };

} //namespace mcutl::instruction::detail

TEST_F(instruction_test_fixture, InstructionReturnValueTest)
{
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::fake>(), ::testing::IsEmpty())).
		WillOnce(::testing::Return(12345u));
	
	EXPECT_EQ(mcutl::instruction::execute<mcutl::device::instruction::type::fake>(), 12345u);
}

TEST_F(instruction_test_fixture, InstructionArgsTest)
{
	constexpr const char* str = "test";
	
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::wfe>(),
		mcutl::tests::instruction::InstructionArgsEqual(123, str)));
	mcutl::instruction::execute<mcutl::device::instruction::type::wfe>(123, str);
}
