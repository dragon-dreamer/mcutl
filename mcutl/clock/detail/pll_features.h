#pragma once

#include "mcutl/clock/clock_defs.h"

namespace mcutl::clock
{

struct force_use_pll {};
struct force_skip_pll {};

namespace detail
{

template<auto PllOptions, typename ClockOptions>
constexpr auto process_pll_options(ClockOptions clock_opts_lambda) noexcept
{
	constexpr auto clock_opts = clock_opts_lambda();
	static_assert(clock_opts.pll_options == pll_options_t::automatic,
		"Duplicate PLL options definition");
	auto clock_opts_modified = clock_opts;
	clock_opts_modified.pll_options = PllOptions;
	return clock_opts_modified;
}

template<typename PllOption>
struct base_pll_options_processor : base_clock_option_processor
{
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.pll_options != pll_options_t::automatic)
			return result;
		else
			return config<ResultOptions..., PllOption>{};
	}
};

template<typename Limits>
struct clock_option_processor<force_use_pll, Limits> : base_pll_options_processor<force_use_pll>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_pll_options<pll_options_t::force_on>(clock_opts_lambda);
	}
};

template<typename Limits>
struct clock_option_processor<force_skip_pll, Limits> : base_pll_options_processor<force_skip_pll>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_pll_options<pll_options_t::force_off>(clock_opts_lambda);
	}
};

} //namespace detail

} //namespace mcutl::clock
