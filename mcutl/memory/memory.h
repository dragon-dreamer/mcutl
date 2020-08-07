#pragma once

#ifdef MCUTL_TEST
#	include "mcutl/tests/volatile_memory.h"
#else //MCUTL_TEST
#	include "mcutl/device/memory/device_memory.h"
#endif //MCUTL_TEST

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
