#pragma once

#include <limits>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include "mcutl/adc/adc_defs.h"
#include "mcutl/clock/clock.h"
#include "mcutl/device/device.h"
#include "mcutl/dma/dma.h"
#include "mcutl/gpio/gpio.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/systick/systick_wait.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/duration.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::adc
{

using gpio_config = mcutl::gpio::in::analog;
[[maybe_unused]] constexpr uint8_t max_channels = 18;
[[maybe_unused]] constexpr uint8_t max_scan_channels = 16;

template<typename Adc>
[[maybe_unused]] constexpr bool supports_calibration = true;
template<typename Adc>
[[maybe_unused]] constexpr bool supports_input_impedance_option = true;
template<typename Adc>
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags = true;
template<typename Adc>
[[maybe_unused]] constexpr bool supports_data_alignment = true;

template<uint8_t Index, typename Peripheral, uint8_t ResolutionBits, typename ConversionResultType,
	uint32_t Base, mcutl::interrupt::irqn_t Irqn>
struct adc_base : mcutl::adc::detail::traits_base<Peripheral, ResolutionBits, ConversionResultType>
{
	static constexpr auto base = Base;
	static constexpr auto index = Index;
	static constexpr auto irqn = Irqn;
};

struct scan_options
{
	uint32_t scan_channels_set_count = 0;
	uint8_t channel_order[max_scan_channels] {};
	uint8_t total_channel_count = 0;
};

struct init_options : mcutl::adc::detail::init_options, scan_options
{
};

struct conv_options : mcutl::adc::detail::conv_options, scan_options
{
};

struct cal_options : mcutl::adc::detail::cal_options
{
};

} //namespace mcutl::device::adc

