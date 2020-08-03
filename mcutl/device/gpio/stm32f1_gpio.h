#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include "mcutl/device/gpio/stm32_gpio.h"
#include "mcutl/gpio/gpio_defs.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::gpio
{

struct port_bit_data
{
	uint64_t cr_changed_bits = 0;
	uint64_t cr_bit_values = 0;
	uint16_t dr_set_bits = 0;
	uint16_t dr_reset_bits = 0;
	
	bool invalid_output_options = false;
};

struct defines
{
	static constexpr uint64_t conf_and_mode_bit_count = 4ull;
	static constexpr uint64_t conf_bit_count = 2ull;
	static constexpr uint64_t mode_bit_count = 2ull;
	
	static constexpr uint32_t mode_bits_offset = 0;
	static constexpr uint32_t conf_bits_offset = 2;
	
	static constexpr uint32_t open_drain_conf = 0b01;
	static constexpr uint32_t push_pull_conf = 0b00;
	static constexpr uint32_t open_drain_alt_func_conf = 0b11;
	static constexpr uint32_t push_pull_alt_func_conf = 0b10;
	
	static constexpr uint32_t input_mode = 0b00;
	static constexpr uint32_t output_freq_2mhz_mode = 0b10;
	static constexpr uint32_t output_freq_10mhz_mode = 0b01;
	static constexpr uint32_t output_freq_50mhz_mode = 0b11;
	
	static constexpr uint32_t input_floating_conf = 0b01;
	static constexpr uint32_t input_pull_up_conf = 0b10;
	static constexpr uint32_t input_pull_down_conf = 0b10;
	static constexpr uint32_t input_analog_conf = 0b00;
};

template<typename... PinConfig>
struct config_helper {};

enum class config_type
{
	value,
	input,
	output
};

template<typename PinConfig>
struct pin_helper
{
	static constexpr void apply(port_bit_data&, char) noexcept
	{
		static_assert(types::always_false<PinConfig>::value,
			"Unsupported pin configuration type");
	}
};

template<typename... Options>
struct output_options_helper
{
	static constexpr std::pair<bool, uint64_t> apply() noexcept
	{
		return { true, defines::output_freq_50mhz_mode << defines::mode_bits_offset };
	}
};

template<typename Option, typename... Options>
struct output_options_helper<Option, Options...>
{
	static constexpr std::pair<bool, uint64_t> apply() noexcept
	{
		constexpr bool valid = sizeof...(Options) == 0;
		if constexpr (std::is_same_v<Option, mcutl::gpio::out::opt::freq_2mhz>)
			return { valid, defines::output_freq_2mhz_mode << defines::mode_bits_offset };
		else if constexpr (std::is_same_v<Option, mcutl::gpio::out::opt::freq_10mhz>)
			return { valid, defines::output_freq_10mhz_mode << defines::mode_bits_offset };
		else if constexpr (std::is_same_v<Option, mcutl::gpio::out::opt::freq_50mhz>)
			return { valid, defines::output_freq_50mhz_mode << defines::mode_bits_offset };
		else
			return { false, 0 };
	}
};

template<typename Pin>
constexpr void apply_cr_bitmask(port_bit_data& data) noexcept
{
	uint64_t conf_mode_bit_mask = (1ull << defines::conf_and_mode_bit_count) - 1ull;
	uint64_t cr_changed_bits = conf_mode_bit_mask << (Pin::pin_number * defines::conf_and_mode_bit_count);
	data.cr_changed_bits |= cr_changed_bits;
}

template<typename Pin, typename OutputMode,
	typename OutValue, typename... OutputOptions>
struct pin_helper<mcutl::gpio::as_output<Pin, OutputMode, OutValue, OutputOptions...>>
{
	static constexpr config_type type = config_type::output;
	
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		apply_cr_bitmask<Pin>(data);
		
		uint64_t cr_bit_values = 0;
		if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::open_drain>)
			cr_bit_values = defines::open_drain_conf << defines::conf_bits_offset;
		else if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::open_drain_alt_func>)
			cr_bit_values = defines::open_drain_alt_func_conf << defines::conf_bits_offset;
		else if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::push_pull_alt_func>)
			cr_bit_values = defines::push_pull_alt_func_conf << defines::conf_bits_offset;
		else if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::push_pull>)
			cr_bit_values = defines::push_pull_conf << defines::conf_bits_offset;
		else
			unknown_output_mode<OutputMode>();
		
		constexpr auto options_parse_result = output_options_helper<OutputOptions...>::apply();
		static_assert(options_parse_result.first, "Unsupported or conflicting options for gpio::as_output");
		cr_bit_values |= options_parse_result.second;
		
		cr_bit_values <<= Pin::pin_number * defines::conf_and_mode_bit_count;
		data.cr_bit_values |= cr_bit_values;
		
		pin_helper<mcutl::gpio::to_value<Pin, OutValue>>::apply(data, port_letter);
	}
	
	template<typename T>
	static constexpr void unknown_output_mode() noexcept
	{
		static_assert(types::always_false<T>::value, "Unknown pin output mode");
	}
};

