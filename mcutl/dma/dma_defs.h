#pragma once

#include <stdint.h>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::dma
{

namespace mode
{

struct circular {};
struct normal {};

} //namespace mode

namespace data_size
{

struct byte {};
struct halfword {};
struct word {};

} //namespace data_size

namespace pointer_increment
{

struct disabled {};
struct enabled {};

} //namespace pointer_increment

namespace address
{

struct memory {};
struct peripheral {};

} //namespace address

namespace priority
{

struct low {};
struct medium {};
struct high {};
struct very_high {};

} //namespace priority

namespace interrupt
{

struct transfer_complete {};
struct half_transfer {};
struct transfer_error {};

struct global {}; //can be used only for clearing interrupt state

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};

} //namespace interrupt

namespace detail
{

template<typename DataSize, typename AddressType,
	typename PointerIncrementMode>
struct address_description
{
	using data_size_type = DataSize;
	using pointer_increment_mode_type = PointerIncrementMode;
	using address_type = AddressType;
};

} //namespace detail

template<typename DataSize, typename AddressType,
	typename PointerIncrementMode = pointer_increment::enabled>
struct source : detail::address_description<DataSize, AddressType, PointerIncrementMode> {};
template<typename DataSize, typename AddressType,
	typename PointerIncrementMode = pointer_increment::disabled>
struct destination : detail::address_description<DataSize, AddressType, PointerIncrementMode> {};

template<uint32_t DmaIndex, uint32_t ChannelNumber>
struct channel
{
	static constexpr uint32_t dma_index = DmaIndex;
	static constexpr uint32_t channel_number = ChannelNumber;
};

namespace detail
{

template<typename Interrupt, typename Channel>
struct interrupt_type_helper
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown DMA channel or interrupt type");
};

template<typename Channel>
struct peripheral_type_helper
{
	static_assert(types::always_false<Channel>::value,
		"Unknown DMA channel");
};

struct data_size
{
	enum value { byte, halfword, word, end };
};

struct address_type
{
	enum value { memory, peripheral, end };
};

struct pointer_increment_mode
{
	enum value { disabled, enabled, end };
};

struct priority
{
	enum value { low, medium, high, very_high, end };
};

struct mode
{
	enum value { normal, circular, end };
};

struct address_info
{
	data_size::value size {};
	address_type::value addr_type {};
	pointer_increment_mode::value increment_mode {};
};

struct transfer_options
{
	address_info source {};
	address_info destination {};
	priority::value priority_level = priority::low;
	mode::value mode_value = mode::normal;
	mcutl::interrupt::detail::interrupt_info transfer_complete {};
	mcutl::interrupt::detail::interrupt_info transfer_error {};
	mcutl::interrupt::detail::interrupt_info half_transfer {};
	uint64_t priority_count = 0;
	uint32_t source_set_count = 0;
	uint32_t destination_set_count = 0;
	uint32_t priority_level_set_count = 0;
	uint32_t mode_set_count = 0;
	uint32_t transfer_complete_set_count = 0;
	uint32_t transfer_error_set_count = 0;
	uint32_t half_transfer_set_count = 0;
	uint32_t global_interrupt_set_count = 0;
	uint32_t enable_controller_interrupts_set_count = 0;
	uint32_t disable_controller_interrupts_set_count = 0;
	uint32_t priority_count_set_count = 0;
};

template<typename Option>
struct options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown DMA option");
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda) noexcept
	{
	}
};

template<>
struct options_parser<dma::mode::normal>
	: opts::base_option_parser<mode::normal,
		&transfer_options::mode_value, &transfer_options::mode_set_count> {};

template<>
struct options_parser<dma::mode::circular>
	: opts::base_option_parser<mode::circular,
		&transfer_options::mode_value, &transfer_options::mode_set_count> {};

template<>
struct options_parser<dma::priority::low>
	: opts::base_option_parser<priority::low,
		&transfer_options::priority_level, &transfer_options::priority_level_set_count> {};

template<>
struct options_parser<dma::priority::medium>
	: opts::base_option_parser<priority::medium,
		&transfer_options::priority_level, &transfer_options::priority_level_set_count> {};

template<>
struct options_parser<dma::priority::high>
	: opts::base_option_parser<priority::high,
		&transfer_options::priority_level, &transfer_options::priority_level_set_count> {};

template<>
struct options_parser<dma::priority::very_high>
	: opts::base_option_parser<priority::very_high,
		&transfer_options::priority_level, &transfer_options::priority_level_set_count> {};

template<typename Interrupt>
struct interrupt_map : mcutl::interrupt::detail::map<Interrupt> {};

template<> struct interrupt_map<interrupt::transfer_complete>
	: mcutl::interrupt::detail::map_base<&transfer_options::transfer_complete_set_count,
	&transfer_options::transfer_complete> {};
template<> struct interrupt_map<interrupt::transfer_error>
	: mcutl::interrupt::detail::map_base<&transfer_options::transfer_error_set_count,
	&transfer_options::transfer_error> {};
