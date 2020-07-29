#pragma once

#include "mcutl/tests/volatile_memory.h"
#include "mcutl/tests/instruction.h"

namespace mcutl::tests::mcu
{

class test_fixture_base :
	public instruction::test_fixture_base,
	public memory::test_fixture_base
{
public:
	virtual void SetUp() override
	{
		instruction::test_fixture_base::SetUp();
		memory::test_fixture_base::SetUp();
	}

	virtual void TearDown() override
	{
		memory::test_fixture_base::TearDown();
		instruction::test_fixture_base::TearDown();
	}
};

class strict_test_fixture_base :
	public instruction::test_fixture_base,
	public memory::strict_test_fixture_base
{
public:
	virtual void SetUp() override
	{
		instruction::test_fixture_base::SetUp();
		memory::strict_test_fixture_base::SetUp();
	}

	virtual void TearDown() override
	{
		memory::strict_test_fixture_base::TearDown();
		instruction::test_fixture_base::TearDown();
	}
};

} //namespace mcutl::tests::mcu