namespace mcutl::adc
{

#ifdef ADC1
using adc1 = device::adc::adc_base<1, mcutl::periph::adc1, 12, uint16_t, ADC1_BASE, ADC1_2_IRQn>;
#endif //ADC1

#ifdef ADC2
using adc2 = device::adc::adc_base<2, mcutl::periph::adc2, 12, uint16_t, ADC2_BASE, ADC1_2_IRQn>;
#endif //ADC2

#ifdef ADC3
using adc3 = device::adc::adc_base<3, mcutl::periph::adc3, 12, uint16_t, ADC3_BASE, ADC3_IRQn>;
#endif //ADC3

namespace detail
{

struct initialization_time_base : types::identity<types::microseconds<1>> {}; //tSTAB = 1uS

template<typename ClockConfig>
struct calibration_time_base
{
	using type = types::microseconds<1 + 83'000'000 / mcutl::clock::get_clock_info<ClockConfig,
		device::clock::device_source_id::adc>().get_exact_frequency()>; //tCAL = 83/fADC
};

template<typename Channel>
struct channel_validator_base
{
	static_assert(Channel::value < device::adc::max_channels, "Invalid ADC channel index");
};

template<typename Adc>
struct dma_channel_helper
{
	static_assert(types::always_false<Adc>::value,
		"Selected ADC is not mapped to any DMA channels");
};

#ifdef ADC1
template<>
struct dma_channel_helper<adc1> : types::identity<mcutl::dma::dma1<1>> {};
template<> struct traits<adc1> : adc1 {};
template<>
struct interrupt_type_helper<adc1, mcutl::adc::init::interrupt::conversion_complete>
	: types::identity<mcutl::interrupt::type::adc1_2> {};
template<typename ClockConfig>
struct initialization_time<adc1, ClockConfig> : initialization_time_base {};
template<typename ClockConfig>
struct calibration_time<adc1, ClockConfig> : calibration_time_base<ClockConfig> {};
template<>
struct channel_gpio_map<adc1>
{
	using type = channels_and_gpios_result_type<
		channel_map<10, gpio::gpioc<0>>,
		channel_map<11, gpio::gpioc<1>>,
		channel_map<12, gpio::gpioc<2>>,
		channel_map<13, gpio::gpioc<3>>,
		channel_map<14, gpio::gpioc<4>>,
		channel_map<15, gpio::gpioc<5>>,
		channel_map<0, gpio::gpioa<0>>,
		channel_map<1, gpio::gpioa<1>>,
		channel_map<2, gpio::gpioa<2>>,
		channel_map<3, gpio::gpioa<3>>,
		channel_map<4, gpio::gpioa<4>>,
		channel_map<5, gpio::gpioa<5>>,
		channel_map<6, gpio::gpioa<6>>,
		channel_map<7, gpio::gpioa<7>>,
		channel_map<8, gpio::gpiob<0>>,
		channel_map<9, gpio::gpiob<1>>
	>;
};
template<typename Channel>
struct channel_validator<adc1, Channel> : channel_validator_base<Channel> {};
#endif //ADC1

#ifdef ADC2
template<> struct traits<adc2> : adc2 {};
template<>
struct interrupt_type_helper<adc2, mcutl::adc::init::interrupt::conversion_complete>
	: types::identity<mcutl::interrupt::type::adc1_2> {};
template<typename ClockConfig>
struct initialization_time<adc2, ClockConfig> : initialization_time_base {};
template<typename ClockConfig>
struct calibration_time<adc2, ClockConfig> : calibration_time_base<ClockConfig> {};
template<>
struct channel_gpio_map<adc2>
{
	using type = channels_and_gpios_result_type<
		channel_map<10, gpio::gpioc<0>>,
		channel_map<11, gpio::gpioc<1>>,
		channel_map<12, gpio::gpioc<2>>,
		channel_map<13, gpio::gpioc<3>>,
		channel_map<14, gpio::gpioc<4>>,
		channel_map<15, gpio::gpioc<5>>,
		channel_map<0, gpio::gpioa<0>>,
		channel_map<1, gpio::gpioa<1>>,
		channel_map<2, gpio::gpioa<2>>,
		channel_map<3, gpio::gpioa<3>>,
		channel_map<4, gpio::gpioa<4>>,
		channel_map<5, gpio::gpioa<5>>,
		channel_map<6, gpio::gpioa<6>>,
		channel_map<7, gpio::gpioa<7>>,
		channel_map<8, gpio::gpiob<0>>,
		channel_map<9, gpio::gpiob<1>>
	>;
};
template<typename Channel>
struct channel_validator<adc2, Channel> : channel_validator_base<Channel> {};
#endif //ADC2

#ifdef ADC3
template<>
struct dma_channel_helper<adc3> : types::identity<mcutl::dma::dma2<5>> {};
template<> struct traits<adc3> : adc3 {};
template<>
struct interrupt_type_helper<adc3, mcutl::adc::init::interrupt::conversion_complete>
	: types::identity<mcutl::interrupt::type::adc3> {};
template<typename ClockConfig>
struct initialization_time<adc3, ClockConfig> : initialization_time_base {};
template<typename ClockConfig>
struct calibration_time<adc3, ClockConfig> : calibration_time_base<ClockConfig> {};
template<>
struct channel_gpio_map<adc3>
{
	using type = channels_and_gpios_result_type<
		channel_map<4, gpio::gpiof<6>>,
		channel_map<5, gpio::gpiof<7>>,
		channel_map<6, gpio::gpiof<8>>,
		channel_map<7, gpio::gpiof<9>>,
		channel_map<8, gpio::gpiof<10>>,
		channel_map<10, gpio::gpioc<0>>,
		channel_map<11, gpio::gpioc<1>>,
		channel_map<12, gpio::gpioc<2>>,
		channel_map<13, gpio::gpioc<3>>,
		channel_map<0, gpio::gpioa<0>>,
		channel_map<1, gpio::gpioa<1>>,
		channel_map<2, gpio::gpioa<2>>,
		channel_map<3, gpio::gpioa<3>>
	>;
};
template<typename Channel>
struct channel_validator<adc3, Channel> : channel_validator_base<Channel> {};
#endif //ADC3

} //namespace detail

template<typename DmaChannel, typename... DmaTraits>
struct dma_channel_config
{
	using dma_channel = DmaChannel;
};

template<typename Adc>
using dma_channel = typename detail::dma_channel_helper<Adc>::type;

namespace init
{

template<typename DmaChannelConfig, typename Channel, typename... Channels>
struct scan_channels {};

namespace sample_time_cycles
{

struct cycles_1_5 {};
struct cycles_7_5 {};
struct cycles_13_5 {};
struct cycles_28_5 {};
struct cycles_41_5 {};
struct cycles_55_5 {};
struct cycles_71_5 {};
struct cycles_239_5 {};
	
} //namespace sample_time_cycles

template<typename Channel, typename SampleTimeCycles>
struct sample_time {};

} //namespace init

namespace conv
{

using init::scan_channels;

} //namespace conv

namespace detail
{

template<uint64_t InputImpedanceOhms, typename ClockConfig>
constexpr auto select_best_sample_time() noexcept
{
	static_assert(InputImpedanceOhms <= 50'000, "Too large input impedance for ADC");
	
	constexpr uint64_t f_adc = mcutl::clock::get_clock_info<ClockConfig,
		mcutl::device::clock::device_source_id::adc>().get_exact_frequency();
	constexpr uint64_t r_adc = 1'000;
	constexpr uint64_t c_adc = 8;
	constexpr uint64_t c_adc_div = 1'000'000'000'000ull;
	constexpr uint64_t freq_ln = 9'704;
	constexpr uint64_t freq_ln_div = 1'000;
	
	using namespace init::sample_time_cycles;
	constexpr uint64_t cycles = 10 * (InputImpedanceOhms + r_adc)
		* f_adc * c_adc * freq_ln / c_adc_div / freq_ln_div;
	static_assert(cycles <= 2395, "Invalid ADC configuration for selected input impedance");
	if constexpr (cycles <= 15)
		return cycles_1_5 {};
	else if constexpr (cycles <= 75)
		return cycles_7_5 {};
	else if constexpr (cycles <= 135)
		return cycles_13_5 {};
	else if constexpr (cycles <= 285)
		return cycles_28_5 {};
	else if constexpr (cycles <= 415)
		return cycles_41_5 {};
	else if constexpr (cycles <= 555)
		return cycles_55_5 {};
	else if constexpr (cycles <= 715)
		return cycles_71_5 {};
	else if constexpr (cycles <= 2395)
		return cycles_239_5 {};
	else
		return cycles_239_5 {};
}

template<typename SampleTimeCycles>
struct sample_time_to_reg_value_map
{
	static_assert(types::always_false<SampleTimeCycles>::value,
		"Invalid ADC sample time");
};

template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_1_5>
	: std::integral_constant<uint8_t, 0> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_7_5>
	: std::integral_constant<uint8_t, 1> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_13_5>
	: std::integral_constant<uint8_t, 2> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_28_5>
	: std::integral_constant<uint8_t, 3> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_41_5>
	: std::integral_constant<uint8_t, 4> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_55_5>
	: std::integral_constant<uint8_t, 5> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_71_5>
	: std::integral_constant<uint8_t, 6> {};
template<> struct sample_time_to_reg_value_map<init::sample_time_cycles::cycles_239_5>
	: std::integral_constant<uint8_t, 7> {};

template<uint8_t Index, typename Peripheral, uint8_t ResolutionBits, typename ConversionResultType,
	uint32_t Base, mcutl::interrupt::irqn_t Irqn,
	typename Channel, uint64_t InputImpedanceOhms, typename ClockConfig>
struct input_impedance_to_sample_time<
	device::adc::adc_base<Index, Peripheral, ResolutionBits, ConversionResultType, Base, Irqn>,
	Channel, InputImpedanceOhms, ClockConfig>
{
	static constexpr uint64_t value = sample_time_to_reg_value_map<
		decltype(select_best_sample_time<InputImpedanceOhms, ClockConfig>())>::value;
};

template<typename Adc, typename Channel, typename SampleTimeCycles>
struct init_options_parser<Adc, init::sample_time<Channel, SampleTimeCycles>>
	: channel_validator<Adc, Channel>
	, opts::base_array_option_parser<sample_time_to_reg_value_map<SampleTimeCycles>::value, Channel::value,
		&device::adc::init_options::sample_time, &init_options::sample_time_set_count> {};

template<typename Adc, typename DmaChannel, typename... DmaTraits, typename... Channels>
struct init_options_parser<Adc, init::scan_channels<dma_channel_config<DmaChannel, DmaTraits...>, Channels...>>
	: opts::base_option_parser<0, nullptr, &device::adc::scan_options::scan_channels_set_count>
{
public:
	template<typename Options>
	static constexpr void parse(Options& options) noexcept
	{
		validate_channels(channel_validator<Adc, Channels>{}...);
		static_assert(sizeof...(Channels) < device::adc::max_scan_channels,
			"Too many channels for SCAN conversion");
		static_assert(Adc::index == 1 || Adc::index == 3, "Only ADC1 and ADC3 can generate DMA requests");
		
		++options.scan_channels_set_count;
		options.total_channel_count = sizeof...(Channels);
		set_channel_order<0, Channels::value...>(options);
	}
	
private:
	template<uint8_t Index, typename Options>
	static constexpr void set_channel_order(Options&) noexcept
	{
	}
	
	template<uint8_t Index, uint8_t ChannelNumber, uint8_t... ChannelNumbers, typename Options>
	static constexpr void set_channel_order(Options& options) noexcept
	{
		options.channel_order[Index] = ChannelNumber;
		set_channel_order<Index + 1, ChannelNumbers...>(options);
	}
	
	template<typename... T>
	static constexpr void validate_channels(T...) noexcept
	{
	}
};

template<typename Adc, typename DmaChannel, typename... DmaTraits, typename... Channels>
struct conv_options_parser<Adc, conv::scan_channels<dma_channel_config<DmaChannel, DmaTraits...>, Channels...>>
	: init_options_parser<Adc, init::scan_channels<dma_channel_config<DmaChannel, DmaTraits...>, Channels...>>
{
};

} //namespace detail

} //namespace mcu::adc

namespace mcutl::device::adc
{

template<typename Adc, typename... Flags>
void clear_pending_flags() MCUTL_NOEXCEPT
{
	static_assert(sizeof...(Flags) <= 1,
		"Duplicate or unsupported interrupts for ADC clear_pending_flags");
	if constexpr (sizeof...(Flags) == 1)
	{
		static_assert(std::is_same_v<types::first_type_t<Flags...>,
			mcutl::adc::init::interrupt::conversion_complete>,
			"Unsupported interrupt for ADC clear_pending_flags");
		
		mcutl::memory::set_register_value<ADC_SR_STRT | ADC_SR_JSTRT | ADC_SR_JEOC | ADC_SR_AWD,
			&ADC_TypeDef::SR, Adc::base>();
	}
}

template<typename Adc>
constexpr bool is_valid_adc() noexcept
{
#ifdef ADC1
	if constexpr (std::is_same_v<Adc, mcutl::adc::adc1>)
		return true;
#endif //ADC1
#ifdef ADC2
	if constexpr (std::is_same_v<Adc, mcutl::adc::adc2>)
		return true;
#endif //ADC2
#ifdef ADC3
	if constexpr (std::is_same_v<Adc, mcutl::adc::adc3>)
		return true;
#endif //ADC3
	
	return false;
}

template<typename Interrupt>
struct interrupt_flags
{
	static_assert(types::always_false<Interrupt>::value, "Invalid ADC interrupt type");
};

template<>
struct interrupt_flags<mcutl::adc::init::interrupt::conversion_complete>
{
	static constexpr uint32_t value = ADC_SR_EOS_Msk;
};
	
template<typename Adc, typename... Interrupts>
struct pending_flags_helper
{
	static_assert(is_valid_adc<Adc>(), "Invalid ADC type");
	static constexpr uint32_t value = (0 | ... | interrupt_flags<Interrupts>::value);
};

template<typename Adc, typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = pending_flags_helper<Adc, Interrupts...>::value;

template<typename Adc, typename... Interrupts>
inline auto get_pending_flags() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_bits<pending_flags_v<Adc, Interrupts...>,
		&ADC_TypeDef::SR, Adc::base>();
}

template<typename Adc, typename... Flags>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	clear_pending_flags<Adc, Flags...>();
}