template<> struct interrupt_map<interrupt::half_transfer>
	: mcutl::interrupt::detail::map_base<&transfer_options::half_transfer_set_count,
	&transfer_options::half_transfer> {};

template<>
struct options_parser<interrupt::global>
	: opts::base_option_parser<0, nullptr, &transfer_options::global_interrupt_set_count> {};

template<>
struct options_parser<interrupt::transfer_complete>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::transfer_complete, interrupt_map> {};

template<>
struct options_parser<interrupt::transfer_error>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::transfer_error, interrupt_map> {};

template<>
struct options_parser<interrupt::half_transfer>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::half_transfer, interrupt_map> {};

template<typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct options_parser<mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: mcutl::interrupt::detail::interrupt_parser<Interrupt, interrupt_map, Priority, SubPriority> {};

template<typename Interrupt>
struct options_parser<mcutl::interrupt::disabled<Interrupt>>
	: mcutl::interrupt::detail::interrupt_parser<mcutl::interrupt::disabled<Interrupt>, interrupt_map> {};

template<>
struct options_parser<interrupt::enable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &transfer_options::enable_controller_interrupts_set_count> {};

template<>
struct options_parser<interrupt::disable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &transfer_options::disable_controller_interrupts_set_count> {};

template<auto PriorityCount>
struct options_parser<mcutl::interrupt::priority_count<PriorityCount>>
	: opts::base_option_parser<PriorityCount, &transfer_options::priority_count,
		&transfer_options::priority_count_set_count> {};

template<typename DataSize>
constexpr data_size::value get_data_size() noexcept
{
	static_assert(types::always_false<DataSize>::value, "Unsupported data size");
	return data_size::end;
}

template<>
constexpr data_size::value get_data_size<dma::data_size::byte>() noexcept
{
	return data_size::byte;
}

template<>
constexpr data_size::value get_data_size<dma::data_size::halfword>() noexcept
{
	return data_size::halfword;
}

template<>
constexpr data_size::value get_data_size<dma::data_size::word>() noexcept
{
	return data_size::word;
}

template<typename AddressType>
constexpr address_type::value get_address_type() noexcept
{
	static_assert(types::always_false<AddressType>::value, "Unsupported address type");
	return address_type::end;
}

template<>
constexpr address_type::value get_address_type<dma::address::memory>() noexcept
{
	return address_type::memory;
}

template<>
constexpr address_type::value get_address_type<dma::address::peripheral>() noexcept
{
	return address_type::peripheral;
}

template<typename PointerIncrementMode>
constexpr pointer_increment_mode::value get_pointer_increment_mode() noexcept
{
	static_assert(types::always_false<PointerIncrementMode>::value,
		"Unsupported pointer increment mode");
	return pointer_increment_mode::end;
}

template<>
constexpr pointer_increment_mode::value get_pointer_increment_mode<
	dma::pointer_increment::disabled>() noexcept
{
	return pointer_increment_mode::disabled;
}

template<>
constexpr pointer_increment_mode::value get_pointer_increment_mode<
	dma::pointer_increment::enabled>() noexcept
{
	return pointer_increment_mode::enabled;
}

template<typename DataSize, typename AddressType, typename PointerIncrementMode>
constexpr void parse_address_options(address_info& address) noexcept
{
	address.size = static_cast<data_size::value>(get_data_size<DataSize>());
	address.addr_type = static_cast<address_type::value>(get_address_type<AddressType>());
	address.increment_mode = static_cast<pointer_increment_mode::value>(
		get_pointer_increment_mode<PointerIncrementMode>());
}

template<typename DataSize, typename AddressType, typename PointerIncrementMode>
struct options_parser<source<DataSize, AddressType, PointerIncrementMode>>
	: opts::base_option_parser<0, nullptr, &transfer_options::source_set_count>
{
	template<typename Option>
	static constexpr void parse(Option& options) noexcept
	{
		parse_address_options<DataSize, AddressType, PointerIncrementMode>(options.source);
		++options.source_set_count;
	}
};

template<typename DataSize, typename AddressType, typename PointerIncrementMode>
struct options_parser<destination<DataSize, AddressType, PointerIncrementMode>>
	: opts::base_option_parser<0, nullptr, &transfer_options::destination_set_count>
{
	template<typename Option>
	static constexpr void parse(Option& options) noexcept
	{
		parse_address_options<DataSize, AddressType, PointerIncrementMode>(options.destination);
		++options.destination_set_count;
	}
};

template<typename Channel>
struct dma_channel_validator
{
	static constexpr void validate() noexcept
	{
		static_assert(types::always_false<Channel>::value, "Unsupported DMA channel");
	}
};

} //namespace detail

template<typename Interrupt, typename Channel>
using interrupt_type = typename detail::interrupt_type_helper<Interrupt, Channel>::type;

template<typename Channel>
using peripheral_type = typename detail::peripheral_type_helper<Channel>::type;

} //namespace mcutl::dma
