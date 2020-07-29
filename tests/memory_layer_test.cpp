#define STM32F103xE
#define STM32F1

#include <type_traits>

#include "mcutl/memory/volatile_memory.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

struct test_register
{
	uint32_t reg1 = 0;
};

class memory_layer_test_fixture : public mcutl::tests::memory::strict_test_fixture_base
{
public:
	static constexpr uint32_t test_address = 0x12345678u;
	
	void expect_set_register_bits(uint32_t test_mask, uint32_t test_bits)
	{
		memory().set(test_address, 0b11110000111100001111000011110000u);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(test_address));
		EXPECT_CALL(memory(),
			write(test_address, (memory().get(test_address) & ~test_mask) | test_bits));
	}
	
	void expect_set_register_value(uint32_t value)
	{
		EXPECT_CALL(memory(), write(test_address, value));
	}
	
	void expect_get_register_bits(uint32_t value)
	{
		memory().set(test_address, value);
		EXPECT_CALL(memory(), read(test_address));
	}
	
	test_register* test_reg_ptr = reinterpret_cast<test_register*>(test_address);
};

TEST_F(memory_layer_test_fixture, MemoryLayerSetRegisterBits)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t mask = 0b1111111111111111u;
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, bits, &test_register::reg1>(test_reg_ptr);
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, &test_register::reg1>(test_reg_ptr, bits);
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, bits, &test_register::reg1, test_address>();
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, &test_register::reg1, test_address>(bits);
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, bits>(&test_register::reg1, test_reg_ptr);
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask>(&test_register::reg1, test_reg_ptr, bits);
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, bits, test_address>(&test_register::reg1);
	
	expect_set_register_bits(mask, bits);
	mcutl::memory::set_register_bits<mask, test_address>(&test_register::reg1, bits);
}

TEST_F(memory_layer_test_fixture, MemoryLayerSetRegisterValue)
{
	constexpr uint32_t value = 0b1010101010101010u;
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<value, &test_register::reg1>(test_reg_ptr);
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<&test_register::reg1>(test_reg_ptr, value);
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<&test_register::reg1, test_address>(value);
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<value, &test_register::reg1, test_address>();
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<value>(&test_register::reg1, test_reg_ptr);
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value(&test_register::reg1, test_reg_ptr, value);
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<test_address>(&test_register::reg1, value);
	
	expect_set_register_value(value);
	mcutl::memory::set_register_value<value, test_address>(&test_register::reg1);
}

TEST_F(memory_layer_test_fixture, MemoryLayerGetRegisterBits)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t mask = 0b1000011111000011u;
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<&test_register::reg1>(test_reg_ptr)), bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<&test_register::reg1, test_address>()), bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<&test_register::reg1, test_address, mask>()), mask & bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<mask, &test_register::reg1>(test_reg_ptr)), mask & bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits(&test_register::reg1, test_reg_ptr)), bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<test_address>(&test_register::reg1)), bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<test_address, mask>(&test_register::reg1)), mask & bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<mask>(&test_register::reg1, test_reg_ptr)), mask & bits);
}

TEST_F(memory_layer_test_fixture, MemoryLayerGetRegisterFlag)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t true_mask = 0b1000011111000011u;
	constexpr uint32_t false_mask = 0b0101010101010101u;
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<&test_register::reg1, test_address, true_mask>()));
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<true_mask, &test_register::reg1>(test_reg_ptr)));
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<test_address, true_mask>(&test_register::reg1)));
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<true_mask>(&test_register::reg1, test_reg_ptr)));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<&test_register::reg1, test_address, false_mask>()));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<false_mask, &test_register::reg1>(test_reg_ptr)));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<test_address, false_mask>(&test_register::reg1)));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<false_mask>(&test_register::reg1, test_reg_ptr)));
}