template<typename Adc>
inline bool is_calibration_finished() MCUTL_NOEXCEPT
{
	return !mcutl::memory::get_register_bits<ADC_CR2_CAL_Msk, &ADC_TypeDef::CR2, Adc::base>();
}

template<typename Adc, typename... Options, typename OptionsLambda>
void calibrate(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	auto cr2_value = mcutl::memory::get_register_bits<&ADC_TypeDef::CR2, Adc::base>();
	if (!(cr2_value & ADC_CR2_CAL))
		mcutl::memory::set_register_value<&ADC_TypeDef::CR2, Adc::base>(cr2_value | ADC_CR2_CAL);
	
	if constexpr (options_lambda().wait_time_set_count != 0)
	{
		while (!is_calibration_finished<Adc>())
		{
		}
	}
}

struct smpr_values
{
	uint32_t smpr1 = 0;
	uint32_t smpr1_mask = 0;
	uint32_t smpr2 = 0;
	uint32_t smpr2_mask = 0;
};

template<typename OptionsLambda>
constexpr smpr_values calc_smpr_registers(OptionsLambda options_lambda) noexcept
{
#if !defined(ADC_SMPR2_SMP9) || !defined(ADC_SMPR1_SMP10)
	static_assert(false, "Unexpected layout of ADC SMPR1/2 register bits");
#endif
	
	constexpr auto options = options_lambda();
	
	smpr_values result {};
	for (uint8_t i = 0; i != max_channels; ++i)
	{
		if (!options.sample_time_set_count[i])
			continue;
		
		uint8_t sample_time = options.sample_time[i];
		if (i <= 9)
		{
			result.smpr2 |= sample_time << (3 * i);
			result.smpr2_mask |= (0b111 << (3 * i));
		}
		else
		{
			result.smpr1 |= sample_time << (3 * (i - 10));
			result.smpr1_mask |= (0b111 << (3 * (i - 10)));
		}
	}
	
	return result;
}

