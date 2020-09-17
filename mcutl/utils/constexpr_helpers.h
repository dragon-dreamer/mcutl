#pragma once

#include <stddef.h>

namespace mcutl::utils
{

template<typename T, size_t N>
[[nodiscard]] constexpr size_t countof(T const(&)[N]) noexcept
{
	return N;
}

template<typename Char>
[[nodiscard]] constexpr size_t strlen(const Char* str) noexcept
{
	const Char* end = str;
	while (*end)
		++end;
	
	return end - str;
}

} //namespace mcutl::utils
