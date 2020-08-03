#define STM32F103xE
#define STM32F1

#include <stddef.h>
#include <type_traits>

#include "mcutl/memory/volatile_memory.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

struct test_register
{
	uint32_t reg1 = 0;
};

struct test_register_array
{
	uint32_t reg_array[2] {};
};

class memory_layer_test_fixture : public mcutl::tests::memory::strict_test_fixture_base
{
public:
	static constexpr uint32_t test_address = 0x12345678u;
	static constexpr uint32_t test_array_address = 0x90abcdefu;
	static constexpr uint32_t reg_address = test_address + offsetof(test_register, reg1);
	static constexpr uint32_t reg_array_address = test_array_address + offsetof(test_register_array, reg_array);
	
	void expect_set_register_bits(uint32_t test_mask, uint32_t test_bits)
	{
		memory().set(reg_address, 0b11110000111100001111000011110000u);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(reg_address));
		EXPECT_CALL(memory(),
			write(reg_address, (memory().get(reg_address) & ~test_mask) | test_bits));
	}
	
	void expect_set_register_value(uint32_t value)
	{
		EXPECT_CALL(memory(), write(reg_address, value));
	}
	
	void expect_get_register_bits(uint32_t value)
	{
		memory().set(reg_address, value);
		EXPECT_CALL(memory(), read(reg_address));
	}
	
	void expect_set_register_array_bits(uint32_t test_mask, uint32_t test_bits, uint32_t index)
	{
		uint32_t field_address = reg_array_address + sizeof(uint32_t) * index;
		memory().set(field_address, 0b11110000111100001111000011110000u);
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), read(field_address));
		EXPECT_CALL(memory(),
			write(field_address, (memory().get(field_address) & ~test_mask) | test_bits));
	}
	
	void expect_set_register_array_value(uint32_t value, uint32_t index)
	{
		uint32_t field_address = reg_array_address + sizeof(uint32_t) * index;
		EXPECT_CALL(memory(), write(field_address, value));
	}
	
	void expect_get_register_array_bits(uint32_t value, uint32_t index)
	{
		uint32_t field_address = reg_array_address + sizeof(uint32_t) * index;
		memory().set(field_address, value);
		EXPECT_CALL(memory(), read(field_address));
	}
	
	test_register* test_reg_ptr = reinterpret_cast<test_register*>(test_address);
	test_register_array* test_reg_array_ptr = reinterpret_cast<test_register_array*>(test_array_address);
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
	EXPECT_EQ((mcutl::memory::get_register_bits<mask, &test_register::reg1, test_address>()), mask & bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<mask, &test_register::reg1>(test_reg_ptr)), mask & bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits(&test_register::reg1, test_reg_ptr)), bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<test_address>(&test_register::reg1)), bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<mask, test_address>(&test_register::reg1)), mask & bits);
	
	expect_get_register_bits(bits);
	EXPECT_EQ((mcutl::memory::get_register_bits<mask>(&test_register::reg1, test_reg_ptr)), mask & bits);
}

TEST_F(memory_layer_test_fixture, MemoryLayerGetRegisterFlag)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t true_mask = 0b1000011111000011u;
	constexpr uint32_t false_mask = 0b0101010101010101u;
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<true_mask, &test_register::reg1, test_address>()));
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<true_mask, &test_register::reg1>(test_reg_ptr)));
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<true_mask, test_address>(&test_register::reg1)));
	
	expect_get_register_bits(bits);
	EXPECT_TRUE((mcutl::memory::get_register_flag<true_mask>(&test_register::reg1, test_reg_ptr)));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<false_mask, &test_register::reg1, test_address>()));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<false_mask, &test_register::reg1>(test_reg_ptr)));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<false_mask, test_address>(&test_register::reg1)));
	
	expect_get_register_bits(bits);
	EXPECT_FALSE((mcutl::memory::get_register_flag<false_mask>(&test_register::reg1, test_reg_ptr)));
}