struct sqr_values
{
	uint32_t sqr[3] {};
	uint32_t sqr_mask[3] {};
};

constexpr void add_channel_index(uint8_t order, sqr_values& result, uint8_t channel) noexcept
{
#if !defined(ADC_SQR3_SQ6) || !defined(ADC_SQR2_SQ12) || !defined(ADC_SQR1_SQ16)
	static_assert(false, "Unexpected layout of ADC SQR1/2/3 register bits");
#endif
	
	uint8_t sqr_index = 2;
	if (order >= 12)
	{
		sqr_index = 0;
		order -= 12;
	}
	else if (order >= 6)
	{
		sqr_index = 1;
		order -= 6;
	}
	
	order *= 5;
	result.sqr[sqr_index] |= channel << order;
	result.sqr_mask[sqr_index] |= 0b11111 << order;
}

template<typename OptionsLambda>
constexpr sqr_values calc_sqr_registers(OptionsLambda options_lambda) noexcept
{
	constexpr auto options = options_lambda();
	
	sqr_values result {};
	if constexpr (!!options.channel_index_set_count)
	{
		result.sqr_mask[0] |= ADC_SQR1_L_Msk;
		result.sqr[0] |= 1 << ADC_SQR1_L_Pos;
		add_channel_index(0, result, options.channel_index);
	}
	else if constexpr (!!options.scan_channels_set_count)
	{
		result.sqr_mask[0] |= ADC_SQR1_L_Msk;
		result.sqr[0] |= options.total_channel_count << ADC_SQR1_L_Pos;
		for (uint8_t i = 0; i != options.total_channel_count; ++i)
			add_channel_index(i, result, options.channel_order[i]);
	}
	
	return result;
}

