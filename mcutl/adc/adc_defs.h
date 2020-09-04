#pragma once

#include <chrono>
#include <limits>
#include <limits.h>
#include <stdint.h>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::adc
{

using channel_index_type = uint8_t;

template<channel_index_type ChannelIndex>
struct channel
{
	static constexpr auto value = ChannelIndex;
};

namespace init
{

template<bool Enable>
struct enable_peripheral {};

template<bool Enable>
struct enable {};

struct base_configuration_is_currently_present {};

template<uint64_t SysTickFrequency, typename ClockConfig>
struct wait_finished {};

template<typename Channel, uint64_t InputImpedanceOhms, typename ClockConfig>
struct input_impedance {};

namespace data_alignment
{

struct left {};
struct right {};

} //namespace data_alignment

namespace interrupt
{

struct conversion_complete {};

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};

} //namespace interrupt

} //namespace init

namespace cal
{

using init::wait_finished;

} //namespace cal

namespace detail
{

template<typename Adc, typename Interrupt>
struct interrupt_type_helper
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown ADC or interrupt type");
};

template<typename Adc, typename Channel>
struct channel_validator
{
	static_assert(types::always_false<Channel>::value,
		"Unknown ADC type or ADC channel type");
};

template<typename Peripheral, uint8_t ResolutionBits, typename ConversionResultType>
struct traits_base
{
	using peripheral = Peripheral;
	static constexpr uint8_t resolution_bits = ResolutionBits;
	using conversion_result_type = ConversionResultType;
};

template<typename Adc> struct traits
{
	static_assert(types::always_false<Adc>::value, "Invalid ADC type");
};

template<channel_index_type ChannelIndex, typename Gpio>
struct channel_map
{
	static constexpr channel_index_type channel = ChannelIndex;
	using gpio = Gpio;
};

template<typename ChannelList, typename GpioList>
struct channels_and_gpios
{
	using channels = ChannelList;
	using gpios = GpioList;
};

template<typename... ChannelElements, typename... GpioElements>
constexpr auto generate_channel_to_gpio_map_impl(
	channels_and_gpios<types::list<ChannelElements...>,
		types::list<GpioElements...>> result) noexcept
{
	return result;
}

template<typename Mapping, typename... Mappings,
	typename... ChannelElements, typename... GpioElements>
constexpr auto generate_channel_to_gpio_map_impl(
	channels_and_gpios<types::list<ChannelElements...>,
		types::list<GpioElements...>>) noexcept
{
	return generate_channel_to_gpio_map_impl<Mappings...>(
		channels_and_gpios<types::list<ChannelElements..., channel<Mapping::channel>>,
			types::list<GpioElements..., typename Mapping::gpio>> {}
	);
}

template<typename... Mappings>
constexpr auto generate_channel_to_gpio_map() noexcept
{
	return generate_channel_to_gpio_map_impl<Mappings...>(
		channels_and_gpios<types::list<>, types::list<>> {});
}

template<typename... Mappings>
using channels_and_gpios_result_type = decltype(generate_channel_to_gpio_map<Mappings...>());

template<typename Adc>
struct channel_gpio_map
{
	static_assert(types::always_false<Adc>::value, "No GPIO map for specified ADC");
};

template<typename Channel, typename ChannelGpioMap>
using gpio_by_channel = types::type_by_index_t<types::type_index_v<Channel,
		typename ChannelGpioMap::type::channels>, typename ChannelGpioMap::type::gpios>;

template<typename Gpio, typename ChannelGpioMap>
using channel_by_gpio = types::type_by_index_t<types::type_index_v<Gpio,
		typename ChannelGpioMap::type::gpios>, typename ChannelGpioMap::type::channels>;

struct data_alignment
{
	enum value { left, right, end };
};

struct cal_options
{
	uint64_t wait_time = 0;
	uint64_t systick_frequency = 0;
	uint32_t wait_time_set_count = 0;
};

struct conv_options
{
	channel_index_type channel_index = 0;
	uint32_t channel_index_set_count = 0;
};

