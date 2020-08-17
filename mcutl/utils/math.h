#pragma once

#include <limits>
#include <limits.h>
#include <stdint.h>
#include <type_traits>

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

template<auto Value>
[[nodiscard]] constexpr bool is_power_of_2() noexcept
{
	return Value && (Value & (Value - 1)) == 0;
}

template<auto Value>
[[nodiscard]] constexpr uint32_t log2() noexcept
{
	uint32_t res {};
	auto value = Value;
	while (value >>= 1)
		++res;
	return res;
}

template<typename T, auto Val1, auto Val2>
[[nodiscard]] constexpr bool mul_overflows() noexcept
{
	constexpr auto val1 = static_cast<T>(Val1);
	constexpr auto val2 = static_cast<T>(Val2);
	
	if constexpr (std::is_unsigned_v<T>)
	{
		static_assert(std::is_unsigned_v<decltype(Val1)> || Val1 >= 0u,
			"Val1 must be unsigned or positive to produce unsigned result");
		static_assert(std::is_unsigned_v<decltype(Val2)> || Val2 >= 0u,
			"Val2 must be unsigned or positive to produce unsigned result");
		if constexpr (Val1 > (std::numeric_limits<T>::max)()
			|| Val2 > (std::numeric_limits<T>::max)())
		{
			return true;
		}
		
		constexpr T result = val1 * val2;
		return val1 && result / val1 != val2;
	}
	else
	{
		if constexpr (Val1 > (std::numeric_limits<T>::max)()
			|| Val1 < (std::numeric_limits<T>::min)())
		{
			return true;
		}
		if constexpr (Val2 > (std::numeric_limits<T>::max)()
			|| Val2 < (std::numeric_limits<T>::min)())
		{
			return true;
		}
		
		if constexpr (val1 == -1 && val2 == (std::numeric_limits<T>::min)())
			return true;
		else if constexpr (val2 == -1 && val1 == (std::numeric_limits<T>::min)())
			return true;
		else if constexpr (val1 > (std::numeric_limits<T>::max)() / val2)
			return true;
		else if constexpr (val1 < (std::numeric_limits<T>::min)() / val2)
			return true;
		else
			return false;
	}
}

template<typename T, auto Val1, auto Val2>
[[nodiscard]] constexpr T mul_with_check() noexcept
{
	static_assert(!mul_overflows<T, Val1, Val2>(), "Multiplication overflows");
	return static_cast<T>(Val1 * Val2);
}

template<auto Dividend, auto Divider>
constexpr auto div_ceil() noexcept
{
	return Dividend % Divider == 0
		? Dividend / Divider
		: Dividend / Divider + 1;
}

} //namespace mcutl::math