struct cr_values
{
	uint32_t cr1 = 0;
	uint32_t cr1_mask = 0;
	uint32_t cr2 = 0;
	uint32_t cr2_mask = 0;
};

template<typename OptionsLambda>
constexpr cr_values calc_cr_registers(OptionsLambda options_lambda) noexcept
{
	constexpr auto options = options_lambda();
	
	cr_values result {};
	
	if constexpr (options.scan_channels_set_count || options.channel_index_set_count)
	{
		result.cr1_mask |= ADC_CR1_SCAN_Msk;
		result.cr2_mask |= ADC_CR2_DMA_Msk;
	}
	
	if constexpr (!!options.scan_channels_set_count)
	{
		result.cr1 |= ADC_CR1_SCAN_Msk;
		result.cr2 |= ADC_CR2_DMA_Msk;
	}
	
	if constexpr (!!options.conversion_complete_set_count)
	{
		result.cr1_mask |= ADC_CR1_EOSIE_Msk;
		
		if constexpr (!options.conversion_complete.disable)
			result.cr1 |= ADC_CR1_EOSIE;
	}
	
	if constexpr (!!options.alignment_set_count)
	{
		result.cr2_mask |= ADC_CR2_ALIGN_Msk;
		if constexpr (options.alignment == mcutl::adc::detail::data_alignment::left)
			result.cr2 |= ADC_CR2_ALIGN;
	}
	
	if constexpr (!!options.enable_set_count)
	{
		result.cr2_mask |= ADC_CR2_ADON_Msk;
		if constexpr (options.enable)
			result.cr2 |= ADC_CR2_ADON;
	}
	
	return result;
}

template<typename Adc, typename Option>
struct dma_config_helper
{
	static constexpr void configure() noexcept
	{
	}
};

template<typename Adc, typename DmaChannelConfig, typename... Channels>
struct dma_config_helper<Adc, mcutl::adc::conv::scan_channels<DmaChannelConfig, Channels...>>
{
	static void configure() noexcept
	{
		dma_config_helper<Adc, DmaChannelConfig>::configure();
	}
};

template<typename Adc, typename DmaChannel>
constexpr void check_dma() noexcept
{
	static_assert(Adc::index != 2, "ADC2 is not mapped to any DMA channels");
		
	if constexpr (Adc::index == 1)
	{
		static_assert(DmaChannel::dma_index == 1 && DmaChannel::channel_number == 1,
			"ADC1 is mapped to DMA1, channel 1");
	}
	else if constexpr (Adc::index == 3)
	{
		static_assert(DmaChannel::dma_index == 2 && DmaChannel::channel_number == 5,
			"ADC3 is mapped to DMA2, channel 5");
	}
}