struct init_options
{
	mcutl::interrupt::detail::interrupt_info conversion_complete;
	uint64_t priority_count = 0;
	channel_index_type channel_index = 0;
	bool enable = false;
	bool enable_peripheral = false;
	uint64_t wait_time = 0;
	uint64_t systick_frequency = 0;
	data_alignment::value alignment = data_alignment::right;
	uint64_t sample_time[(std::numeric_limits<channel_index_type>::max)()] {};
	
	uint32_t channel_index_set_count = 0;
	uint32_t conversion_complete_set_count = 0;
	uint32_t enable_set_count = 0;
	uint32_t enable_peripheral_set_count = 0;
	uint32_t wait_time_set_count = 0;
	uint32_t alignment_set_count = 0;
	uint32_t enable_controller_interrupts_set_count = 0;
	uint32_t disable_controller_interrupts_set_count = 0;
	uint32_t priority_count_set_count = 0;
	uint32_t sample_time_set_count[(std::numeric_limits<channel_index_type>::max)()] {};
	uint32_t base_configuration_set_count = 0;
};

template<typename Adc, typename ClockConfig>
struct initialization_time
{
	static_assert(types::always_false<ClockConfig>::value,
		"Unknown ADC initialization time");
};

template<typename Adc, typename ClockConfig>
struct calibration_time
{
	static_assert(types::always_false<ClockConfig>::value,
		"Unknown ADC calibration time");
};

template<typename Adc, typename Channel, uint64_t InputImpedanceOhms, typename ClockConfig>
struct input_impedance_to_sample_time
{
	static_assert(types::always_false<ClockConfig>::value,
		"Unknown ADC sample time");
};

template<typename Adc, typename Option>
struct init_options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value,
			"Unknown ADC option");
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda) noexcept
	{
	}
};

template<typename Adc, typename Option>
struct cal_options_parser : init_options_parser<Adc, Option> {};
template<typename Adc, typename Option>
struct conv_options_parser : init_options_parser<Adc, Option> {};

template<typename Adc, channel_index_type ChannelIndex>
struct conv_options_parser<Adc, channel<ChannelIndex>>
	: channel_validator<Adc, channel<ChannelIndex>>
	, opts::base_option_parser<ChannelIndex,
		&conv_options::channel_index, &conv_options::channel_index_set_count> {};

template<uint64_t SysTickFrequency, typename WaitTime, typename Options>
static constexpr void parse_wait_time(Options& options) noexcept
{
	options.systick_frequency = SysTickFrequency;
	options.wait_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
		typename WaitTime::duration_type(WaitTime::interval)).count();
	++options.wait_time_set_count;
}

template<typename Adc, uint64_t SysTickFrequency, typename ClockConfig>
struct cal_options_parser<Adc, cal::wait_finished<SysTickFrequency, ClockConfig>>
	: opts::base_option_parser<0, nullptr, &cal_options::wait_time_set_count>
{
	template<typename Option>
	static constexpr void parse(Option& options) noexcept
	{
		parse_wait_time<SysTickFrequency, typename calibration_time<Adc, ClockConfig>::type>(options);
	}
};

template<typename Adc, channel_index_type ChannelIndex>
struct init_options_parser<Adc, channel<ChannelIndex>>
	: channel_validator<Adc, channel<ChannelIndex>>
	, opts::base_option_parser<ChannelIndex,
		&init_options::channel_index, &init_options::channel_index_set_count> {};

template<typename Adc, uint64_t SysTickFrequency, typename ClockConfig>
struct init_options_parser<Adc, init::wait_finished<SysTickFrequency, ClockConfig>>
	: opts::base_option_parser<0, nullptr, &init_options::wait_time_set_count>
{
	template<typename Option>
	static constexpr void parse(Option& options) noexcept
	{
		parse_wait_time<SysTickFrequency, typename initialization_time<Adc, ClockConfig>::type>(options);
	}
};

template<typename Adc, bool Enable>
struct init_options_parser<Adc, init::enable<Enable>>
	: opts::base_option_parser<Enable,
		&init_options::enable, &init_options::enable_set_count> {};

