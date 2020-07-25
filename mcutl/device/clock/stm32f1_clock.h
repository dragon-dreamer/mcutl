#pragma once

#include <stdint.h>
#include <type_traits>

#include "mcutl/device/device.h"
#include "mcutl/clock/clock_defs.h"
#include "mcutl/clock/detail/clock_tree_processor.h"
#include "mcutl/clock/detail/core_features.h"
#include "mcutl/clock/detail/high_speed_clock_features.h"
#include "mcutl/clock/detail/pll_features.h"
#include "mcutl/clock/detail/usb_features.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::clock
{

template<typename... FrequencyOptions>
struct adc {};
template<typename... FrequencyOptions>
struct apb1 {};
template<typename... FrequencyOptions>
struct apb2 {};
template<typename... FrequencyOptions>
struct ahb {};

#ifdef RCC_APB2ENR_SPI1EN
template<typename... FrequencyOptions>
struct spi1 {};
#endif //RCC_APB2ENR_SPI1EN

#ifdef RCC_APB1ENR_SPI2EN
template<typename... FrequencyOptions>
struct spi2 {};
#endif // RCC_APB1ENR_SPI2EN

#ifdef RCC_APB1ENR_SPI3EN
template<typename... FrequencyOptions>
struct spi3 {};
#endif //RCC_APB1ENR_SPI3EN

#if defined (RCC_APB1ENR_TIM2EN) || defined (RCC_APB1ENR_TIM3EN) || defined (RCC_APB1ENR_TIM4EN) \
	|| defined (RCC_APB1ENR_TIM5EN) || defined(RCC_APB1ENR_TIM6EN) || defined(RCC_APB1ENR_TIM7EN) \
	|| defined(RCC_APB1ENR_TIM12EN) || defined(RCC_APB1ENR_TIM13EN) || defined(RCC_APB1ENR_TIM14EN)
template<typename... FrequencyOptions>
struct timer2_3_4_5_6_7_12_13_14 {};
#endif //RCC_APB1ENR_TIM2EN - RCC_APB1ENR_TIM7EN, RCC_APB1ENR_TIM12EN - RCC_APB1ENR_TIM14EN

#if defined (RCC_APB2ENR_TIM1EN) || defined (RCC_APB2ENR_TIM8EN) || defined (RCC_APB2ENR_TIM9EN) \
	|| defined (RCC_APB2ENR_TIM10EN) || defined(RCC_APB2ENR_TIM11EN)
template<typename... FrequencyOptions>
struct timer1_8_9_10_11 {};
#endif //RCC_APB2ENR_TIM1EN, RCC_APB2ENR_TIM8EN - RCC_APB2ENR_TIM11EN

template<bool Disable>
struct disable_flash_programming_interface_type : std::bool_constant<Disable> {};

using disable_flash_programming_interface = disable_flash_programming_interface_type<true>;
using enable_flash_programming_interface = disable_flash_programming_interface_type<false>;

namespace detail
{

struct device_specific_clock_options : base_clock_options
{
	frequency_limits apb1_frequency;
	frequency_limits apb2_frequency;
	frequency_limits adc_frequency;
	frequency_limits ahb_frequency;
	frequency_limits spi1_frequency;
	frequency_limits spi2_frequency;
	frequency_limits spi3_frequency;
	frequency_limits timer2_3_4_5_6_7_12_13_14_frequency;
	frequency_limits timer1_8_9_10_11_frequency;
	bool use_flitf = true;
	bool flitf_option_changed = false;
};

template<bool Disable, typename Limits>
struct clock_option_processor<disable_flash_programming_interface_type<Disable>, Limits>
	: base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(clock_opts.flitf_option_changed, "Flash programming interface is already disabled");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.use_flitf = !Disable;
		clock_opts_modified.flitf_option_changed = true;
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.flitf_option_changed)
			return result;
		else
			return config<ResultOptions..., disable_flash_programming_interface_type<Disable>>{};
	}
};

template<typename... Options, typename Limits>
struct clock_option_processor<adc<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.adc_frequency.has_limits(),
			"Duplicate declarations for ADC frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.adc_frequency = check_frequency_options<
			typename Limits::adc_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.adc_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., adc<Options...>>{};
	}
};

template<typename... Options, typename Limits>
struct clock_option_processor<apb1<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.apb1_frequency.has_limits(),
			"Duplicate declarations for APB1 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.apb1_frequency = check_frequency_options<
			typename Limits::apb1_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.apb1_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., apb1<Options...>>{};
	}
};

template<typename... Options, typename Limits>
struct clock_option_processor<apb2<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.apb2_frequency.has_limits(),
			"Duplicate declarations for APB2 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.apb2_frequency = check_frequency_options<
			typename Limits::apb2_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.apb2_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., apb2<Options...>>{};
	}
};

template<typename... Options, typename Limits>
struct clock_option_processor<ahb<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.ahb_frequency.has_limits(),
			"Duplicate declarations for AHB frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.ahb_frequency = check_frequency_options<
			typename Limits::ahb_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.ahb_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., ahb<Options...>>{};
	}
};

#ifdef RCC_APB2ENR_SPI1EN
template<typename... Options, typename Limits>
struct clock_option_processor<spi1<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.spi1_frequency.has_limits(),
			"Duplicate declarations for SPI1 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.spi1_frequency = check_frequency_options<
			typename Limits::spi1_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.spi1_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., spi1<Options...>>{};
	}
};
#endif //RCC_APB2ENR_SPI1EN

#ifdef RCC_APB1ENR_SPI2EN
template<typename... Options, typename Limits>
struct clock_option_processor<spi2<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.spi2_frequency.has_limits(),
			"Duplicate declarations for SPI2 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.spi2_frequency = check_frequency_options<
			typename Limits::spi2_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.spi2_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., spi2<Options...>>{};
	}
};
#endif //RCC_APB1ENR_SPI2EN

