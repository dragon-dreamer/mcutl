#pragma once

#include <limits>
#include <limits.h>
#include <stdint.h>
#include <type_traits>

#include "mcutl/utils/endian.h"

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
[[nodiscard]] constexpr auto div_ceil() noexcept
{
	return Dividend % Divider == 0
		? Dividend / Divider
		: Dividend / Divider + 1;
}

template<typename Ratio>
[[nodiscard]] constexpr auto round() noexcept
{
	return (Ratio::num + (Ratio::den / 2)) / Ratio::den;
}

namespace detail
{

template<typename T>
constexpr T sqrt_helper(T x, T lo, T hi) noexcept
{
	if (lo == hi)
		return lo;

	const T mid = (lo + hi + 1) / 2;

	if (x / mid < mid)
		return sqrt_helper<T>(x, lo, mid - 1);
	else
		return sqrt_helper(x, mid, hi);
}

} //namespace detail

template<typename T>
[[nodiscard]] constexpr T sqrt(T x) noexcept
{
	return detail::sqrt_helper<T>(x, 0, x / 2 + 1);
}

template<typename T, T Value> 
[[nodiscard]] constexpr T max_value() noexcept
{
	return Value;
}

template<typename T, T Value1, T Value2> 
[[nodiscard]] constexpr T max_value() noexcept
{
	if constexpr (Value1 > Value2)
		return Value1;
	else
		return Value2;
}

template<typename T, T Value1, T Value2, T Value3, T... Values> 
[[nodiscard]] constexpr T max_value() noexcept
{
	return max_value<T, max_value<T, Value1, Value2>(), Value3, Values...>();
}

template<typename T, T Value> 
[[nodiscard]] constexpr T min_value() noexcept
{
	return Value;
}

template<typename T, T Value1, T Value2> 
[[nodiscard]] constexpr T min_value() noexcept
{
	if constexpr (Value1 < Value2)
		return Value1;
	else
		return Value2;
}

template<typename T, T Value1, T Value2, T Value3, T... Values> 
[[nodiscard]] constexpr T min_value() noexcept
{
	return min_value<T, min_value<T, Value1, Value2>(), Value3, Values...>();
}

[[nodiscard]] constexpr uint16_t reverse_bytes(uint16_t value) noexcept
{
	uint16_t low = value & 0xff;
	uint16_t high = value >> 8;
	return high | (low << 8);
}

[[nodiscard]] constexpr uint32_t reverse_bytes(uint32_t value) noexcept
{
	uint16_t octet1 = value & 0xff;
	uint16_t octet2 = (value >> 8) & 0xff;
	uint16_t octet3 = (value >> 16) & 0xff;
	uint16_t octet4 = value >> 24;
	return (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;
}

template<typename Value>
[[nodiscard]] constexpr auto reverse_bytes_if_le(Value value) noexcept
{
	if constexpr (types::endian::native == types::endian::little)
		return reverse_bytes(value);
	else
		return static_cast<decltype(reverse_bytes(value))>(value);
}

} //namespace mcutl::math
