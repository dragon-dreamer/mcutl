#pragma once

#include <stdint.h>
#include <utility>

#include "mcutl/utils/type_helpers.h"

namespace mcutl::clock
{

template<typename...>
struct config {};

namespace detail
{
template<uint64_t Frequency>
struct frequency_base
{
	static constexpr uint64_t frequency = Frequency;
	static_assert(Frequency, "Frequency can not be zero");
};
} //namespace detail

template<uint64_t Frequency>
struct required_frequency : detail::frequency_base<Frequency> {};
template<uint64_t Frequency>
struct min_frequency : detail::frequency_base<Frequency> {};
template<uint64_t Frequency>
struct max_frequency : detail::frequency_base<Frequency> {};

struct set_highest_possible_frequencies {};
struct set_lowest_possible_frequencies {};

struct base_configuration_is_currently_present {};

template<typename Requirement>
struct remove_requirement {};

namespace detail
{

enum class frequency_requirements
{
	highest,
	lowest
};

struct frequency_limits
{
	uint64_t exact_frequency = 0;
	uint64_t min_frequency = 0;
	uint64_t max_frequency = 0;
	
	constexpr bool has_limits() const noexcept
	{
		return exact_frequency || min_frequency || max_frequency;
	}
	
	constexpr void clear() noexcept
	{
		exact_frequency = 0;
		min_frequency = 0;
		max_frequency = 0;
	}
};

struct base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions) noexcept
	{
		static_assert(types::always_false<ClockOptions>::value, "Unknown clock configuration option");
		return false;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options(config<ResultOptions...> result, ClockOptions) noexcept
	{
		static_assert(types::always_false<ClockOptions>::value, "Unknown clock configuration option");
		return result;
	}
};

template<typename ClockOption, typename Limits>
struct clock_option_processor : base_clock_option_processor
{
};

template<typename FrequencyOption>
struct base_frequency_requirements_processor : base_clock_option_processor
{
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.frequency_reqs_explicitly_set)
			return result;
		else
			return config<ResultOptions..., FrequencyOption>{};
	}
};

template<auto FrequencyRequirements, typename ClockOptions>
constexpr auto process_frequency_requirements(ClockOptions clock_opts_lambda) noexcept
{
	constexpr auto clock_opts = clock_opts_lambda();
	static_assert(!clock_opts.frequency_reqs_explicitly_set,
		"Duplicate frequency requirements definition");
	auto clock_opts_modified = clock_opts;
	clock_opts_modified.frequency_reqs = FrequencyRequirements;
	clock_opts_modified.frequency_reqs_explicitly_set = true;
	return clock_opts_modified;
}

template<typename Limits>
struct clock_option_processor<set_highest_possible_frequencies, Limits>
	: base_frequency_requirements_processor<set_highest_possible_frequencies>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_frequency_requirements<frequency_requirements::highest>(clock_opts_lambda);
	}
};

template<typename Limits>
struct clock_option_processor<set_lowest_possible_frequencies, Limits>
	: base_frequency_requirements_processor<set_lowest_possible_frequencies>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_frequency_requirements<frequency_requirements::lowest>(clock_opts_lambda);
	}
};

template<typename Option>
struct frequency_option_processor
{
	constexpr void process(frequency_limits&, frequency_limits&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown frequency limit option");
	}
};

template<uint64_t Frequency>
struct frequency_option_processor<required_frequency<Frequency>>
{
	static constexpr void process(frequency_limits& limits, frequency_limits& previous) noexcept
	{
		static_assert(Frequency > 0, "Exact frequency can not be 0");
		previous.exact_frequency = limits.exact_frequency;
		limits.exact_frequency = Frequency;
	}
};

template<uint64_t Frequency>
struct frequency_option_processor<min_frequency<Frequency>>
{
	static constexpr void process(frequency_limits& limits, frequency_limits& previous) noexcept
	{
		static_assert(Frequency > 0, "Min frequency can not be 0");
		previous.min_frequency = limits.min_frequency;
		limits.min_frequency = Frequency;
	}
};

