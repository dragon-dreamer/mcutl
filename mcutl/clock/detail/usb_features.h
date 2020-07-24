#pragma once

#include <type_traits>

#include "mcutl/clock/clock_defs.h"

namespace mcutl::clock
{

namespace detail
{
template<bool Provide>
struct provide_usb_frequency_type : std::bool_constant<Provide> {};
} //namespace detail

using provide_usb_frequency = detail::provide_usb_frequency_type<true>;
using disable_usb_frequency = detail::provide_usb_frequency_type<false>;

namespace detail
{

template<bool Provide, typename Limits>
struct clock_option_processor<provide_usb_frequency_type<Provide>, Limits> : base_clock_option_processor
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		constexpr auto clock_opts = clock_opts_lambda();
		static_assert(!clock_opts.usb_required_explicitly_set, "Duplicate declarations for USB frequency options");
		auto clock_opts_modified = clock_opts;
		clock_opts_modified.usb_required = Provide;
		clock_opts_modified.usb_required_explicitly_set = true;
		return clock_opts_modified;
	}
	
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.usb_required_explicitly_set)
			return result;
		else
			return config<ResultOptions..., provide_usb_frequency_type<Provide>>{};
	}
};

} //namespace detail

} //namespace mcutl::clock
