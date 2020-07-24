#pragma once

#include "mcutl/device/device.h"
#include "mcutl/device/memory/memory.h"
#include "mcutl/device/memory/volatile_memory.h"

namespace mcutl::device::memory
{
//Use generic implementations
using mcutl::device::memory::common::max_bitmask;
using mcutl::device::memory::common::to_address;
using mcutl::device::memory::common::to_bytes;
using mcutl::device::memory::common::volatile_memory;
using mcutl::device::memory::common::get_register_bits;
using mcutl::device::memory::common::get_register_flag;
using mcutl::device::memory::common::set_register_value;

#ifndef __CORTEX_M
using mcutl::device::memory::common::set_register_bits;
#else //__CORTEX_M
#	if __CORTEX_M != 3u
using mcutl::device::memory::common::set_register_bits;
#	endif //__CORTEX_M != 3u
#endif //__CORTEX_M
} //namespace mcutl::device::memory

#ifdef __CORTEX_M
#	if __CORTEX_M == 3u
#		include "mcutl/device/memory/arm/cortex_m3.h"
#	endif //__CORTEX_M == 3u
#endif //__CORTEX_M
