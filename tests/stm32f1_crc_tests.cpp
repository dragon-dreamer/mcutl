#define STM32F103xE
#define STM32F1

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include "mcutl/crc/crc.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class crc_strict_test_fixture
	: public mcutl::tests::memory::strict_test_fixture_base
{
public:
	void expect_crc_calculation()
	{
		EXPECT_CALL(memory(), write(addr(&CRC->CR), CRC_CR_RESET));
		memory().allow_reads(addr(&CRC->DR));
		EXPECT_CALL(memory(), write(addr(&CRC->DR),::testing::_))
			.WillRepeatedly([this] (auto, auto value) { fake_crc_ += value; });
		ON_CALL(memory(), read(addr(&CRC->DR)))
			.WillByDefault([this] (auto) { return fake_crc_; });
	}
	
	void reset_crc() noexcept
	{
		fake_crc_ = 0;
		EXPECT_CALL(memory(), write(addr(&CRC->CR), CRC_CR_RESET));
	}
	
	static mcutl::crc::crc_input_type calc_fake_crc(const uint8_t* ptr,
		size_t size) noexcept
	{
		mcutl::crc::crc_input_type result = 0;
		mcutl::crc::crc_input_type value = 0;
		
		while (size >= sizeof(mcutl::crc::crc_input_type))
		{
			std::memcpy(&value, ptr, sizeof(value));
			ptr += sizeof(mcutl::crc::crc_input_type);
			size -= sizeof(mcutl::crc::crc_input_type);
			result += value;
		}
		
		if (size)
		{
			value = 0;
			std::memcpy(&value, ptr, size);
			result += value;
		}
		
		return result;
	}
	
private:
	uint32_t fake_crc_ = 0;
};

TEST_F(crc_strict_test_fixture, CrcResetTest)
{
	EXPECT_CALL(memory(), write(addr(&CRC->CR), CRC_CR_RESET));
	mcutl::crc::reset();
}

TEST_F(crc_strict_test_fixture, CrcAddDataTest)
{
	constexpr mcutl::crc::crc_input_type data = 0x12345678;
	EXPECT_CALL(memory(), write(addr(&CRC->DR), data));
	mcutl::crc::add_data(data);
}

TEST_F(crc_strict_test_fixture, GetCrcTest)
{
	constexpr mcutl::crc::crc_input_type data = 0x12345678;
	memory().set(addr(&CRC->DR), data);
	memory().allow_reads(addr(&CRC->DR));
	
	EXPECT_EQ(mcutl::crc::get_crc(), data);
}

TEST_F(crc_strict_test_fixture, CalculateCrcTest)
{
	expect_crc_calculation();
	mcutl::crc::crc_input_type source[] { 0x11, 0x22, 0x33, 0x44, 0x55 };
	EXPECT_EQ(mcutl::crc::calculate_crc(source, sizeof(source) / sizeof(source[0])),
		calc_fake_crc(reinterpret_cast<const uint8_t*>(source), sizeof(source)));
	
	reset_crc();
	EXPECT_EQ(mcutl::crc::calculate_crc(source),
		calc_fake_crc(reinterpret_cast<const uint8_t*>(source), sizeof(source)));
}

TEST_F(crc_strict_test_fixture, CalculateCrcByteTest1)
{
	expect_crc_calculation();
	uint8_t bytes[] { 0x11, 0x22, 0x33, 0x44,
			0x55, 0x66, 0x77, 0x88 };
	EXPECT_EQ(mcutl::crc::calculate_crc(bytes, sizeof(bytes) / sizeof(bytes[0])),
		calc_fake_crc(bytes, sizeof(bytes) / sizeof(bytes[0])));
	
	reset_crc();
	EXPECT_EQ(mcutl::crc::calculate_crc(bytes), calc_fake_crc(bytes, sizeof(bytes)));
}

TEST_F(crc_strict_test_fixture, CalculateCrcByteTest2)
{
	expect_crc_calculation();
	uint8_t bytes[] { 0x11, 0x22, 0x33, 0x44,
			0x55, 0x66, 0x77 };
	EXPECT_EQ(mcutl::crc::calculate_crc(bytes, sizeof(bytes) / sizeof(bytes[0])),
		calc_fake_crc(bytes, sizeof(bytes) / sizeof(bytes[0])));
	
	reset_crc();
	EXPECT_EQ(mcutl::crc::calculate_crc(bytes), calc_fake_crc(bytes, sizeof(bytes)));
}

struct test_crc_struct
{
	uint32_t val1 = 1234567;
	uint8_t val2 = 0x56;
};

TEST_F(crc_strict_test_fixture, CalculateCrcStructTest)
{
	expect_crc_calculation();
	test_crc_struct instance {};
	EXPECT_EQ(mcutl::crc::calculate_crc(instance),
		calc_fake_crc(reinterpret_cast<const uint8_t*>(&instance), sizeof(instance)));
}