template<typename Pin, typename OutValue, typename... OutputOptions>
struct pin_helper<mcutl::gpio::to_value<Pin, OutValue, OutputOptions...>>
{
	static constexpr config_type type = config_type::value;
	
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		uint16_t dr_set_bits = 0, dr_reset_bits = 0;
		if constexpr (std::is_same_v<OutValue, mcutl::gpio::out::one>)
			dr_set_bits = 1u << Pin::pin_number;
		else if constexpr (std::is_same_v<OutValue, mcutl::gpio::out::zero>)
			dr_reset_bits = 1u << Pin::pin_number;
		
		if constexpr (sizeof...(OutputOptions) > 0)
		{
			static_assert(sizeof...(OutputOptions) == 1
					&& std::is_same_v<types::first_type_t<OutputOptions...>, mcutl::gpio::out::opt::atomic>,
				"Unsupported or conflicting options for gpio::to_value");
		}
		
		data.dr_set_bits |= dr_set_bits;
		data.dr_reset_bits |= dr_reset_bits;
	}
};

template<typename Pin, typename InputMode, typename... InputOptions>
struct pin_helper<mcutl::gpio::as_input<Pin, InputMode, InputOptions...>>
{
	static constexpr config_type type = config_type::input;
	
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		static_assert(sizeof...(InputOptions) == 0,
			"No additional gpio::as_input options are supported");
		
		apply_cr_bitmask<Pin>(data);
		
		uint64_t cr_bit_values = 0;
		if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::floating>)
		{
			cr_bit_values = defines::input_floating_conf << defines::conf_bits_offset;
		}
		else if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::pull_up>)
		{
			cr_bit_values = defines::input_pull_up_conf << defines::conf_bits_offset;
			data.dr_set_bits |= 1u << Pin::pin_number;
		}
		else if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::pull_down>)
		{
			cr_bit_values = defines::input_pull_down_conf << defines::conf_bits_offset;
			data.dr_reset_bits |= 1u << Pin::pin_number;
		}
		else if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::analog>)
		{
			cr_bit_values = defines::input_analog_conf << defines::conf_bits_offset;
		}
		else
		{
			unknown_input_mode<InputMode>();
		}
		
		cr_bit_values <<= Pin::pin_number * defines::conf_and_mode_bit_count;
		data.cr_bit_values |= cr_bit_values;
	}
	
	template<typename T>
	static constexpr void unknown_input_mode() noexcept
	{
		static_assert(types::always_false<T>::value, "Unknown pin input mode");
	}
};

template<typename... PinConfig>
struct config_helper<mcutl::gpio::config<PinConfig...>>
{
	static void configure() MCUTL_NOEXCEPT
	{
		config_helper<mcutl::gpio::config<PinConfig...>,
			available_regs_t>::configure();
	}
	
	template<char PortLetter>
	static constexpr port_bit_data get_port_bit_data() noexcept
	{
		return config_helper<mcutl::gpio::config<PinConfig...>,
			available_regs_t>::template get_port_bit_data<PortLetter>();
	}
};