#if defined (RCC_APB1ENR_TIM2EN) || defined (RCC_APB1ENR_TIM3EN) || defined (RCC_APB1ENR_TIM4EN) \
	|| defined (RCC_APB1ENR_TIM5EN) || defined(RCC_APB1ENR_TIM6EN) || defined(RCC_APB1ENR_TIM7EN) \
	|| defined(RCC_APB1ENR_TIM12EN) || defined(RCC_APB1ENR_TIM13EN) || defined(RCC_APB1ENR_TIM14EN)
template<typename... Options, typename Limits>
struct clock_option_processor<timer2_3_4_5_6_7_12_13_14<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.timer2_3_4_5_6_7_12_13_14_frequency.has_limits(),
			"Duplicate declarations for TIM2-TIM7, TIM12-TIM14 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.timer2_3_4_5_6_7_12_13_14_frequency = check_frequency_options<
			typename Limits::timer2_3_4_5_6_7_12_13_14_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.timer2_3_4_5_6_7_12_13_14_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., timer2_3_4_5_6_7_12_13_14<Options...>>{};
	}
};
#endif //RCC_APB1ENR_TIM2EN - RCC_APB1ENR_TIM7EN, RCC_APB1ENR_TIM12EN - RCC_APB1ENR_TIM14EN

#if defined (RCC_APB2ENR_TIM1EN) || defined (RCC_APB2ENR_TIM8EN) || defined (RCC_APB2ENR_TIM9EN) \
	|| defined (RCC_APB2ENR_TIM10EN) || defined(RCC_APB2ENR_TIM11EN)
template<typename... Options, typename Limits>
struct clock_option_processor<timer1_8_9_10_11<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.timer1_8_9_10_11_frequency.has_limits(),
			"Duplicate declarations for TIM1, TIM8-TIM11 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.timer1_8_9_10_11_frequency = check_frequency_options<
			typename Limits::timer1_8_9_10_11_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.timer1_8_9_10_11_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., timer1_8_9_10_11<Options...>>{};
	}
};
#endif //RCC_APB2ENR_TIM1EN, RCC_APB2ENR_TIM8EN - RCC_APB2ENR_TIM11EN

#ifdef RCC_APB1ENR_SPI3EN
template<typename... Options, typename Limits>
struct clock_option_processor<spi3<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.spi3_frequency.has_limits(),
			"Duplicate declarations for SPI3 frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.spi3_frequency = check_frequency_options<
			typename Limits::spi3_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.spi3_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., spi3<Options...>>{};
	}
};
#endif //RCC_APB1ENR_SPI3EN

} //namespace detail

} //namespace mcutl::clock