template<uint64_t Frequency>
struct frequency_option_processor<max_frequency<Frequency>>
{
	static constexpr void process(frequency_limits& limits, frequency_limits& previous) noexcept
	{
		static_assert(Frequency > 0, "Max frequency can not be 0");
		previous.max_frequency = limits.max_frequency;
		limits.max_frequency = Frequency;
	}
};

template<typename... Options>
constexpr auto calculate_frequency_options() noexcept
{
	frequency_limits result, previous;
	(frequency_option_processor<Options>::process(result, previous), ...);
	return std::pair(result, previous);
}

template<typename Limits, typename... Options>
constexpr auto check_frequency_options() noexcept
{
	constexpr auto frequency_limits_pair = calculate_frequency_options<Options...>();
	
	constexpr auto prev_limits = frequency_limits_pair.second;
	static_assert(!prev_limits.max_frequency, "Duplicate declaration for maximal frequency");
	static_assert(!prev_limits.min_frequency, "Duplicate declaration for minimal frequency");
	static_assert(!prev_limits.exact_frequency, "Duplicate declaration for required frequency");
	
	constexpr auto calculated = frequency_limits_pair.first;
	static_assert(!calculated.exact_frequency || !calculated.min_frequency,
		"Minimal frequency is set together with strictly required frequency");
	static_assert(!calculated.exact_frequency || !calculated.max_frequency,
		"Maximal frequency is set together with strictly required frequency");
	static_assert(!calculated.exact_frequency ||
		Limits::template is_valid<calculated.exact_frequency>(),
		"Required frequency is out of allowed range");
	static_assert((!calculated.min_frequency || !calculated.max_frequency)
		|| calculated.min_frequency <= calculated.max_frequency,
		"Minimal frequency is greater than maximal frequency");
	
	frequency_limits result = calculated;
	if (!calculated.exact_frequency)
	{
		if (!calculated.min_frequency)
			result.min_frequency = Limits::min_value;
		if (!calculated.max_frequency)
			result.max_frequency = Limits::max_value;
	}
	
	if (result.min_frequency < Limits::min_value)
		result.min_frequency = Limits::min_value;
	if (result.max_frequency > Limits::max_value)
		result.max_frequency = Limits::max_value;
	
	return result;
}

template<typename Limits>
struct clock_option_processor<base_configuration_is_currently_present, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.base_configuration_is_present,
			"Duplicate declarations for base_configuration_is_present option");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.base_configuration_is_present = true;
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.base_configuration_is_present)
			return result;
		else
			return config<ResultOptions..., base_configuration_is_currently_present>{};
	}
};

enum class oscillator_options_t
{
	unset            = 0,
	external_crystal = 1 << 0,
	external_bypass  = 1 << 1,
	internal         = 1 << 2
};

enum class pll_options_t
{
	force_on,
	force_off,
	automatic
};

struct base_clock_options
{
	oscillator_options_t oscillator_options = oscillator_options_t::unset;
	uint64_t external_high_frequency = 0;
	frequency_limits core_frequency {};
	bool usb_required = false;
	bool usb_required_explicitly_set = false;
	pll_options_t pll_options = pll_options_t::automatic;
	frequency_requirements frequency_reqs = frequency_requirements::highest;
	bool base_configuration_is_present = false;
	bool frequency_reqs_explicitly_set = false;
};

} //namespace detail

} //namespace mcutl::clock

namespace mcutl::clock::literals
{

[[nodiscard]] constexpr uint64_t operator ""_MHz(unsigned long long value) noexcept
{
	return value * 1'000'000;
}

[[nodiscard]] constexpr uint64_t operator ""_MHz(long double value) noexcept
{
	return static_cast<uint64_t>(value * 1'000'000);
}

[[nodiscard]] constexpr uint64_t operator ""_KHz(unsigned long long value) noexcept
{
	return value * 1'000;
}

} //namespace mcutl::clock::literals