template<typename Adc, typename DmaChannel, typename... DmaTraits>
struct dma_config_helper<Adc, mcutl::adc::dma_channel_config<DmaChannel, DmaTraits...>>
{
	static void configure() noexcept
	{
		check_dma<Adc, DmaChannel>();
		
		mcutl::dma::configure_channel<DmaChannel,
			mcutl::dma::source<mcutl::dma::data_size::word,
				mcutl::dma::address::peripheral, mcutl::dma::pointer_increment::disabled>,
			mcutl::dma::destination<mcutl::dma::data_size::halfword,
				mcutl::dma::address::memory, mcutl::dma::pointer_increment::enabled>,
			DmaTraits...
		>();
	}
};

template<typename Adc, typename... Options>
void configure_dma() noexcept
{
	(..., dma_config_helper<Adc, Options>::configure());
}

template<typename Option>
struct init_wait_helper
{
	static constexpr void wait() noexcept
	{
	}
};

template<uint64_t SysTickFrequency, typename ClockConfig>
struct init_wait_helper<mcutl::adc::init::wait_finished<SysTickFrequency, ClockConfig>>
{
	static void wait() noexcept
	{
		mcutl::systick::wait<SysTickFrequency,
			mcutl::adc::detail::initialization_time_base::type>();
	}
};

template<typename... Options>
void wait_initialization() noexcept
{
	(..., init_wait_helper<Options>::wait());
}

template<typename Adc, typename... Options, typename OptionsLambda>
void configure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto options = options_lambda();
	
	static_assert(!(options.channel_index_set_count && options.scan_channels_set_count),
		"Single conversion and SCAN mode can not be selected simultaneously");
	
	constexpr auto smpr = calc_smpr_registers(options_lambda);
	constexpr auto sqr = calc_sqr_registers(options_lambda);
	constexpr auto cr = calc_cr_registers(options_lambda);
	
	if constexpr (!!options.scan_channels_set_count)
		configure_dma<Adc, Options...>();
	
	if constexpr (options.enable_peripheral_set_count != 0)
	{
		if constexpr (options.enable_peripheral)
		{
			mcutl::periph::configure_peripheral<
				mcutl::periph::enable<mcutl::adc::peripheral_type<Adc>>>();
		}
	}
	
	if constexpr (!options.base_configuration_set_count || smpr.smpr1)
		mcutl::memory::set_register_value<smpr.smpr1, &ADC_TypeDef::SMPR1, Adc::base>();
	if constexpr (!options.base_configuration_set_count || smpr.smpr2)
		mcutl::memory::set_register_value<smpr.smpr2, &ADC_TypeDef::SMPR2, Adc::base>();
	if constexpr (!options.base_configuration_set_count || sqr.sqr[0])
		mcutl::memory::set_register_value<sqr.sqr[0], &ADC_TypeDef::SQR1, Adc::base>();
	if constexpr (!options.base_configuration_set_count || sqr.sqr[1])
		mcutl::memory::set_register_value<sqr.sqr[1], &ADC_TypeDef::SQR2, Adc::base>();
	if constexpr (!options.base_configuration_set_count || sqr.sqr[2])
		mcutl::memory::set_register_value<sqr.sqr[2], &ADC_TypeDef::SQR3, Adc::base>();
	if constexpr (!options.base_configuration_set_count || cr.cr1)
		mcutl::memory::set_register_value<cr.cr1, &ADC_TypeDef::CR1, Adc::base>();
	
	if constexpr (!(cr.cr2 & ADC_CR2_ADON))
	{
		if constexpr (!options.base_configuration_set_count || cr.cr2)
			mcutl::memory::set_register_value<cr.cr2, &ADC_TypeDef::CR2, Adc::base>();
	}
	else
	{
		auto cr2_value = mcutl::memory::get_register_bits<&ADC_TypeDef::CR2, Adc::base>();
		if (cr2_value != cr.cr2)
		{
			mcutl::memory::set_register_value<cr.cr2, &ADC_TypeDef::CR2, Adc::base>();
	
			if constexpr (options.wait_time_set_count)
				wait_initialization<Options...>();
		}
	}
	
	if constexpr (options.enable_peripheral_set_count != 0)
	{
		if constexpr (!options.enable_peripheral)
		{
			static_assert(!options.enable,
				"Can not enable ADC while disabling its peripheral");
			mcutl::periph::configure_peripheral<
				mcutl::periph::disable<mcutl::adc::peripheral_type<Adc>>>();
		}
	}
	
	if constexpr (options.enable_controller_interrupts_set_count
		&& options.conversion_complete_set_count && !options.conversion_complete.disable)
	{
		mcutl::interrupt::enable<
			mcutl::interrupt::interrupt<
				mcutl::adc::interrupt_type<Adc, mcutl::adc::init::interrupt::conversion_complete>,
				options.conversion_complete.priority,
				options.conversion_complete.subpriority
			>,
			options.priority_count_set_count ? options.priority_count : mcutl::interrupt::maximum_priorities
		>();
	}
	
	if constexpr (options.disable_controller_interrupts_set_count
		&& options.conversion_complete_set_count && options.conversion_complete.disable)
	{
		mcutl::interrupt::disable<
			mcutl::adc::interrupt_type<Adc, mcutl::adc::init::interrupt::conversion_complete>
		>();
	}
}

