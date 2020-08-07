#pragma once

#include <type_traits>

#include "mcutl/gpio/gpio_defs.h"
#include "mcutl/device/gpio/device_gpio.h"
#include "mcutl/periph/periph.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::gpio
{

namespace detail
{

template<typename PinConfig, char PortLetter, uint32_t... PinNumbers>
constexpr bool is_valid_pin(device::gpio::valid_gpio_base<PortLetter, PinNumbers...>) noexcept
{
	return PortLetter == PinConfig::pin::port_letter
		&& (... || (PinConfig::pin::pin_number == PinNumbers));
}

template<typename PinConfig>
constexpr bool is_valid_pin(device::gpio::valid_gpio_list<>) noexcept
{
	return false;
}

template<typename PinConfig, typename AvailableReg, typename... AvailableRegs>
constexpr bool is_valid_pin(device::gpio::valid_gpio_list<AvailableReg, AvailableRegs...>) noexcept
{
	return is_valid_pin<PinConfig>(AvailableReg{})
		|| is_valid_pin<PinConfig>(device::gpio::valid_gpio_list<AvailableRegs...>{});
}

template<typename PinConfig1, typename PinConfig2>
struct pin_config_same
{
	static constexpr bool value
		= std::is_same_v<typename PinConfig1::pin, typename PinConfig2::pin>
		&& std::is_same_v<typename PinConfig1::tag, typename PinConfig2::tag>;
};

template<typename PinConfig1, typename PinConfig2>
struct exti_line_same
{
	static constexpr bool get_value() noexcept
	{
		if constexpr (std::is_same_v<typename PinConfig1::tag, exti_tag>
			&& std::is_same_v<typename PinConfig2::tag, exti_tag>)
		{
			return PinConfig1::line == PinConfig2::line;
		}
		else
		{
			return false;
		}
	}
	
	static constexpr bool value = get_value();
};

template<typename... PinConfig>
constexpr bool validate_pin_configs(config<PinConfig...>) noexcept
{
	constexpr bool has_duplicates = types::has_duplicates_filtered_v<
		pin_config_same, PinConfig...>;
	constexpr bool pins_are_valid = (... && is_valid_pin<PinConfig>(
		device::gpio::available_regs_t{}));
	constexpr bool has_duplicate_exti_lines = types::has_duplicates_filtered_v<
		exti_line_same, PinConfig...>;
	static_assert(!has_duplicates, "Duplicate pin configurations");
	static_assert(!has_duplicate_exti_lines, "Duplicate pin EXTI lines");
	static_assert(pins_are_valid, "Invalid gpio pin for selected MCU");
	return pins_are_valid && !has_duplicates;
}

template<typename... PinConfig>
struct configuration_helper
{
	static void configure_gpio() MCUTL_NOEXCEPT
	{
		using pin_config_t = types::remove_from_container_t<enable_peripherals, config<PinConfig...>>;
		
		constexpr bool enable_peripherals_requested
			= !std::is_same_v<pin_config_t, config<PinConfig...>>;
		
		static_assert(pin_config_t::length != 0, "Empty pin configuration");
		if constexpr (pin_config_t::length && validate_pin_configs(pin_config_t{}))
			device::gpio::configure_gpio<enable_peripherals_requested, pin_config_t>();
	}
	
	static constexpr auto get_pin_bit_mask() noexcept
	{
		if constexpr (!validate_pin_configs(
			config<detail::config_base<PinConfig, detail::gpio_tag>...>{}))
		{
			return 0;
		}
		else
		{
			return device::gpio::get_pin_bit_mask<PinConfig...>();
		}
	}
};

template<typename... PinConfig>
struct configuration_helper<config<PinConfig...>> : configuration_helper<PinConfig...> {};

} //namespace detail

template<typename... PinConfig>
void configure_gpio() MCUTL_NOEXCEPT
{
	detail::configuration_helper<PinConfig...>::configure_gpio();
}

template<typename... PinConfigs>
[[maybe_unused]] constexpr auto pin_bit_mask_v
	= detail::configuration_helper<PinConfigs...>::get_pin_bit_mask();

template<typename Pin, typename Value, typename... OutputOptions>
void set_out_value() MCUTL_NOEXCEPT
{
	configure_gpio<to_value<Pin, Value, OutputOptions...>>();
}

[[maybe_unused]] constexpr bool has_atomic_set_out_value = device::gpio::has_atomic_set_out_value;

template<typename Pin, typename Value, typename... OutputOptions>
void set_out_value_atomic() MCUTL_NOEXCEPT
{
	configure_gpio<to_value<Pin, Value, out::opt::atomic, OutputOptions...>>();
}

template<typename Pin>
void set_one() MCUTL_NOEXCEPT
{
	set_out_value<Pin, out::one>();
}

template<typename Pin>
void set_zero() MCUTL_NOEXCEPT
{
	set_out_value<Pin, out::zero>();
}

template<typename Pin>
void set_one_atomic() MCUTL_NOEXCEPT
{
	set_out_value_atomic<Pin, out::one>();
}

template<typename Pin>
void set_zero_atomic() MCUTL_NOEXCEPT
{
	set_out_value_atomic<Pin, out::zero>();
}

template<bool NegateBits, typename... Pins>
[[nodiscard]] auto get_input_values_mask() MCUTL_NOEXCEPT
{
	if constexpr (sizeof...(Pins) == 0)
		return 0u;
	else
		return device::gpio::get_input_values_mask<NegateBits, Pins...>();
}

template<bool NegateBits, typename... Pins>
[[nodiscard]] auto get_output_values_mask() MCUTL_NOEXCEPT
{
	if constexpr (sizeof...(Pins) == 0)
		return 0u;
	else
		return device::gpio::get_output_values_mask<NegateBits, Pins...>();
}

template<typename Pin>
[[nodiscard]] bool get_input_bit() MCUTL_NOEXCEPT
{
	return static_cast<bool>(get_input_values_mask<false, Pin>());
}

template<typename Pin>
[[nodiscard]] bool get_output_bit() MCUTL_NOEXCEPT
{
	return static_cast<bool>(get_output_values_mask<false, Pin>());
}

template<typename Pin>
[[nodiscard]] bool is_output() MCUTL_NOEXCEPT
{
	return device::gpio::is_output<Pin>();
}

template<typename PinConfig>
[[nodiscard]] bool is() MCUTL_NOEXCEPT
{
	if constexpr (!detail::validate_pin_configs(config<PinConfig>{}))
		return false;
	else
		return device::gpio::is<PinConfig>();
}

template<typename Pin, typename... Options>
[[nodiscard]] bool has() MCUTL_NOEXCEPT
{
	static_assert(sizeof...(Options) != 0, "No options passed to gpio::has()");
	
	constexpr bool has_unknown_options = (... || std::is_same_v<
		detail::option_category_t<Options>, detail::unknown_option_category>);
	static_assert(!has_unknown_options, "Unknown options passed to gpio::has()");
	
	if constexpr (!sizeof...(Options)
		|| has_unknown_options
		|| !detail::validate_pin_configs(config<to_value<Pin, out::one>>{}))
	{
		return false;
	}
	else
	{
		return device::gpio::has<Pin, Options...>();
	}
}

} //namespace mcutl::gpio
