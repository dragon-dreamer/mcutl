#pragma once

#include <stdint.h>
#include <type_traits>
#include <utility>

#include "mcutl/gpio/gpio.h"
#include "mcutl/spi/spi_defs.h"
#include "mcutl/device/spi/device_spi.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/options_parser.h"

namespace mcutl::spi
{

namespace detail
{

template<typename Spi, typename... Options>
struct options_helper
{
	static constexpr auto parse_and_validate_options() noexcept
	{
		return opts::parse_and_validate_options<
			device::spi::options, options_parser, Spi, Options...>();
	}
};

template<typename Spi, typename... Options>
struct options_helper<Spi, config<Options...>>
	: options_helper<Spi, Options...>
{
};

} //namespace detail

template<typename Spi>
[[maybe_unused]] constexpr bool supports_software_slave_management
	= device::spi::supports_software_slave_management<Spi>;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_hardware_with_output_slave_management
	= device::spi::supports_hardware_with_output_slave_management<Spi>;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_hardware_without_output_slave_management
	= device::spi::supports_hardware_without_output_slave_management<Spi>;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_frame_format
	= device::spi::supports_frame_format<Spi>;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_error_interrupt
	= device::spi::supports_error_interrupt<Spi>;

template<typename Spi, typename ReceiveMode, typename TransmitMode,
	typename DataType = uint8_t>
class master : device::spi::master<
	master<Spi, ReceiveMode, TransmitMode, DataType>>
{
private:
	using device_master = device::spi::master<
		master<Spi, ReceiveMode, TransmitMode, DataType>>;
	friend device_master;
	
	static_assert(!std::is_same_v<TransmitMode, empty_mode>,
		"TransmitMode for SPI master can not be empty");
	
private:
	template<typename... Options>
	static constexpr auto prepare_spi_config() noexcept
	{
		return ReceiveMode::template prepare<master>([] () constexpr {
			return TransmitMode::template prepare<master>([] () constexpr {
				return device_master::prepare([] () constexpr {
					return detail::options_helper<Spi, Options...>
						::parse_and_validate_options();
				});
			});
		});
	}
	
public:
	using spi_type = Spi;
	using spi_traits = detail::spi_traits<spi_type>;
	using data_type = DataType;
	using receive_mode = ReceiveMode;
	using transmit_mode = TransmitMode;
	using data_length_type = typename device_master::data_length_type;
	static constexpr data_length_type max_data_length = device_master::max_data_length;
	
	template<typename... Options>
	static constexpr auto get_gpio_config() noexcept
	{
		constexpr auto options_lambda = [] () constexpr { return prepare_spi_config<Options...>(); };
		return device_master::template get_gpio_config(options_lambda);
	}
	
	template<typename... Options>
	using gpio_config = decltype(get_gpio_config<Options...>());
	
public:
	master(const master&) = delete;
	master& operator=(const master&) = delete;
	constexpr master(master&&) MCUTL_NOEXCEPT = default;
	constexpr master& operator=(master&&) MCUTL_NOEXCEPT = default;
	
public:
	template<typename ReceiveModeType = ReceiveMode, typename TransmitModeType = TransmitMode>
	constexpr master(ReceiveModeType&& receive_mode = {},
		TransmitModeType&& transmit_mode = {}) MCUTL_NOEXCEPT
		: receive_mode_(std::forward<ReceiveModeType>(receive_mode))
		, transmit_mode_(std::forward<TransmitModeType>(transmit_mode))
	{
	}
	
	[[nodiscard]] ReceiveMode& get_receive_mode() noexcept
	{
		return receive_mode_;
	}
	
	[[nodiscard]] TransmitMode& get_transmit_mode() noexcept
	{
		return transmit_mode_;
	}
	
	template<typename... Options>
	void configure() MCUTL_NOEXCEPT
	{
		constexpr auto options_lambda = [] () constexpr { return prepare_spi_config<Options...>(); };
		device_master::configure(options_lambda);
		transmit_mode_.template configure<master>(options_lambda);
		receive_mode_.template configure<master>(options_lambda);
	}
	
	template<typename... Options>
	static void init_pins() MCUTL_NOEXCEPT
	{
		mcutl::gpio::configure_gpio<gpio_config<Options...>>();
	}
	
	void disable() MCUTL_NOEXCEPT
	{
		wait();
		device_master::disable();
	}
	
	void enable() MCUTL_NOEXCEPT
	{
		device_master::enable();
	}
	
	void wait() MCUTL_NOEXCEPT
	{
		receive_mode_.template wait<master>();
		transmit_mode_.template wait<master>();
		device_master::wait();
	}
	
	void transmit(const data_type* data, data_length_type length) MCUTL_NOEXCEPT
	{
		device_master::transmit_prepare();
		receive_mode_.template ignore_received_data<master>();
		transmit_mode_.template transmit<master>(data, length);
	}
	
	void transmit_receive(const data_type* data, data_type* receive_buffer,
		data_length_type length) MCUTL_NOEXCEPT
	{
		device_master::transmit_receive_prepare();
		receive_mode_.template receive<master>(receive_buffer, length);
		transmit_mode_.template transmit<master>(data, length);
	}
	
	void clear_error_flags() MCUTL_NOEXCEPT
	{
		device_master::clear_error_flags();
		receive_mode_.template clear_error_flags<master>();
		transmit_mode_.template clear_error_flags<master>();
	}
	
	void set_data(data_type data) MCUTL_NOEXCEPT
	{
		transmit_mode_.template set_data<master>(data);
	}
	
	[[nodiscard]] data_type get_data() MCUTL_NOEXCEPT
	{
		return receive_mode_.template get_data<master>();
	}
	
	template<bool Enable>
	void enable_source_increment() MCUTL_NOEXCEPT
	{
		transmit_mode_.template enable_source_increment<master, Enable>();
	}
	
private:
	ReceiveMode receive_mode_;
	TransmitMode transmit_mode_;
};

template<typename Spi, typename DataType = uint8_t, typename ReceiveMode, typename TransmitMode>
[[nodiscard]] decltype(auto) create_spi_master(ReceiveMode&& receive_mode,
	TransmitMode&& transmit_mode) MCUTL_NOEXCEPT
{
	return master<Spi, ReceiveMode, TransmitMode, DataType>(
		std::forward<ReceiveMode>(receive_mode),
		std::forward<TransmitMode>(transmit_mode));
}

template<typename Spi, typename... Interrupts>
[[nodiscard]] inline auto get_pending_flags() MCUTL_NOEXCEPT
{
	return device::spi::get_pending_flags<Spi, Interrupts...>();
}

template<typename Spi, typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = device::spi::pending_flags_v<Spi, Interrupts...>;

} //namespace mcutl::spi
