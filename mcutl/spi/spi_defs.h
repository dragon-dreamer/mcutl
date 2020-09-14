#pragma once

#include <stdint.h>
#include <type_traits>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::spi
{

namespace slave_management
{

struct none {};
struct software {};
struct hardware_with_output {};
struct hardware_without_output {};

} //namespace slave_management

namespace frame_format
{

struct lsb_first {};
struct msb_first {};

} //namespace frame_format_type

namespace clock_polarity
{

struct idle_0 {};
struct idle_1 {};

} //namespace clock_polarity

namespace clock_phase
{

struct capture_first_clock {};
struct capture_second_clock {};

} //namespace clock_phase

namespace interrupt
{

struct transmit_buffer_empty {};
struct recieve_buffer_not_empty {};
struct error {};

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};

} //namespace interrupt

using cpol_0 = clock_polarity::idle_0;
using cpol_1 = clock_polarity::idle_1;
using cpha_0 = clock_phase::capture_first_clock;
using cpha_1 = clock_phase::capture_second_clock;

struct initialize_pins {};
struct clear_error_flags {};

namespace detail
{

struct slave_management_type
{
	enum value { none, software, hardware_with_output, hardware_without_output, end };
};

struct frame_format_type
{
	enum value { lsb_first, msb_first, end };
};

struct clock_polarity_type
{
	enum value { idle_0, idle_1, end };
};

struct clock_phase_type
{
	enum value { capture_first_clock, capture_second_clock, end };
};

struct options
{
	slave_management_type::value slave_management = slave_management_type::none;
	frame_format_type::value frame_format = frame_format_type::lsb_first;
	clock_polarity_type::value clock_polarity = clock_polarity_type::idle_0;
	clock_phase_type::value clock_phase = clock_phase_type::capture_first_clock;
	mcutl::interrupt::detail::interrupt_info transmit_buffer_empty {};
	mcutl::interrupt::detail::interrupt_info recieve_buffer_not_empty {};
	mcutl::interrupt::detail::interrupt_info error {};
	uint64_t priority_count = 0;
	
	uint32_t slave_management_set_count = 0;
	uint32_t frame_format_set_count = 0;
	uint32_t clock_polarity_set_count = 0;
	uint32_t clock_phase_set_count = 0;
	uint32_t initialize_pins_set_count = 0;
	uint32_t clear_error_flags_set_count = 0;
	uint32_t transmit_buffer_empty_set_count = 0;
	uint32_t recieve_buffer_not_empty_set_count = 0;
	uint32_t error_set_count = 0;
	uint32_t enable_controller_interrupts_set_count = 0;
	uint32_t disable_controller_interrupts_set_count = 0;
	uint32_t priority_count_set_count = 0;
};

template<typename Spi, typename Option>
struct options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown SPI option");
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda) noexcept
	{
	}
};

template<typename Spi>
struct options_parser<Spi, slave_management::none>
	: opts::base_option_parser<slave_management_type::none,
		&options::slave_management, &options::slave_management_set_count> {};
template<typename Spi>
struct options_parser<Spi, slave_management::software>
	: opts::base_option_parser<slave_management_type::software,
		&options::slave_management, &options::slave_management_set_count> {};
template<typename Spi>
struct options_parser<Spi, slave_management::hardware_with_output>
	: opts::base_option_parser<slave_management_type::hardware_with_output,
		&options::slave_management, &options::slave_management_set_count> {};
template<typename Spi>
struct options_parser<Spi, slave_management::hardware_without_output>
	: opts::base_option_parser<slave_management_type::hardware_without_output,
		&options::slave_management, &options::slave_management_set_count> {};

template<typename Spi>
struct options_parser<Spi, frame_format::lsb_first>
	: opts::base_option_parser<frame_format_type::lsb_first,
		&options::frame_format, &options::frame_format_set_count> {};
template<typename Spi>
struct options_parser<Spi, frame_format::msb_first>
	: opts::base_option_parser<frame_format_type::msb_first,
		&options::frame_format, &options::frame_format_set_count> {};

template<typename Spi>
struct options_parser<Spi, clock_polarity::idle_0>
	: opts::base_option_parser<clock_polarity_type::idle_0,
		&options::clock_polarity, &options::clock_polarity_set_count> {};
template<typename Spi>
struct options_parser<Spi, clock_polarity::idle_1>
	: opts::base_option_parser<clock_polarity_type::idle_1,
		&options::clock_polarity, &options::clock_polarity_set_count> {};

template<typename Spi>
struct options_parser<Spi, clock_phase::capture_first_clock>
	: opts::base_option_parser<clock_phase_type::capture_first_clock,
		&options::clock_phase, &options::clock_phase_set_count> {};
template<typename Spi>
struct options_parser<Spi, clock_phase::capture_second_clock>
	: opts::base_option_parser<clock_phase_type::capture_second_clock,
		&options::clock_phase, &options::clock_phase_set_count> {};

template<typename Spi>
struct options_parser<Spi, initialize_pins>
	: opts::base_option_parser<0, nullptr, &options::initialize_pins_set_count> {};
template<typename Spi>
struct options_parser<Spi, clear_error_flags>
	: opts::base_option_parser<0, nullptr, &options::clear_error_flags_set_count> {};

template<typename Interrupt>
struct interrupt_map : mcutl::interrupt::detail::map<Interrupt> {};

template<> struct interrupt_map<interrupt::transmit_buffer_empty>
	: mcutl::interrupt::detail::map_base<&options::transmit_buffer_empty_set_count,
		&options::transmit_buffer_empty> {};