template<typename Adc, typename... Options, typename OptionsLambda>
void reconfigure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto options = options_lambda();
	
	static_assert(!(options.channel_index_set_count && options.scan_channels_set_count),
		"Single conversion and SCAN mode can not be selected simultaneously");
	static_assert(!options.base_configuration_set_count,
		"base_configuration_is_currently_present has no meaning for ADC reconfigure()");
	
	constexpr auto smpr = calc_smpr_registers(options_lambda);
	constexpr auto sqr = calc_sqr_registers(options_lambda);
	constexpr auto cr = calc_cr_registers(options_lambda);
	
	if constexpr (!!options.scan_channels_set_count)
		configure_dma<Adc, Options...>();
	
	if constexpr (options.enable_peripheral_set_count != 0)
	{
		if constexpr (options.enable_peripheral)
		{
			mcutl::periph::configure_peripheral<
				mcutl::periph::enable<mcutl::adc::peripheral_type<Adc>>>();
		}
	}
	
	mcutl::memory::set_register_bits<smpr.smpr1_mask, smpr.smpr1, &ADC_TypeDef::SMPR1, Adc::base>();
	mcutl::memory::set_register_bits<smpr.smpr2_mask, smpr.smpr2, &ADC_TypeDef::SMPR2, Adc::base>();
	if constexpr (sqr.sqr_mask[0] != 0)
		mcutl::memory::set_register_value<sqr.sqr[0], &ADC_TypeDef::SQR1, Adc::base>();
	if constexpr (sqr.sqr_mask[1] != 0)
		mcutl::memory::set_register_value<sqr.sqr[1], &ADC_TypeDef::SQR2, Adc::base>();
	if constexpr (sqr.sqr_mask[2] != 0)
		mcutl::memory::set_register_value<sqr.sqr[2], &ADC_TypeDef::SQR3, Adc::base>();
	
	[[maybe_unused]] uint32_t cr1_value;
	
	if constexpr (cr.cr1_mask
		|| options.enable_controller_interrupts_set_count
		|| options.disable_controller_interrupts_set_count)
	{
		cr1_value = mcutl::memory::get_register_bits<&ADC_TypeDef::CR1, Adc::base>();
		
		if constexpr (!!cr.cr1_mask)
		{
			cr1_value &= ~cr.cr1_mask;
			cr1_value |= cr.cr1;
			mcutl::memory::set_register_value<&ADC_TypeDef::CR1, Adc::base>(cr1_value);
		}
	}
	
	if constexpr (!!cr.cr2_mask)
	{
		auto cr2_value = mcutl::memory::get_register_bits<&ADC_TypeDef::CR2, Adc::base>();
		if ((cr2_value & cr.cr2_mask) != cr.cr2)
		{
			[[maybe_unused]] auto initial_cr2_value = cr2_value;
			cr2_value &= ~cr.cr2_mask;
			cr2_value |= cr.cr2;
			mcutl::memory::set_register_value<&ADC_TypeDef::CR2, Adc::base>(cr2_value);
			
			if constexpr (options.wait_time_set_count && options.enable)
			{
				if (!(initial_cr2_value & ADC_CR2_ADON))
					wait_initialization<Options...>();
			}
		}
	}
	
	if constexpr (options.enable_peripheral_set_count != 0)
	{
		if constexpr (!options.enable_peripheral)
		{
			static_assert(!options.enable,
				"Can not enable ADC while disabling its peripheral");
			mcutl::periph::configure_peripheral<
				mcutl::periph::disable<mcutl::adc::peripheral_type<Adc>>>();
		}
	}
	
	if constexpr (options.enable_controller_interrupts_set_count)
	{
		if (cr1_value & ADC_CR1_EOSIE)
		{
			mcutl::interrupt::enable<
				mcutl::interrupt::interrupt<
					mcutl::adc::interrupt_type<Adc, mcutl::adc::init::interrupt::conversion_complete>,
					options.conversion_complete.priority,
					options.conversion_complete.subpriority
				>,
				options.priority_count_set_count ? options.priority_count : mcutl::interrupt::maximum_priorities
			>();
		}
	}
	
	if constexpr (options.disable_controller_interrupts_set_count)
	{
		if (!(cr1_value & ADC_CR1_EOSIE))
		{
			mcutl::interrupt::disable<
				mcutl::adc::interrupt_type<Adc, mcutl::adc::init::interrupt::conversion_complete>
			>();
		}
	}
}

