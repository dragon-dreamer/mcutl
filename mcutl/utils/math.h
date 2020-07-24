#pragma once

#include <limits.h>
#include <stdint.h>

namespace mcutl::math
{

[[nodiscard]] constexpr uint32_t set_bits_count(uint32_t value) noexcept
{
	value = value - ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	return (((value + (value >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

template<uint32_t Value>
[[nodiscard]] constexpr uint8_t first_set_bit_position() noexcept
{
	uint8_t i = 0;
	for (; i != CHAR_BIT * sizeof(Value); ++i)
	{
		if (Value & (1 << i))
			return i;
	}
	
	return i;
}

} //namespace mcutl::math