TEST_F(memory_layer_test_fixture, MemoryLayerSetRegisterArrayBits)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t mask = 0b1111111111111111u;
	constexpr uint32_t array_index = 1;
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, bits,
		&test_register_array::reg_array, array_index>(test_reg_array_ptr);
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, &test_register_array::reg_array,
		array_index>(test_reg_array_ptr, bits);
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, bits, &test_register_array::reg_array,
		array_index, test_array_address>();
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, &test_register_array::reg_array,
		array_index, test_array_address>(bits);
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, bits, array_index>(
		&test_register_array::reg_array, test_reg_array_ptr);
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, array_index>(
		&test_register_array::reg_array, test_reg_array_ptr, bits);
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, bits, array_index, test_array_address>(
		&test_register_array::reg_array);
	
	expect_set_register_array_bits(mask, bits, array_index);
	mcutl::memory::set_register_array_bits<mask, array_index, test_array_address>(
		&test_register_array::reg_array, bits);
}

TEST_F(memory_layer_test_fixture, MemoryLayerSetRegisterArrayValue)
{
	constexpr uint32_t value = 0b1010101010101010u;
	constexpr uint32_t array_index = 1;
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<value, &test_register_array::reg_array,
		array_index>(test_reg_array_ptr);
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<&test_register_array::reg_array,
		array_index>(test_reg_array_ptr, value);
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<&test_register_array::reg_array,
		array_index, test_array_address>(value);
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<value, &test_register_array::reg_array,
		array_index, test_array_address>();
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<value, array_index>(
		&test_register_array::reg_array, test_reg_array_ptr);
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<array_index>(
		&test_register_array::reg_array, test_reg_array_ptr, value);
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<array_index, test_array_address>(
		&test_register_array::reg_array, value);
	
	expect_set_register_array_value(value, array_index);
	mcutl::memory::set_register_array_value<value, array_index, test_array_address>(
		&test_register_array::reg_array);
}

TEST_F(memory_layer_test_fixture, MemoryLayerGetRegisterArrayBits)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t mask = 0b1000011111000011u;
	constexpr uint32_t array_index = 1;
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<&test_register_array::reg_array, array_index>(test_reg_array_ptr)), bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<&test_register_array::reg_array, array_index, test_array_address>()), bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<mask, &test_register_array::reg_array, array_index, test_array_address>()), mask & bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<mask, &test_register_array::reg_array, array_index>(test_reg_array_ptr)), mask & bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<array_index>(&test_register_array::reg_array, test_reg_array_ptr)), bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<array_index, test_array_address>(&test_register_array::reg_array)), bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<mask, array_index, test_array_address>(&test_register_array::reg_array)), mask & bits);
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_EQ((mcutl::memory::get_register_array_bits<mask, array_index>(&test_register_array::reg_array, test_reg_array_ptr)), mask & bits);
}

TEST_F(memory_layer_test_fixture, MemoryLayerGetRegisterArrayFlag)
{
	constexpr uint32_t bits = 0b1010101010101010u;
	constexpr uint32_t true_mask = 0b1000011111000011u;
	constexpr uint32_t false_mask = 0b0101010101010101u;
	constexpr uint32_t array_index = 1;
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_TRUE((mcutl::memory::get_register_array_flag<true_mask, &test_register_array::reg_array, array_index, test_array_address>()));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_TRUE((mcutl::memory::get_register_array_flag<true_mask, &test_register_array::reg_array, array_index>(test_reg_array_ptr)));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_TRUE((mcutl::memory::get_register_array_flag<true_mask, array_index, test_array_address>(&test_register_array::reg_array)));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_TRUE((mcutl::memory::get_register_array_flag<true_mask, array_index>(&test_register_array::reg_array, test_reg_array_ptr)));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_FALSE((mcutl::memory::get_register_array_flag<false_mask, &test_register_array::reg_array, array_index, test_array_address>()));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_FALSE((mcutl::memory::get_register_array_flag<false_mask, &test_register_array::reg_array, array_index>(test_reg_array_ptr)));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_FALSE((mcutl::memory::get_register_array_flag<false_mask, array_index, test_array_address>(&test_register_array::reg_array)));
	
	expect_get_register_array_bits(bits, array_index);
	EXPECT_FALSE((mcutl::memory::get_register_array_flag<false_mask, array_index>(&test_register_array::reg_array, test_reg_array_ptr)));
}