template<typename Adc, typename DmaChannelConfig, uint8_t ChannelCount>
struct dma_adc
{
	template<typename ValueType, size_t N>
	static void start(volatile ValueType (&result_pointer)[N]) MCUTL_NOEXCEPT
	{
		static_assert(N >= ChannelCount, "Too short array to store ADC DMA result");
		start(static_cast<volatile ValueType*>(result_pointer));
	}
	
	template<typename ValueType>
	static void start(volatile ValueType* result_pointer) MCUTL_NOEXCEPT
	{
		static_assert(sizeof(ValueType) == sizeof(uint16_t), "ADC data word must be 2 bytes wide");
		check_dma<Adc, typename DmaChannelConfig::dma_channel>();
		
		mcutl::dma::start_transfer<typename DmaChannelConfig::dma_channel>(
			&mcutl::memory::volatile_memory<ADC_TypeDef, Adc::base>()->DR, result_pointer,
			ChannelCount);
		
		mcutl::memory::set_register_bits<ADC_CR2_ADON_Msk, ADC_CR2_ADON,
			&ADC_TypeDef::CR2, Adc::base>();
	}
	
	[[nodiscard]] static bool is_finished() MCUTL_NOEXCEPT
	{
		return mcutl::memory::get_register_flag<ADC_SR_EOS_Msk,
			&ADC_TypeDef::SR, Adc::base>();
	}
};

template<typename Adc, typename... Options>
struct dma_adc_creater
{
	static constexpr void create() noexcept
	{
	}
};

template<typename Adc, typename Option, typename... Options>
struct dma_adc_creater<Adc, Option, Options...>
{
	static inline auto create() noexcept
	{
		return dma_adc_creater<Adc, Options...>::create();
	}
};

template<typename Adc, typename DmaChannelConfig, typename... Channels, typename... Options>
struct dma_adc_creater<Adc, mcutl::adc::conv::scan_channels<DmaChannelConfig, Channels...>, Options...>
{
	static inline auto create() noexcept
	{
		return dma_adc<Adc, DmaChannelConfig, sizeof...(Channels)>{};
	}
};

template<typename Adc, typename... Options>
inline auto create_dma_adc() noexcept
{
	return dma_adc_creater<Adc, Options...>::create();
}

template<typename Adc>
struct single_conversion_adc
{
	static void start() MCUTL_NOEXCEPT
	{
		mcutl::memory::set_register_bits<ADC_CR2_ADON_Msk,
			ADC_CR2_ADON, &ADC_TypeDef::CR2, Adc::base>();
	}
	
	[[nodiscard]] static mcutl::adc::conversion_result_type<Adc> get_conversion_result() MCUTL_NOEXCEPT
	{
		return (mcutl::memory::get_register_bits<&ADC_TypeDef::DR, Adc::base>()
			& ADC_DR_DATA_Msk) >> ADC_DR_DATA_Pos;
	}
	
	[[nodiscard]] static bool is_finished() MCUTL_NOEXCEPT
	{
		return mcutl::memory::get_register_flag<ADC_SR_EOS_Msk,
			&ADC_TypeDef::SR, Adc::base>();
	}
};

template<typename Adc, typename... Options>
inline auto create_single_conversion_adc() noexcept
{
	return single_conversion_adc<Adc>{};
}

template<typename Adc, typename... Options, typename OptionsLambda>
inline decltype(auto) prepare_conversion(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr auto options = options_lambda();
	
	static_assert(!(options.channel_index_set_count && options.scan_channels_set_count),
		"Single conversion and SCAN mode can not be selected simultaneously");
	static_assert(options.channel_index_set_count || options.scan_channels_set_count,
		"Either single conversion or SCAN mode must be selected");
	
	if constexpr (!!options.scan_channels_set_count)
		return create_dma_adc<Adc, Options...>();
	else
		return create_single_conversion_adc<Adc, Options...>();
}

} //namespace mcutl::device::adc