namespace mcutl::device::clock
{

using namespace mcutl::clock::literals;

#if defined(STM32F105xC) || defined(STM32F107xC) //Connectivity line
struct limits
{
	using external_crystal_frequency = utils::limits<3_MHz, 25_MHz>;
	using external_bypass_frequency = utils::limits<1_MHz, 50_MHz>;
	using core_frequency = utils::limits<0_MHz, 72_MHz>;
	using apb1_frequency = utils::limits<0_MHz, 36_MHz>;
	using apb2_frequency = utils::limits<0_MHz, 72_MHz>;
	using adc_frequency = utils::limits<600'000, 14_MHz>;
	using ahb_frequency = utils::limits<0_MHz, 72_MHz>;
	using spi1_frequency = utils::limits<0_MHz, 18_MHz>;
	using spi2_frequency = utils::limits<0_MHz, 18_MHz>;
	using spi3_frequency = utils::limits<0_MHz, 18_MHz>;
	using timer2_3_4_5_6_7_12_13_14_frequency = utils::limits<0_MHz, 72_MHz>;
	using timer1_8_9_10_11_frequency = utils::limits<0_MHz, 72_MHz>;
};
#else //Other
struct limits
{
	using external_crystal_frequency = types::limits<4_MHz, 16_MHz>;
	using external_bypass_frequency = types::limits<1_MHz, 25_MHz>;
	using core_frequency = types::limits<0_MHz, 72_MHz>;
	using apb1_frequency = types::limits<0_MHz, 36_MHz>;
	using apb2_frequency = types::limits<0_MHz, 72_MHz>;
	using adc_frequency = types::limits<600'000, 14_MHz>;
	using ahb_frequency = types::limits<0_MHz, 72_MHz>;
	using spi1_frequency = types::limits<0_MHz, 18_MHz>;
	using spi2_frequency = types::limits<0_MHz, 18_MHz>;
	using spi3_frequency = types::limits<0_MHz, 18_MHz>;
	using timer2_3_4_5_6_7_12_13_14_frequency = types::limits<0_MHz, 72_MHz>;
	using timer1_8_9_10_11_frequency = types::limits<0_MHz, 72_MHz>;
};
#endif //Connectivity line

constexpr uint64_t internal_oscillator_frequency = 8_MHz;

enum class device_source_id : uint8_t
{
	empty,
	hse,
	hsi,
	hse_prediv,
	hsi_prediv,
	pll,
	usb,
	sys,
	ahb,
	apb1,
	apb2,
	adc,
	spi1,
	spi2,
	spi3,
	timer2_3_4_5_6_7_12_13_14,
	timer1_8_9_10_11,
	timer2_3_4_5_6_7_12_13_14_multiplier,
	timer1_8_9_10_11_multiplier
};

class device_clock_tree : public mcutl::clock::detail::clock_tree_base
{
public:
	constexpr explicit device_clock_tree(
		const mcutl::clock::detail::device_specific_clock_options& clock_options) noexcept
		: clock_tree_base()
		, hse{ device_source_id::hse }
		, hsi{ device_source_id::hsi }
		, hse_prediv{ device_source_id::hse_prediv }
		, hsi_prediv{ device_source_id::hsi_prediv }
		, pll{ device_source_id::pll }
		, usb{ device_source_id::usb }
		, sys{ device_source_id::sys }
		, ahb{ device_source_id::ahb }
		, apb1{ device_source_id::apb1 }
		, apb2{ device_source_id::apb2 }
		, adc{ device_source_id::adc }
		, spi1{ device_source_id::spi1 }
		, spi2{ device_source_id::spi2 }
		, spi3{ device_source_id::spi3 }
		, timer2_3_4_5_6_7_12_13_14{ device_source_id::timer2_3_4_5_6_7_12_13_14 }
		, timer1_8_9_10_11{ device_source_id::timer1_8_9_10_11 }
		, timer2_3_4_5_6_7_12_13_14_multiplier{ device_source_id::timer2_3_4_5_6_7_12_13_14_multiplier }
		, timer1_8_9_10_11_multiplier{ device_source_id::timer1_8_9_10_11_multiplier }
	{
		using namespace mcutl::clock::detail;
		
		if ((clock_options.oscillator_options & oscillator_options_t::internal)
			== oscillator_options_t::internal)
		{
			add_root(hsi << exact_frequency(internal_oscillator_frequency));
			add_source(hsi_prediv << divider(2))
				<< tree_parent(hsi);
		}
		
		if ((clock_options.oscillator_options & oscillator_options_t::external_bypass)
				== oscillator_options_t::external_bypass
			|| (clock_options.oscillator_options & oscillator_options_t::external_crystal)
				== oscillator_options_t::external_crystal)
		{
			add_root(hse << exact_frequency(clock_options.external_high_frequency));
			add_source(hse_prediv << divider({ {1,2} }))
				<< tree_parent(hse);
		}
		
		if (clock_options.pll_options != pll_options_t::force_off)
		{
			add_source(pll << max_frequency(72'000'000)
				<< min_frequency(1'000'000)
				<< multiplier({ { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } }))
				<< tree_parent(hsi_prediv) << tree_parent(hse_prediv);
		}

		if (clock_options.usb_required)
		{
			add_leaf(usb << exact_frequency(48'000'000)
				<< divider({ { 2, 3 } }, 2))
				<< tree_parent(pll);
		}

		auto sys_source = add_source(sys << max_frequency(72'000'000)
			<< clock_options.core_frequency)
			<< tree_parent(pll);
		if (clock_options.pll_options != pll_options_t::force_on)
			sys_source << tree_parent(hsi) << tree_parent(hse);

		add_source(ahb << limits::ahb_frequency{}
			<< divider({ {1, 2, 4, 8, 16, 64, 128, 256, 512} })
			<< clock_options.ahb_frequency)
			<< tree_parent(sys);

		add_leaf(apb1 << limits::apb1_frequency{}
			<< (clock_options.usb_required ? min_frequency(8'000'000) : min_frequency(0))
			<< divider({ {1, 2, 4, 8, 16} })
			<< clock_options.apb1_frequency)
			<< tree_parent(ahb);

		add_source(apb2 << limits::apb2_frequency{}
			<< divider({ {1, 2, 4, 8, 16} })
			<< clock_options.apb2_frequency)
			<< tree_parent(ahb);

		add_leaf(adc << limits::adc_frequency{}
			<< divider({ {2, 4, 6, 8} })
			<< clock_options.adc_frequency)
			<< tree_parent(apb2);
		
		if (clock_options.spi1_frequency.has_limits())
		{
			add_leaf(spi1
				<< limits::spi1_frequency{}
				<< divider({ { 2, 4, 8, 16, 32, 64, 128, 256 } })
				<< clock_options.spi1_frequency)
				<< tree_parent(apb2);
		}
		if (clock_options.spi2_frequency.has_limits())
		{
			add_leaf(spi2
				<< limits::spi2_frequency{}
				<< divider({ { 2, 4, 8, 16, 32, 64, 128, 256 } })
				<< clock_options.spi2_frequency)
				<< tree_parent(apb1);
		}
		if (clock_options.spi3_frequency.has_limits())
		{
			add_leaf(spi3
				<< limits::spi3_frequency{}
				<< divider({ { 2, 4, 8, 16, 32, 64, 128, 256 } })
				<< clock_options.spi3_frequency)
				<< tree_parent(apb1);
		}
		
		if (clock_options.timer2_3_4_5_6_7_12_13_14_frequency.has_limits())
		{
			add_source(timer2_3_4_5_6_7_12_13_14_multiplier
				<< multiplier({ { 1, 2 } })
				<< timer_multiplier_validator)
				<< tree_parent(apb1);
			
			add_leaf(timer2_3_4_5_6_7_12_13_14
				<< limits::timer2_3_4_5_6_7_12_13_14_frequency{}
				<< clock_options.timer2_3_4_5_6_7_12_13_14_frequency)
				<< tree_parent(timer2_3_4_5_6_7_12_13_14_multiplier);
		}
		
		if (clock_options.timer1_8_9_10_11_frequency.has_limits())
		{
			add_source(timer1_8_9_10_11_multiplier
				<< multiplier({ { 1, 2 } })
				<< timer_multiplier_validator)
				<< tree_parent(apb2);
			
			add_leaf(timer1_8_9_10_11
				<< limits::timer1_8_9_10_11_frequency{}
				<< clock_options.timer1_8_9_10_11_frequency)
				<< tree_parent(timer1_8_9_10_11_multiplier);
		}

		set_frequency_priority(sys, pll, ahb, apb1, apb2, adc, spi1, spi2, spi3,
			timer2_3_4_5_6_7_12_13_14, timer1_8_9_10_11, hse_prediv, hsi_prediv,
			timer2_3_4_5_6_7_12_13_14_multiplier, timer1_8_9_10_11_multiplier);
	}
	
private:
	static constexpr bool timer_multiplier_validator(
		uint64_t parent_prescaler_value, uint64_t child_prescaler_value) noexcept
	{
		if (parent_prescaler_value == 1)
			return child_prescaler_value == 1;
		
		return child_prescaler_value == 2;
	}

private:
	using clock_source = mcutl::clock::detail::clock_source;
	clock_source hse, hsi;
	clock_source hse_prediv, hsi_prediv;
	clock_source pll;
	clock_source usb;
	clock_source sys;
	clock_source ahb, apb1, apb2;
	clock_source adc;
	clock_source spi1, spi2, spi3;
	clock_source timer2_3_4_5_6_7_12_13_14, timer1_8_9_10_11;
	clock_source timer2_3_4_5_6_7_12_13_14_multiplier, timer1_8_9_10_11_multiplier;
};

template<uint32_t AhbPrescaler>
constexpr uint32_t get_hpre_bits() noexcept
{
	switch (AhbPrescaler)
	{
	case 2:
		return RCC_CFGR_HPRE_DIV2;
	case 4:
		return RCC_CFGR_HPRE_DIV4;
	case 8:
		return RCC_CFGR_HPRE_DIV8;
	case 16:
		return RCC_CFGR_HPRE_DIV16;
	case 64:
		return RCC_CFGR_HPRE_DIV64;
	case 128:
		return RCC_CFGR_HPRE_DIV128;
	case 256:
		return RCC_CFGR_HPRE_DIV256;
	case 512:
		return RCC_CFGR_HPRE_DIV512;
	default:
		break;
	}
	
	return RCC_CFGR_HPRE_DIV1;
}

template<uint32_t Apb1Prescaler>
constexpr uint32_t get_ppre1_bits() noexcept
{
	switch (Apb1Prescaler)
	{
	case 2:
		return RCC_CFGR_PPRE1_DIV2;
	case 4:
		return RCC_CFGR_PPRE1_DIV4;
	case 8:
		return RCC_CFGR_PPRE1_DIV8;
	case 16:
		return RCC_CFGR_PPRE1_DIV16;
	default:
		break;
	}
	
	return RCC_CFGR_PPRE1_DIV1;
}

template<uint32_t Apb2Prescaler>
constexpr uint32_t get_ppre2_bits() noexcept
{
	switch (Apb2Prescaler)
	{
	case 2:
		return RCC_CFGR_PPRE2_DIV2;
	case 4:
		return RCC_CFGR_PPRE2_DIV4;
	case 8:
		return RCC_CFGR_PPRE2_DIV8;
	case 16:
		return RCC_CFGR_PPRE2_DIV16;
	default:
		break;
	}
	
	return RCC_CFGR_PPRE2_DIV1;
}

template<uint32_t AdcPrescaler>
constexpr uint32_t get_adcpre_bits() noexcept
{
	switch (AdcPrescaler)
	{
	case 4:
		return RCC_CFGR_ADCPRE_DIV4;
	case 6:
		return RCC_CFGR_ADCPRE_DIV6;
	case 8:
		return RCC_CFGR_ADCPRE_DIV8;
	default:
		break;
	}
	
	return RCC_CFGR_ADCPRE_DIV2;
}

template<uint32_t PllPrescaler>
constexpr uint32_t get_pllmul_bits() noexcept
{
	switch (PllPrescaler)
	{
	case 3: return RCC_CFGR_PLLMULL3;
	case 4: return RCC_CFGR_PLLMULL4;
	case 5: return RCC_CFGR_PLLMULL5;
	case 6: return RCC_CFGR_PLLMULL6;
	case 7: return RCC_CFGR_PLLMULL7;
	case 8: return RCC_CFGR_PLLMULL8;
	case 9: return RCC_CFGR_PLLMULL9;
	case 10: return RCC_CFGR_PLLMULL10;
	case 11: return RCC_CFGR_PLLMULL11;
	case 12: return RCC_CFGR_PLLMULL12;
	case 13: return RCC_CFGR_PLLMULL13;
	case 14: return RCC_CFGR_PLLMULL14;
	case 15: return RCC_CFGR_PLLMULL15;
	case 16: return RCC_CFGR_PLLMULL16;
	default: break;
	}
	
	return RCC_CFGR_PLLMULL2;
}

struct spi_options
{
	uint32_t prescaler_bits;
	bool used;
};

template<uint32_t SpiPrescaler>
constexpr spi_options get_spi_prescaler_bits() noexcept
{
	spi_options result {};
	result.used = true;
	switch (SpiPrescaler)
	{
	case 4: result.prescaler_bits = SPI_CR1_BR_0; break;
	case 8: result.prescaler_bits = SPI_CR1_BR_1; break;
	case 16: result.prescaler_bits = SPI_CR1_BR_0 | SPI_CR1_BR_1; break;
	case 32: result.prescaler_bits = SPI_CR1_BR_2; break;
	case 64: result.prescaler_bits = SPI_CR1_BR_2 | SPI_CR1_BR_0; break;
	case 128: result.prescaler_bits = SPI_CR1_BR_2 | SPI_CR1_BR_1; break;
	case 256: result.prescaler_bits = SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0; break;
	default: break;
	}
	
	return result;
}

struct device_clock_options
{
	uint32_t flitf_sys_frequency = 0u;
	uint32_t cfgr_bits = 0u;
	uint32_t cfgr_bits_mask = 0u;
	device_source_id sys_source = device_source_id::hsi;
	bool usb_used = false;
	bool pll_used = false;
	bool hsi_used = false;
	bool hse_used = false;
	bool base_configuration_is_present = false;
	bool use_external_bypass = false;
	spi_options spi1_opts {};
	spi_options spi2_opts {};
	spi_options spi3_opts {};
};

template<typename ClockOptionsLambda, typename BestTreeLambda>
constexpr device_clock_options get_clock_options(
	ClockOptionsLambda options_lambda, BestTreeLambda best_clock_tree_lambda) noexcept
{
	using namespace mcutl::clock::detail;
	
	device_clock_options result;
	constexpr auto best_clock_tree = best_clock_tree_lambda();
	constexpr auto clock_options = options_lambda();
	
	result.base_configuration_is_present = clock_options.base_configuration_is_present;
	result.use_external_bypass = (clock_options.oscillator_options
		& oscillator_options_t::external_bypass) == oscillator_options_t::external_bypass;
	
	if constexpr (clock_options.use_flitf)
	{
		result.flitf_sys_frequency = best_clock_tree.get_config_by_id(
			device_source_id::sys).get_exact_frequency();
	}
	
	result.hse_used = best_clock_tree.get_config_by_id(device_source_id::hse).is_used()
		|| (clock_options.oscillator_options & oscillator_options_t::external_bypass)
			== oscillator_options_t::external_bypass
		|| (clock_options.oscillator_options & oscillator_options_t::external_crystal)
			== oscillator_options_t::external_crystal;
	
	constexpr uint32_t hpre_bits = get_hpre_bits<best_clock_tree.get_config_by_id(
		device_source_id::ahb).get_prescaler_value()>();
	constexpr uint32_t ppre1_bits = get_ppre1_bits<best_clock_tree.get_config_by_id(
		device_source_id::apb1).get_prescaler_value()>();
	constexpr uint32_t ppre2_bits = get_ppre2_bits<best_clock_tree.get_config_by_id(
		device_source_id::apb2).get_prescaler_value()>();
	constexpr uint32_t adcpre_bits = get_adcpre_bits<best_clock_tree.get_config_by_id(
		device_source_id::adc).get_prescaler_value()>();
	result.cfgr_bits_mask |= RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2
		| RCC_CFGR_ADCPRE | RCC_CFGR_HPRE | RCC_CFGR_SWS;
	result.cfgr_bits |= hpre_bits | ppre1_bits | ppre2_bits | adcpre_bits;
	
	constexpr auto pll_config = best_clock_tree.get_config_by_id(device_source_id::pll);
	constexpr auto usb_config = best_clock_tree.get_config_by_id(device_source_id::usb);
	if constexpr (pll_config.is_used())
	{
		result.pll_used = true;
		constexpr auto pll_parent = best_clock_tree.get_node_parent(device_source_id::pll);
		constexpr uint32_t pllmul_bits = get_pllmul_bits<pll_config.get_prescaler_value()>();
		constexpr uint32_t pllxtpre_bits = pll_parent == device_source_id::hse_prediv
				&& best_clock_tree.get_config_by_id(device_source_id::hse_prediv).get_prescaler_value() == 2u
			? RCC_CFGR_PLLXTPRE_HSE_DIV2 : RCC_CFGR_PLLXTPRE_HSE;
		constexpr uint32_t pllsrc_bits = pll_parent == device_source_id::hse_prediv ? RCC_CFGR_PLLSRC : 0;
		
		result.cfgr_bits_mask |= RCC_CFGR_PLLMULL | RCC_CFGR_PLLXTPRE_HSE_DIV2 | RCC_CFGR_PLLSRC | RCC_CFGR_USBPRE;
		result.cfgr_bits |= pllmul_bits | pllxtpre_bits | pllsrc_bits;
		if constexpr (usb_config.is_used() && usb_config.get_prescaler_value() == 2u)
			result.cfgr_bits |= RCC_CFGR_USBPRE; // USB clock is not divided
	}
	
	result.usb_used = usb_config.is_used();
	result.hsi_used = best_clock_tree.get_config_by_id(device_source_id::hsi).is_used();
	
	if constexpr (usb_config.is_used())
	{
		constexpr auto apb1_config = best_clock_tree.get_config_by_id(device_source_id::pll);
		static_assert(apb1_config.is_used() && apb1_config.get_exact_frequency() > 13_MHz,
			"When using USB, APB1 frequency must be at least 13 MHz");
	}
	
	if constexpr (best_clock_tree.get_node_parent(device_source_id::sys) == device_source_id::hsi)
		result.sys_source = device_source_id::hsi;
	else if constexpr (best_clock_tree.get_node_parent(device_source_id::sys) == device_source_id::hse)
		result.sys_source = device_source_id::hse;
	else if constexpr (best_clock_tree.get_node_parent(device_source_id::sys) == device_source_id::pll)
		result.sys_source = device_source_id::pll;
	
	constexpr auto spi1_config = best_clock_tree.get_config_by_id(device_source_id::spi1);
	if constexpr (spi1_config.is_used())
		result.spi1_opts = get_spi_prescaler_bits<spi1_config.get_prescaler_value()>();
	
	constexpr auto spi2_config = best_clock_tree.get_config_by_id(device_source_id::spi2);
	if constexpr (spi2_config.is_used())
		result.spi2_opts = get_spi_prescaler_bits<spi2_config.get_prescaler_value()>();
	
	constexpr auto spi3_config = best_clock_tree.get_config_by_id(device_source_id::spi3);
	if constexpr (spi3_config.is_used())
		result.spi3_opts = get_spi_prescaler_bits<spi3_config.get_prescaler_value()>();
	
	return result;
}

template<uint32_t PrescalerBits, uint32_t SpiBase>
void set_spi_prescaler() noexcept
{
	memory::set_register_bits<SPI_CR1_BR_Msk, PrescalerBits, &SPI_TypeDef::CR1, SpiBase>();
}

template<uint64_t FlitfSysFrequency>
constexpr uint32_t get_flash_acr() noexcept
{
	uint32_t flash_acr = 0;
	if constexpr (FlitfSysFrequency > 24_MHz && FlitfSysFrequency <= 48_MHz)
		flash_acr |= FLASH_ACR_LATENCY_0;
	else if constexpr (FlitfSysFrequency > 48_MHz)
		flash_acr |= FLASH_ACR_LATENCY_1;
	return flash_acr;
}

template<typename ClockOptionsLambda, typename BestTreeLambda>
void configure_clocks(ClockOptionsLambda options_lambda,
	BestTreeLambda best_clock_tree_lambda) MCUTL_NOEXCEPT
{
	using namespace mcutl::clock::detail;
	
#if defined(STM32F105xC) || defined(STM32F107xC) //Connectivity line
	static_assert(false, "Connectivity line is not supported yet");
#endif //Connectivity line
	
	constexpr auto clock_opts = get_clock_options(options_lambda, best_clock_tree_lambda);
	
	bool need_to_disable_usb = false;
	uint32_t current_cfg = memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE>();
	if constexpr (!clock_opts.base_configuration_is_present)
	{
		uint32_t current_control = memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE>();
		
		need_to_disable_usb = memory::get_register_flag<&RCC_TypeDef::APB1ENR,
			RCC_BASE, RCC_APB1ENR_USBEN>();
		bool need_to_switch_sys_to_hsi = (current_cfg & (RCC_CFGR_SW | RCC_CFGR_SWS))
			!= (RCC_CFGR_SW_HSI | RCC_CFGR_SWS_HSI);
		bool need_to_disable_pll = static_cast<bool>(
			current_control & (RCC_CR_PLLON | RCC_CR_PLLRDY));
		bool need_to_disable_hse = static_cast<bool>(
			current_control & (RCC_CR_HSEON | RCC_CR_HSERDY));
		bool need_to_enable_hsi = (current_control & (RCC_CR_HSION | RCC_CR_HSIRDY))
			!= (RCC_CR_HSION | RCC_CR_HSIRDY);
		
		if (need_to_disable_usb)
		{
			memory::set_register_bits<RCC_APB1ENR_USBEN_Msk, ~RCC_APB1ENR_USBEN,
				&RCC_TypeDef::APB1ENR, RCC_BASE>();
			[[maybe_unused]] auto temp = memory::get_register_bits<&RCC_TypeDef::APB1ENR, RCC_BASE>();
		}
	
		if (need_to_enable_hsi)
		{
			memory::set_register_bits<RCC_CR_HSION_Msk, RCC_CR_HSION,
				&RCC_TypeDef::CR, RCC_BASE>();
			while (!memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_HSIRDY>())
			{
			}
		}
	
		if (need_to_switch_sys_to_hsi)
		{
			current_cfg &= ~RCC_CFGR_SW;
			current_cfg |= RCC_CFGR_SW_HSI;
			memory::set_register_value<&RCC_TypeDef::CFGR, RCC_BASE>(current_cfg);
			while (memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE, RCC_CFGR_SWS>()
				!= RCC_CFGR_SWS_HSI)
			{
			}
		}
	
		if (need_to_disable_pll)
		{
			memory::set_register_bits<RCC_CR_PLLON_Msk, ~RCC_CR_PLLON,
				&RCC_TypeDef::CR, RCC_BASE>();
			while (memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_PLLRDY>())
			{
			}
		}
	
		if (need_to_disable_hse)
		{
			memory::set_register_bits<RCC_CR_HSEON_Msk, ~RCC_CR_HSEON,
				&RCC_TypeDef::CR, RCC_BASE>();
			while (memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_HSERDY>())
			{
			}
			memory::set_register_bits<RCC_CR_HSEBYP_Msk, ~RCC_CR_HSEBYP,
				&RCC_TypeDef::CR, RCC_BASE>();
		}
	}
	
	if constexpr (clock_opts.hse_used)
	{
		constexpr uint32_t cr_bits = RCC_CR_HSEON
			| (clock_opts.use_external_bypass ? RCC_CR_HSEBYP : 0);
		memory::set_register_bits<cr_bits, cr_bits, &RCC_TypeDef::CR, RCC_BASE>();
		while (!memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_HSERDY>())
		{
		}
	}
	
	current_cfg &= ~clock_opts.cfgr_bits_mask;
	current_cfg |= clock_opts.cfgr_bits;
	memory::set_register_value<&RCC_TypeDef::CFGR, RCC_BASE>(current_cfg);
	if constexpr (clock_opts.pll_used)
	{
		memory::set_register_bits<RCC_CR_PLLON_Msk, RCC_CR_PLLON,
			&RCC_TypeDef::CR, RCC_BASE>();
		while (!memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_PLLRDY>())
		{
		}
	}
	
	if constexpr (!!clock_opts.flitf_sys_frequency)
	{
		memory::set_register_bits<FLASH_ACR_LATENCY_Msk,
			get_flash_acr<clock_opts.flitf_sys_frequency>(), &FLASH_TypeDef::ACR, FLASH_R_BASE>();
	}
	
	if constexpr (clock_opts.sys_source != device_source_id::hsi)
	{
		current_cfg = memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE>();
		current_cfg &= ~RCC_CFGR_SW;
		if constexpr (clock_opts.sys_source == device_source_id::hse)
			current_cfg |= RCC_CFGR_SW_HSE;
		else if constexpr (clock_opts.sys_source == device_source_id::pll)
			current_cfg |= RCC_CFGR_SW_PLL;
		
		memory::set_register_value<&RCC_TypeDef::CFGR, RCC_BASE>(current_cfg);
		while (memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE, RCC_CFGR_SWS>()
			== RCC_CFGR_SWS_HSI)
		{
		}
	}
	
	if constexpr (clock_opts.usb_used)
	{
		if (need_to_disable_usb) //re-enable USB
		{
			memory::set_register_bits<RCC_APB1ENR_USBEN_Msk, RCC_APB1ENR_USBEN,
				&RCC_TypeDef::APB1ENR, RCC_BASE>();
			[[maybe_unused]] auto temp = memory::get_register_bits<&RCC_TypeDef::APB1ENR, RCC_BASE>();
		}
	}
	
	if constexpr (!clock_opts.hsi_used)
	{
		memory::set_register_bits<RCC_CR_HSION_Msk, ~RCC_CR_HSION,
			&RCC_TypeDef::CR, RCC_BASE>();
	}
	
#ifdef RCC_APB2ENR_SPI1EN
	if constexpr (clock_opts.spi1_opts.used)
		set_spi_prescaler<clock_opts.spi1_opts.prescaler_bits, SPI1_BASE>();
#endif //RCC_APB2ENR_SPI1EN
#ifdef RCC_APB1ENR_SPI2EN
	if constexpr (clock_opts.spi2_opts.used)
		set_spi_prescaler<clock_opts.spi2_opts.prescaler_bits, SPI2_BASE>();
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	if constexpr (clock_opts.spi3_opts.used)
		set_spi_prescaler<clock_opts.spi3_opts.prescaler_bits, SPI3_BASE>();
#endif //RCC_APB1ENR_SPI3EN
}

template<typename Options>
constexpr bool usb_must_be_reenabled(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	return (usb_must_be_disabled(old_clock_opts, new_clock_opts) || !old_clock_opts.usb_used)
		&& new_clock_opts.usb_used;
}

template<typename Options>
constexpr bool pll_must_be_disabled(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	if (!old_clock_opts.pll_used)
		return false;
	
	if (!new_clock_opts.pll_used)
		return true;
	
	constexpr uint32_t mask = RCC_CFGR_PLLMULL | RCC_CFGR_PLLXTPRE_HSE_DIV2
		| RCC_CFGR_PLLSRC | RCC_CFGR_USBPRE;
	return (old_clock_opts.cfgr_bits & mask) != (new_clock_opts.cfgr_bits & mask);
}

template<typename Options>
constexpr bool usb_must_be_disabled(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	if (!old_clock_opts.usb_used)
		return false;

	if (!new_clock_opts.usb_used)
		return true;

	return pll_must_be_disabled(old_clock_opts, new_clock_opts);
}

template<typename Options>
constexpr bool switch_sys_to_hsi(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	return (old_clock_opts.sys_source != new_clock_opts.sys_source
			|| pll_must_be_disabled(old_clock_opts, new_clock_opts))
		&& old_clock_opts.sys_source != device_source_id::hsi;
}

template<typename Options>
constexpr bool pll_must_be_reenabled(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	return (pll_must_be_disabled(old_clock_opts, new_clock_opts) || !old_clock_opts.pll_used)
		&& new_clock_opts.pll_used;
}

template<typename Options>
constexpr bool hse_must_be_disabled(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	return (old_clock_opts.hse_used && !new_clock_opts.hse_used)
		|| (old_clock_opts.use_external_bypass != new_clock_opts.use_external_bypass);
}

template<typename Options>
constexpr bool hse_must_be_reenabled(const Options& old_clock_opts, const Options& new_clock_opts) noexcept
{
	return (hse_must_be_disabled(old_clock_opts, new_clock_opts) || !old_clock_opts.hse_used)
		&& new_clock_opts.hse_used;
}

template<typename Options>
constexpr uint32_t get_sys_source_bits(const Options& new_clock_opts) noexcept
{
	if (new_clock_opts.sys_source == device_source_id::hse)
		return RCC_CFGR_SW_HSE;
	else if (new_clock_opts.sys_source == device_source_id::pll)
		return RCC_CFGR_SW_PLL;
	
	return RCC_CFGR_SW_HSI;
}

template<typename OldClockOptionsLambda, typename OldBestTreeLambda,
	typename NewClockOptionsLambda, typename NewBestTreeLambda>
void reconfigure_clocks(OldClockOptionsLambda old_options_lambda,
	OldBestTreeLambda old_best_clock_tree_lambda,
	NewClockOptionsLambda new_options_lambda,
	NewBestTreeLambda new_best_clock_tree_lambda) MCUTL_NOEXCEPT
{
	using namespace mcutl::clock::detail;
	
#if defined(STM32F105xC) || defined(STM32F107xC) //Connectivity line
	static_assert(false, "Connectivity line is not supported yet");
#endif //Connectivity line
	
	constexpr auto new_clock_opts = get_clock_options(new_options_lambda, new_best_clock_tree_lambda);
	constexpr auto old_clock_opts = get_clock_options(old_options_lambda, old_best_clock_tree_lambda);
	
	constexpr bool need_to_switch_sys_to_hsi = switch_sys_to_hsi(old_clock_opts, new_clock_opts);
	
	if constexpr (usb_must_be_disabled(old_clock_opts, new_clock_opts))
	{
		memory::set_register_bits<RCC_APB1ENR_USBEN_Msk, ~RCC_APB1ENR_USBEN,
			&RCC_TypeDef::APB1ENR, RCC_BASE>();
		[[maybe_unused]] auto temp = memory::get_register_bits<&RCC_TypeDef::APB1ENR, RCC_BASE>();
	}
	
	if constexpr (need_to_switch_sys_to_hsi && !old_clock_opts.hsi_used)
	{
		memory::set_register_bits<RCC_CR_HSION_Msk, RCC_CR_HSION,
			&RCC_TypeDef::CR, RCC_BASE>();
		while (!memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_HSIRDY>())
		{
		}
	}
	
	//It's safe to switch SYSCLK to HSI without changing the prescalers of AHB, APB1, APB2,
	//as HSI works at 8 MHz, and all these prescalers can only divide the frequency, so
	//no frequency falls out of maximum allowed range.
	if constexpr (need_to_switch_sys_to_hsi)
	{
		memory::set_register_bits<old_clock_opts.cfgr_bits_mask | RCC_CFGR_SW_Msk,
			old_clock_opts.cfgr_bits & ~RCC_CFGR_SW_Msk,
			&RCC_TypeDef::CFGR, RCC_BASE>();
		while (memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE, RCC_CFGR_SWS>()
			!= RCC_CFGR_SWS_HSI)
		{
		}
	}
	
	if constexpr (pll_must_be_disabled(old_clock_opts, new_clock_opts))
	{
		memory::set_register_bits<RCC_CR_PLLON_Msk, ~RCC_CR_PLLON,
			&RCC_TypeDef::CR, RCC_BASE>();
		while (memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_PLLRDY>())
		{
		}
	}
	
	if constexpr (hse_must_be_disabled(old_clock_opts, new_clock_opts))
	{
		memory::set_register_bits<RCC_CR_HSEON_Msk, ~RCC_CR_HSEON,
			&RCC_TypeDef::CR, RCC_BASE>();
		while (memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_HSERDY>())
		{
		}
		if constexpr (old_clock_opts.use_external_bypass)
		{
			memory::set_register_bits<RCC_CR_HSEBYP_Msk, ~RCC_CR_HSEBYP,
				&RCC_TypeDef::CR, RCC_BASE>();
		}
	}
	
	if constexpr (hse_must_be_reenabled(old_clock_opts, new_clock_opts))
	{
		constexpr uint32_t cr_bits = RCC_CR_HSEON | (new_clock_opts.use_external_bypass ? RCC_CR_HSEBYP : 0);
		memory::set_register_bits<cr_bits, cr_bits, &RCC_TypeDef::CR, RCC_BASE>();
		while (!memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_HSERDY>())
		{
		}
	}
	
	if constexpr (old_clock_opts.cfgr_bits != new_clock_opts.cfgr_bits)
	{
		//This can change the prescalers and change the PLL source only
		memory::set_register_bits<new_clock_opts.cfgr_bits_mask | old_clock_opts.cfgr_bits_mask,
			new_clock_opts.cfgr_bits, &RCC_TypeDef::CFGR, RCC_BASE>();
	}
	
	if constexpr (pll_must_be_reenabled(old_clock_opts, new_clock_opts))
	{
		memory::set_register_bits<RCC_CR_PLLON_Msk, RCC_CR_PLLON,
			&RCC_TypeDef::CR, RCC_BASE>();
		while (!memory::get_register_bits<&RCC_TypeDef::CR, RCC_BASE, RCC_CR_PLLRDY>())
		{
		}
	}
	
	if constexpr (get_flash_acr<new_clock_opts.flitf_sys_frequency>()
		!= get_flash_acr<old_clock_opts.flitf_sys_frequency>())
	{
		memory::set_register_bits<FLASH_ACR_LATENCY_Msk,
			get_flash_acr<new_clock_opts.flitf_sys_frequency>(), &FLASH_TypeDef::ACR, FLASH_R_BASE>();
	}
	
	if constexpr (need_to_switch_sys_to_hsi && new_clock_opts.sys_source != device_source_id::hsi)
	{
		constexpr auto new_sys_source_bits = get_sys_source_bits(new_clock_opts);
		memory::set_register_bits<RCC_CFGR_SW_Msk, new_sys_source_bits,
			&RCC_TypeDef::CFGR, RCC_BASE>();
		while (memory::get_register_bits<&RCC_TypeDef::CFGR, RCC_BASE, RCC_CFGR_SWS>()
			== RCC_CFGR_SWS_HSI)
		{
		}
	}
	
	if constexpr (usb_must_be_reenabled(old_clock_opts, new_clock_opts))
	{
		memory::set_register_bits<RCC_APB1ENR_USBEN_Msk, RCC_APB1ENR_USBEN,
			&RCC_TypeDef::APB1ENR, RCC_BASE>();
		[[maybe_unused]] auto temp = memory::get_register_bits<&RCC_TypeDef::APB1ENR, RCC_BASE>();
	}
	
	if constexpr ((need_to_switch_sys_to_hsi || old_clock_opts.hsi_used) && !new_clock_opts.hsi_used)
	{
		memory::set_register_bits<RCC_CR_HSION_Msk, ~RCC_CR_HSION,
			&RCC_TypeDef::CR, RCC_BASE>();
	}
	
#ifdef RCC_APB2ENR_SPI1EN
	if constexpr (new_clock_opts.spi1_opts.used
		&& (new_clock_opts.spi1_opts.prescaler_bits != old_clock_opts.spi1_opts.prescaler_bits
			|| !old_clock_opts.spi1_opts.used))
	{
		set_spi_prescaler<new_clock_opts.spi1_opts.prescaler_bits, SPI1_BASE>();
	}
#endif //RCC_APB2ENR_SPI1EN
#ifdef RCC_APB1ENR_SPI2EN
	if constexpr (new_clock_opts.spi2_opts.used
		&& (new_clock_opts.spi2_opts.prescaler_bits != old_clock_opts.spi2_opts.prescaler_bits
			|| !old_clock_opts.spi2_opts.used))
	{
		set_spi_prescaler<new_clock_opts.spi2_opts.prescaler_bits, SPI2_BASE>();
	}
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_SPI3EN
	if constexpr (new_clock_opts.spi3_opts.used
		&& (new_clock_opts.spi3_opts.prescaler_bits != old_clock_opts.spi3_opts.prescaler_bits
			|| !old_clock_opts.spi3_opts.used))
	{
		set_spi_prescaler<new_clock_opts.spi3_opts.prescaler_bits, SPI3_BASE>();
	}
#endif //RCC_APB1ENR_SPI3EN
}
	
} //namespace mcutl::device::clock
