#pragma once

#include <cstring>
#include <stddef.h>
#include <stdint.h>

#include "mcutl/device/crc/device_crc.h"
#include "mcutl/memory/memory.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::crc
{

using crc_input_type = device::crc::crc_input_type;
[[maybe_unused]] constexpr crc_input_type crc_initial_value = device::crc::crc_initial_value;
[[maybe_unused]] constexpr crc_input_type crc_polynomial = device::crc::crc_polynomial;
using peripheral_type = device::crc::peripheral_type;

inline void reset() MCUTL_NOEXCEPT
{
	device::crc::reset();
}

inline void add_data(crc_input_type data) MCUTL_NOEXCEPT
{
	device::crc::add_data(data);
}

[[nodiscard]] inline crc_input_type get_crc() MCUTL_NOEXCEPT
{
	return device::crc::get_crc();
}

[[nodiscard]] inline crc_input_type calculate_crc(const crc_input_type* data_pointer,
	size_t data_size) MCUTL_NOEXCEPT
{
	reset();
	for (size_t i = 0; i != data_size; ++i, ++data_pointer)
		add_data(*data_pointer);
	
	return get_crc();
}

namespace detail
{

inline crc_input_type get_last_data_value(const uint8_t* remaining_data,
	size_t remaining_size) noexcept
{
	crc_input_type input {};
	std::memcpy(&input, remaining_data, remaining_size);
	return input;
}

template<size_t DataSize, size_t RemainingDataSize, typename DataPointer>
crc_input_type calculate_crc(DataPointer* data_pointer) MCUTL_NOEXCEPT
{
	reset();
	
	if constexpr (!!DataSize)
	{
		for (size_t i = 0; i != DataSize; ++i, data_pointer += sizeof(crc_input_type))
		{
			crc_input_type input;
			std::memcpy(&input, data_pointer, sizeof(input));
			add_data(input);
		}
	}
	
	if constexpr (!!RemainingDataSize)
		add_data(get_last_data_value(data_pointer, RemainingDataSize));
	
	return get_crc();
}

} //namespace detail

[[nodiscard]] inline crc_input_type calculate_crc(const uint8_t* data_pointer,
	size_t data_size_in_bytes) MCUTL_NOEXCEPT
{
	reset();
	
	size_t data_size = data_size_in_bytes / sizeof(crc_input_type);
	size_t remaining_data_size = data_size_in_bytes - data_size * sizeof(crc_input_type);
	
	for (size_t i = 0; i != data_size; ++i, data_pointer += sizeof(crc_input_type))
	{
		crc_input_type input;
		std::memcpy(&input, data_pointer, sizeof(input));
		add_data(input);
	}
	
	if (remaining_data_size)
		add_data(detail::get_last_data_value(data_pointer, remaining_data_size));
	
	return get_crc();
}

template<typename Data>
[[nodiscard]] crc_input_type calculate_crc(const Data& data) MCUTL_NOEXCEPT
{
	static constexpr size_t data_size = sizeof(Data) / sizeof(crc_input_type);
	static constexpr size_t remaining_data_size = sizeof(Data)
		- data_size * sizeof(crc_input_type);
	
	return detail::calculate_crc<data_size, remaining_data_size>(
		mcutl::memory::to_bytes(data));
}

template<typename T, size_t N>
[[nodiscard]] crc_input_type calculate_crc(T (&arr)[N]) MCUTL_NOEXCEPT
{
	static constexpr size_t data_size = sizeof(T) * N / sizeof(crc_input_type);
	static constexpr size_t remaining_data_size = sizeof(T) * N
		- data_size * sizeof(crc_input_type);
	
	return detail::calculate_crc<data_size, remaining_data_size>(
		mcutl::memory::to_bytes(arr));
}

} //namespace mcutl::crc