template<typename Adc, bool Enable>
struct init_options_parser<Adc, init::enable_peripheral<Enable>>
	: opts::base_option_parser<Enable,
		&init_options::enable_peripheral, &init_options::enable_peripheral_set_count> {};

template<typename Adc, typename Channel, uint64_t InputImpedanceOhms, typename ClockConfig>
struct init_options_parser<Adc, init::input_impedance<Channel, InputImpedanceOhms, ClockConfig>>
	: channel_validator<Adc, Channel>
	, opts::base_array_option_parser<
		input_impedance_to_sample_time<Adc, Channel, InputImpedanceOhms, ClockConfig>::value,
		Channel::value, &init_options::sample_time, &init_options::sample_time_set_count> {};

template<typename Adc>
struct init_options_parser<Adc, init::data_alignment::left>
	: opts::base_option_parser<data_alignment::left,
		&init_options::alignment, &init_options::alignment_set_count> {};

template<typename Adc>
struct init_options_parser<Adc, init::data_alignment::right>
	: opts::base_option_parser<data_alignment::right,
		&init_options::alignment, &init_options::alignment_set_count> {};

template<typename Interrupt>
struct interrupt_map : mcutl::interrupt::detail::map<Interrupt> {};

template<> struct interrupt_map<init::interrupt::conversion_complete>
	: mcutl::interrupt::detail::map_base<&init_options::conversion_complete_set_count,
		&init_options::conversion_complete> {};

template<typename Adc>
struct init_options_parser<Adc, init::interrupt::conversion_complete>
	: opts::base_option_parser<0, nullptr, &init_options::conversion_complete_set_count> {};

template<typename Adc, typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct init_options_parser<Adc, mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: mcutl::interrupt::detail::interrupt_parser<Interrupt, interrupt_map, Priority, SubPriority> {};

template<typename Adc, typename Interrupt>
struct init_options_parser<Adc, mcutl::interrupt::disabled<Interrupt>>
	: mcutl::interrupt::detail::interrupt_parser<mcutl::interrupt::disabled<Interrupt>, interrupt_map> {};

template<typename Adc>
struct init_options_parser<Adc, init::interrupt::enable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &init_options::enable_controller_interrupts_set_count> {};

template<typename Adc>
struct init_options_parser<Adc, init::interrupt::disable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &init_options::disable_controller_interrupts_set_count> {};

template<typename Adc, auto PriorityCount>
struct init_options_parser<Adc, mcutl::interrupt::priority_count<PriorityCount>>
	: opts::base_option_parser<PriorityCount, &init_options::priority_count,
		&init_options::priority_count_set_count> {};

template<typename Adc>
struct init_options_parser<Adc, init::base_configuration_is_currently_present>
	: opts::base_option_parser<0, nullptr, &init_options::base_configuration_set_count> {};

} //namespace detail

template<typename Adc>
using peripheral_type = typename detail::traits<Adc>::peripheral;

template<typename Adc, typename Interrupt>
using interrupt_type = typename detail::interrupt_type_helper<Adc, Interrupt>::type;

template<typename Adc>
using conversion_result_type = typename detail::traits<Adc>::conversion_result_type;

template<typename Adc>
[[maybe_unused]] constexpr auto resolution_bits = detail::traits<Adc>::resolution_bits;

template<typename Adc>
[[maybe_unused]] constexpr conversion_result_type<Adc> max_result_value
	= (std::numeric_limits<conversion_result_type<Adc>>::max)()
	>> (CHAR_BIT * sizeof(conversion_result_type<Adc>) - resolution_bits<Adc>);

template<typename Adc, typename Channel>
using map_adc_channel_to_gpio = detail::gpio_by_channel<Channel, detail::channel_gpio_map<Adc>>;
template<typename Adc, typename Gpio>
using map_gpio_to_adc_channel = detail::channel_by_gpio<Gpio, detail::channel_gpio_map<Adc>>;

template<typename Adc, typename ClockConfig>
using initialization_time = typename detail::initialization_time<Adc, ClockConfig>::type;

template<typename Adc, typename ClockConfig>
using calibration_time = typename detail::calibration_time<Adc, ClockConfig>::type;

} //namespace mcutl::adc
