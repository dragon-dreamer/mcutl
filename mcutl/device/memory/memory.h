#pragma once

#include <memory>
#include <stdint.h>
#include <type_traits>

namespace mcutl::device::memory::common
{

template<typename Address>
[[nodiscard]] inline auto to_bytes(Address* address) noexcept
{
	if constexpr (std::is_volatile_v<Address>)
	{
		if constexpr (std::is_const_v<Address>)
			return reinterpret_cast<const volatile uint8_t*>(address);
		else
			return reinterpret_cast<volatile uint8_t*>(address);
	}
	else
	{
		if constexpr (std::is_const_v<Address>)
			return reinterpret_cast<const uint8_t*>(address);
		else
			return reinterpret_cast<uint8_t*>(address);
	}
}

template<typename Struct>
[[nodiscard]] inline auto to_bytes(Struct& obj) noexcept
{
	static_assert(std::is_trivially_copyable_v<Struct>, "Invalid input structure");
	return to_bytes(std::addressof(obj));
}

} //namespace mcutl::device::memory::common
