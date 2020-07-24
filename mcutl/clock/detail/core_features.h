#pragma once

#include "mcutl/clock/clock_defs.h"

namespace mcutl::clock
{

template<typename... FrequencyOptions>
struct core {};

namespace detail
{

template<typename... Options, typename Limits>
struct clock_option_processor<core<Options...>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.core_frequency.has_limits(), "Duplicate declarations for core frequency limits");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.core_frequency = check_frequency_options<typename Limits::core_frequency, Options...>();
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.core_frequency.has_limits())
			return result;
		else
			return config<ResultOptions..., core<Options...>>{};
	}
};

} //namespace detail

} //namespace mcutl::clock
