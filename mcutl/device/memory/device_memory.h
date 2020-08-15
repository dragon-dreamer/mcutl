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
using mcutl::device::memory::common::get_register_array_bits;

#ifndef MCUTL_CORTEX_M3
using mcutl::device::memory::common::set_register_bits;
using mcutl::device::memory::common::set_register_array_bits;
#endif //MCUTL_CORTEX_M3
} //namespace mcutl::device::memory

#ifdef MCUTL_CORTEX_M3
#	include "mcutl/device/memory/arm/cortex_m3.h"
#endif //MCUTL_CORTEX_M3
