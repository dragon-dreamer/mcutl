#pragma once

#include <stdint.h>

#include "mcutl/device/device.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::device::crc
{

using crc_input_type = uint32_t;
[[maybe_unused]] constexpr crc_input_type crc_initial_value = 0xffffffffu;
[[maybe_unused]] constexpr crc_input_type crc_polynomial = 0x4c11db7u;

inline void reset() MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<CRC_CR_RESET, &CRC_TypeDef::CR, CRC_BASE>();
}

inline void add_data(crc_input_type data) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<&CRC_TypeDef::DR, CRC_BASE>(data);
}

[[nodiscard]] inline crc_input_type get_crc() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_bits<&CRC_TypeDef::DR, CRC_BASE>();
}

} //namespace mcutl::device::crc
