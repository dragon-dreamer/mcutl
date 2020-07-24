#pragma once

#include "mcutl/device/memory/device_memory.h"

namespace mcutl::memory
{

template<typename Address>
[[nodiscard]] inline auto to_bytes(Address* address) noexcept
{
	return mcutl::device::memory::to_bytes(address);
}

template<typename Struct>
[[nodiscard]] inline auto to_bytes(Struct& obj) noexcept
{
	return mcutl::device::memory::to_bytes(obj);
}

} //namespace mcutl::device::memory