template<> struct interrupt_map<interrupt::recieve_buffer_not_empty>
	: mcutl::interrupt::detail::map_base<&options::recieve_buffer_not_empty_set_count,
		&options::recieve_buffer_not_empty> {};
template<> struct interrupt_map<interrupt::error>
	: mcutl::interrupt::detail::map_base<&options::error_set_count, &options::error> {};

template<typename Spi>
struct options_parser<Spi, interrupt::transmit_buffer_empty>
	: opts::base_option_parser<0, nullptr, &options::transmit_buffer_empty_set_count> {};
template<typename Spi>
struct options_parser<Spi, interrupt::recieve_buffer_not_empty>
	: opts::base_option_parser<0, nullptr, &options::recieve_buffer_not_empty_set_count> {};
template<typename Spi>
struct options_parser<Spi, interrupt::error>
	: opts::base_option_parser<0, nullptr, &options::error_set_count> {};

template<typename Spi, typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct options_parser<Spi, mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: mcutl::interrupt::detail::interrupt_parser<Interrupt, interrupt_map, Priority, SubPriority> {};

template<typename Spi, typename Interrupt>
struct options_parser<Spi, mcutl::interrupt::disabled<Interrupt>>
	: mcutl::interrupt::detail::interrupt_parser<mcutl::interrupt::disabled<Interrupt>, interrupt_map> {};

template<typename Spi>
struct options_parser<Spi, interrupt::enable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &options::enable_controller_interrupts_set_count> {};

template<typename Spi>
struct options_parser<Spi, interrupt::disable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &options::disable_controller_interrupts_set_count> {};

template<typename Spi, auto PriorityCount>
struct options_parser<Spi, mcutl::interrupt::priority_count<PriorityCount>>
	: opts::base_option_parser<PriorityCount, &options::priority_count,
		&options::priority_count_set_count> {};

template<typename Spi>
struct spi_traits
{
	static_assert(types::always_false<Spi>::value, "Unknown SPI type");
};

template<uint8_t SpiIndex, typename Peripheral,
	typename MisoPin, typename MosiPin, typename SckPin, typename SsPin,
	typename DefaultSlaveManagementMode, typename SupportedDataTypes>
struct spi_base
{
	static constexpr uint8_t index = SpiIndex;
	using peripheral = Peripheral;
	using miso_pin = MisoPin;
	using mosi_pin = MosiPin;
	using sck_pin = SckPin;
	using ss_pin = SsPin;
	using default_slave_management_mode = DefaultSlaveManagementMode;
	using supported_data_types = SupportedDataTypes;
};

template<uint8_t SpiIndex, typename Peripheral,
	typename MisoPin, typename MosiPin, typename SckPin, typename SsPin,
	typename DefaultSlaveManagementMode, typename SupportedDataTypes>
struct spi_traits<spi_base<SpiIndex, Peripheral, MisoPin, MosiPin, SckPin, SsPin,
		DefaultSlaveManagementMode, SupportedDataTypes>>
	: spi_base<SpiIndex, Peripheral, MisoPin, MosiPin, SckPin, SsPin,
		DefaultSlaveManagementMode, SupportedDataTypes>
{
};

template<typename Spi, typename Interrupt>
struct interrupt_type_helper
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown SPI or interrupt type");
};

} //namespace detail

template<typename Spi>
using peripheral_type = typename detail::spi_traits<Spi>::peripheral;

template<typename Spi>
using default_slave_management_mode = typename detail::spi_traits<Spi>::default_slave_management_mode;

template<typename Spi>
using miso_pin = typename detail::spi_traits<Spi>::miso_pin;

template<typename Spi>
using mosi_pin = typename detail::spi_traits<Spi>::mosi_pin;

template<typename Spi>
using ss_pin = typename detail::spi_traits<Spi>::ss_pin;

template<typename Spi>
using sck_pin = typename detail::spi_traits<Spi>::sck_pin;

template<typename Spi, typename Interrupt>
using interrupt_type = typename detail::interrupt_type_helper<Spi, Interrupt>::type;

template<typename Spi>
using supported_data_types = typename detail::spi_traits<Spi>::supported_data_types;

template<typename...>
struct config {};

struct empty_mode
{
	template<typename Master, typename InitInfoLambda>
	static constexpr void init(InitInfoLambda) noexcept {}
	template<typename Master>
	static constexpr void wait() noexcept {}
	template<typename Master>
	static constexpr void ignore_received_data() noexcept {}
	template<typename Master, typename Data, typename Length>
	static constexpr void transmit(const Data*, Length) noexcept {}
	template<typename Master, typename Data, typename Length>
	static constexpr void receive(Data*, Length) noexcept {}
	template<typename Master>
	static constexpr void clear_error_flags() noexcept {}
	
	template<typename Master, typename InitInfoLambda>
	static constexpr auto prepare(InitInfoLambda options_lambda) noexcept
	{
		return options_lambda();
	}
	
	template<typename Master, typename Data>
	static void set_data(Data) noexcept
	{
		static_assert(types::always_false<Master>::value,
			"Unable to set SPI data for absent transmit mode");
	}
	
	template<typename Master>
	[[nodiscard]] static typename Master::data_type get_data() noexcept
	{
		static_assert(types::always_false<Master>::value,
			"Unable to get SPI data for absent receive mode");
		return {};
	}
	
	template<typename Master, bool Enable>
	static void enable_source_pointer_increment() noexcept
	{
		static_assert(types::always_false<Master>::value,
			"Unable to enable_source_pointer_increment for absent SPI transmit mode");
	}
};

} //namespace mcutl::spi
