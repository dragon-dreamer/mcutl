#pragma once

#include <stdint.h>

#include "mcutl/device/dma/device_dma.h"
#include "mcutl/dma/dma_defs.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::dma
{

[[maybe_unused]] constexpr bool supports_priority_levels
	= device::dma::supports_priority_levels;
[[maybe_unused]] constexpr bool supports_byte_transfer
	= device::dma::supports_byte_transfer;
[[maybe_unused]] constexpr bool supports_halfword_transfer
	= device::dma::supports_halfword_transfer;
[[maybe_unused]] constexpr bool supports_word_transfer
	= device::dma::supports_word_transfer;
[[maybe_unused]] constexpr bool supports_circular_mode
	= device::dma::supports_circular_mode;
[[maybe_unused]] constexpr bool supports_memory_to_memory_transfer
	= device::dma::supports_memory_to_memory_transfer;
[[maybe_unused]] constexpr bool supports_memory_to_periph_transfer
	= device::dma::supports_memory_to_periph_transfer;
[[maybe_unused]] constexpr bool supports_periph_to_memory_transfer
	= device::dma::supports_periph_to_memory_transfer;
[[maybe_unused]] constexpr bool supports_periph_to_periph_transfer
	= device::dma::supports_periph_to_periph_transfer;
[[maybe_unused]] constexpr bool supports_transfer_complete_interrupt
	= device::dma::supports_transfer_complete_interrupt;
[[maybe_unused]] constexpr bool supports_half_transfer_interrupt
	= device::dma::supports_half_transfer_interrupt;
[[maybe_unused]] constexpr bool supports_transfer_error_interrupt
	= device::dma::supports_transfer_error_interrupt;
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags
	= device::dma::supports_atomic_clear_pending_flags;

namespace detail
{

template<bool Reconfigure, typename... Options>
constexpr auto parse_and_validate_transfer_options() noexcept
{
	constexpr auto options = opts::parse_and_validate_options<device::dma::transfer_options,
		detail::options_parser, Options...>();
	if constexpr (!Reconfigure)
	{
		static_assert(options.source_set_count != 0,
			"DMA source description is not present");
		static_assert(options.destination_set_count != 0,
			"DMA destination description is not present");
		
		constexpr bool mem_to_mem = options.source.addr_type == address_type::memory
			&& options.destination.addr_type == address_type::memory;
		static_assert(options.mode_value != mode::circular || !mem_to_mem,
			"DMA memory to memory mode is not compatible with circular data read mode");
	}
	else
	{
		static_assert(static_cast<bool>(options.source_set_count)
			== static_cast<bool>(options.destination_set_count),
			"Either both or none source and destination must be specified for reconfigure()");
	}
	return options;
}

} //namespace detail

using size_type = device::dma::size_type;

template<typename Channel, typename... Options>
void configure_channel() MCUTL_NOEXCEPT
{
	detail::dma_channel_validator<Channel>::validate();
	device::dma::configure_dma<Channel>([]() constexpr
		{ return detail::parse_and_validate_transfer_options<false, Options...>(); });
}

template<typename Channel, typename... Options>
void reconfigure_channel() MCUTL_NOEXCEPT
{
	detail::dma_channel_validator<Channel>::validate();
	device::dma::reconfigure_dma<Channel>([] () constexpr
		{ return detail::parse_and_validate_transfer_options<true, Options...>(); });
}

template<typename Channel>
inline void start_transfer(const volatile void* from,
	volatile void* to, size_type size) MCUTL_NOEXCEPT
{
	detail::dma_channel_validator<Channel>::validate();
	device::dma::start_transfer<Channel>(from, to, size);
}

template<typename Channel>
inline void wait_transfer() MCUTL_NOEXCEPT
{
	detail::dma_channel_validator<Channel>::validate();
	device::dma::wait_transfer<Channel>();
}

template<typename Channel, typename... Interrupts>
inline void clear_pending_flags() MCUTL_NOEXCEPT
{
	device::dma::clear_pending_flags<Channel, Interrupts...>();
}

template<typename Channel, typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	device::dma::clear_pending_flags_atomic<Channel, Interrupts...>();
}

} //namespace mcutl::dma