template<typename... PinConfig, typename... ValidPorts>
struct config_helper<mcutl::gpio::config<PinConfig...>,
	valid_gpio_list<ValidPorts...>>
{
	template<char PortLetter>
	static constexpr port_bit_data get_port_bit_data() noexcept
	{
		port_bit_data data;
		(..., pin_helper<PinConfig>::apply(data, PortLetter));
		return data;
	}
	
	template<char PortLetter>
	static void configure() MCUTL_NOEXCEPT
	{
		constexpr auto data = get_port_bit_data<PortLetter>();
		constexpr auto port_base = get_port_base<PortLetter>();
		
		mcutl::memory::set_register_bits<
			static_cast<uint32_t>(data.cr_changed_bits & (std::numeric_limits<uint32_t>::max)()),
			static_cast<uint32_t>(data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()),
			&GPIO_TypeDef::CRL, port_base>();
		mcutl::memory::set_register_bits<
			static_cast<uint32_t>((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()),
			static_cast<uint32_t>((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()),
			&GPIO_TypeDef::CRH, port_base>();
		if constexpr (data.dr_set_bits || data.dr_reset_bits)
		{
			mcutl::memory::set_register_value<
				(static_cast<uint32_t>(data.dr_set_bits) << GPIO_BSRR_BS0_Pos)
					| static_cast<uint32_t>(data.dr_reset_bits) << GPIO_BSRR_BR0_Pos,
				&GPIO_TypeDef::BSRR, port_base>();
		}
	}
	
	static void configure() MCUTL_NOEXCEPT
	{
		(..., configure<ValidPorts::port_letter>());
	}
};

template<typename PinConfig>
void configure_gpio() MCUTL_NOEXCEPT
{
	config_helper<PinConfig>::configure();
}

template<bool NegateBits, typename... Pins>
auto get_input_values_mask() MCUTL_NOEXCEPT
{
	constexpr auto port_letter = pin_port_letter_helper<Pins...>::get_first_pin_port_letter();
	constexpr auto port_base = get_port_base<port_letter>();
	constexpr auto pin_bit_mask = get_pin_bit_mask<Pins...>();
	if constexpr (NegateBits)
		return ~mcutl::memory::get_register_bits<&GPIO_TypeDef::IDR, port_base>() & pin_bit_mask;
	else
		return mcutl::memory::get_register_bits<pin_bit_mask, &GPIO_TypeDef::IDR, port_base>();
}

template<bool NegateBits, typename... Pins>
auto get_output_values_mask() MCUTL_NOEXCEPT
{
	constexpr auto port_letter = pin_port_letter_helper<Pins...>::get_first_pin_port_letter();
	constexpr auto port_base = get_port_base<port_letter>();
	constexpr auto pin_bit_mask = get_pin_bit_mask<Pins...>();
	if constexpr (NegateBits)
		return ~mcutl::memory::get_register_bits<&GPIO_TypeDef::ODR, port_base>() & pin_bit_mask;
	else
		return mcutl::memory::get_register_bits<pin_bit_mask, &GPIO_TypeDef::ODR, port_base>();
}

template<typename Pin>
bool is_output() MCUTL_NOEXCEPT
{
	constexpr auto port_letter = Pin::port_letter;
	constexpr auto port_base = get_port_base<port_letter>();
	if constexpr (Pin::pin_number < 8)
	{
		constexpr uint32_t pin_mode_mask = (GPIO_CRL_MODE0 << (Pin::pin_number * defines::conf_and_mode_bit_count));
		return mcutl::memory::get_register_bits<pin_mode_mask, &GPIO_TypeDef::CRL, port_base>() != defines::input_mode;
	}
	else
	{
		constexpr uint32_t pin_mode_mask = (GPIO_CRH_MODE8 << ((Pin::pin_number - 8) * defines::conf_and_mode_bit_count));
		return mcutl::memory::get_register_bits<pin_mode_mask, &GPIO_TypeDef::CRH, port_base>() != defines::input_mode;
	}
}

template<typename PinConfig>
[[nodiscard]] bool is() MCUTL_NOEXCEPT
{
	constexpr auto data = config_helper<mcutl::gpio::config<PinConfig>, available_regs_t>
		::template get_port_bit_data<PinConfig::pin::port_letter>();
	
	constexpr auto port_base = get_port_base<PinConfig::pin::port_letter>();
	if constexpr ((data.cr_changed_bits & (std::numeric_limits<uint32_t>::max)()) != 0)
	{
		if (mcutl::memory::get_register_bits<data.cr_changed_bits, &GPIO_TypeDef::CRL, port_base>()
			!= (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
	}
	else if constexpr (((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()) != 0)
	{
		if (mcutl::memory::get_register_bits<
			static_cast<uint32_t>((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()),
			&GPIO_TypeDef::CRH, port_base>()
				!= ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
	}
	
	if constexpr (data.dr_set_bits != 0 || data.dr_reset_bits != 0)
	{
		constexpr auto pin_type = pin_helper<PinConfig>::type;
		if constexpr (pin_type == config_type::input)
		{
			if (mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
				&GPIO_TypeDef::IDR, port_base>() != data.dr_set_bits)
			{
				return false;
			}
		}
		else if constexpr (pin_type == config_type::output)
		{
			if (mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
				&GPIO_TypeDef::ODR, port_base>() != data.dr_set_bits)
			{
				return false;
			}
		}
		else
		{
			auto dr = is_output<typename PinConfig::pin>() ? &GPIO_TypeDef::ODR : &GPIO_TypeDef::IDR;
			if (mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
				port_base>(dr) != data.dr_set_bits)
			{
				return false;
			}
		}
	}
	
	return true;
}

struct port_bit_data_ex : port_bit_data
{
	uint32_t pin_type_set_count = 0;
	uint32_t pin_mode_set_count = 0;
	uint32_t dr_set_count = 0;
	bool input_category = false;
};

template<typename Pin, typename Option>
struct single_option_helper
{
	static constexpr void apply(port_bit_data_ex&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unsupported gpio::has() option");
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::keep_value>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		++data.dr_set_count;
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::one>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		++data.dr_set_count;
		data.dr_set_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::zero>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		++data.dr_set_count;
		data.dr_reset_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
constexpr void apply_pin_type_option(port_bit_data_ex& data, uint64_t conf) noexcept
{
	apply_cr_bitmask<Pin>(data);
	++data.pin_type_set_count;
	data.cr_bit_values |= conf << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::open_drain>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::open_drain_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::open_drain_alt_func>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::open_drain_alt_func_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::push_pull>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::push_pull_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::push_pull_alt_func>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::push_pull_alt_func_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::analog>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_analog_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::floating>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_floating_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::pull_down>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_pull_down_conf << defines::conf_bits_offset);
		++data.dr_set_count;
		data.dr_reset_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::pull_up>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_pull_up_conf << defines::conf_bits_offset);
		++data.dr_set_count;
		data.dr_set_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
constexpr void apply_pin_frequency_option(port_bit_data_ex& data, uint64_t mode) noexcept
{
	apply_cr_bitmask<Pin>(data);
	++data.pin_mode_set_count;
	data.cr_bit_values |= mode << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::opt::freq_2mhz>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_frequency_option<Pin>(data, defines::output_freq_2mhz_mode << defines::mode_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::opt::freq_10mhz>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_frequency_option<Pin>(data, defines::output_freq_10mhz_mode << defines::mode_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::opt::freq_50mhz>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_frequency_option<Pin>(data, defines::output_freq_50mhz_mode << defines::mode_bits_offset);
	}
};

template<typename Pin, typename Option, typename... Options>
constexpr port_bit_data_ex get_options_description() noexcept
{
	port_bit_data_ex data {};
	using options_category = mcutl::gpio::detail::option_category_t<Option>;
	single_option_helper<Pin, Option>::apply(data);
	(..., single_option_helper<Pin, Options>::apply(data));
	if constexpr (std::is_same_v<options_category, mcutl::gpio::detail::in_option>)
	{
		data.input_category = true;
		apply_pin_frequency_option<Pin>(data,
			defines::input_mode << defines::mode_bits_offset);
	}
	
	return data;
}

template<typename Pin>
constexpr uint64_t get_mode_bitmask() noexcept
{
	uint64_t mode_bit_mask = ((1ull << defines::mode_bit_count) - 1ull) << defines::mode_bits_offset;
	return mode_bit_mask << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin>
constexpr uint64_t get_conf_bitmask() noexcept
{
	uint64_t conf_bit_mask = ((1ull << defines::conf_bit_count) - 1ull) << defines::conf_bits_offset;
	return conf_bit_mask << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin, typename Option, typename... Options>
[[nodiscard]] bool has() MCUTL_NOEXCEPT
{
	constexpr port_bit_data_ex data = get_options_description<Pin, Option, Options...>();
	[[maybe_unused]] constexpr auto port_base = get_port_base<Pin::port_letter>();
	
	static_assert(data.pin_mode_set_count <= 1, "Pin frequency options conflict");
	static_assert(data.dr_set_count <= 1, "Pin value options conflict");
	static_assert(data.pin_type_set_count <= 1, "Pin type options conflict");
	
	if constexpr (!data.cr_changed_bits && (data.dr_set_bits || data.dr_reset_bits))
	{
		return is_output<Pin>() && mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
			&GPIO_TypeDef::ODR, port_base>() == data.dr_set_bits;
	}
	else if constexpr (data.cr_changed_bits != 0)
	{
		if constexpr ((data.cr_changed_bits & (std::numeric_limits<uint32_t>::max)()) != 0)
		{
			auto cr = mcutl::memory::get_register_bits<data.cr_changed_bits,
				&GPIO_TypeDef::CRL, port_base>();
			if constexpr (data.pin_mode_set_count && data.pin_type_set_count)
			{
				if (cr != (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()))
					return false;
			}
			else if constexpr (data.pin_type_set_count)
			{
				//Out mode, no frequency option supplied
				if ((cr & ~static_cast<uint32_t>(get_mode_bitmask<Pin>()))
						!= (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)())
					|| !(cr & static_cast<uint32_t>(get_mode_bitmask<Pin>())))
				{
					return false;
				}
			}
			else
			{
				if ((cr & ~static_cast<uint32_t>(get_conf_bitmask<Pin>()))
						!= (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()))
				{
					return false;
				}
			}
		}
		else
		{
			auto cr = mcutl::memory::get_register_bits<
				static_cast<uint32_t>((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()),
				&GPIO_TypeDef::CRH, port_base>();
			if constexpr (data.pin_mode_set_count && data.pin_type_set_count)
			{
				if (cr != ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()))
					return false;
			}
			else if constexpr (data.pin_type_set_count)
			{
				//Out mode, no frequency option supplied
				if ((cr & ~static_cast<uint32_t>(get_mode_bitmask<Pin>() >> 32u))
						!= ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)())
					|| !(cr & static_cast<uint32_t>(get_mode_bitmask<Pin>() >> 32u)))
				{
					return false;
				}
			}
			else
			{
				if ((cr & ~static_cast<uint32_t>(get_conf_bitmask<Pin>() >> 32u))
						!= ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()))
				{
					return false;
				}
			}
		}
		
		if constexpr (data.dr_set_bits || data.dr_reset_bits)
		{
			constexpr auto dr = data.input_category ? &GPIO_TypeDef::IDR : &GPIO_TypeDef::ODR;
			if (mcutl::memory::get_register_bits<
				data.dr_set_bits | data.dr_reset_bits, dr, port_base>() != data.dr_set_bits)
			{
				return false;
			}
		}
	}
	else if constexpr (data.dr_set_count != 0)
	{
		return is_output<Pin>();
	}
	
	return true;
}

} //namespace mcutl::device::gpio

