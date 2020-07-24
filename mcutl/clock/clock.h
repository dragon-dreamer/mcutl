#pragma once

#include <type_traits>
#include <utility>

#include "mcutl/clock/clock_defs.h"
#include "mcutl/device/clock/device_clock.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::clock
{

namespace detail
{

template<typename...>
struct clock_options_helper
{
	static constexpr auto process() noexcept
	{
		return device_specific_clock_options();
	}
	
	template<typename... ResultOptions, typename OverriddenOptions>
	static constexpr auto override_options(config<ResultOptions...> result,
		const OverriddenOptions&) noexcept
	{
		return result;
	}
};

template<typename ClockOption, typename... ClockOptions>
struct clock_options_helper<ClockOption, ClockOptions...>
{
	static constexpr auto process() noexcept
	{
		constexpr auto clock_opts = clock_options_helper<ClockOptions...>::process();
		if constexpr (!std::is_same_v<decltype(clock_opts), const bool>)
		{
			using option_processor = clock_option_processor<ClockOption, mcutl::device::clock::limits>;
			constexpr auto modified_clock_opts = option_processor::template process(
				[]() constexpr { return clock_options_helper<ClockOptions...>::process(); });
			return modified_clock_opts;
		}
		else
		{
			return clock_opts;
		}
	}
	
	template<typename... ResultOptions, typename OverriddenOptions>
	static constexpr auto override_options(config<ResultOptions...> result,
		OverriddenOptions overridden_options) noexcept
	{
		using option_processor = clock_option_processor<ClockOption, mcutl::device::clock::limits>;
		constexpr auto new_result = option_processor::template override_options(result, overridden_options);
		return clock_options_helper<ClockOptions...>::template override_options(new_result, overridden_options);
	}
};

template<typename... ClockOptions>
constexpr auto configure_clocks_impl() noexcept
{
	constexpr auto clock_config = detail::clock_options_helper<ClockOptions...>::process();
	static_assert(clock_config.oscillator_options != oscillator_options_t::unset,
		"Oscillator type is not set");
	return clock_config;
}

template<typename... ClockOptions, typename... OverriddenOptions>
constexpr auto merge_original_with_overridden(config<ClockOptions...>, config<OverriddenOptions...>) noexcept
{
	return config<ClockOptions..., OverriddenOptions...>{};
}
	
template<typename... ClockOptions, typename... OverriddenOptions>
constexpr auto override_options(config<ClockOptions...>, config<OverriddenOptions...> overridden_options) noexcept
{
	constexpr auto overridden_options_lambda = []() constexpr {
		return detail::clock_options_helper<OverriddenOptions...>::process();
	};
	
	return merge_original_with_overridden(
		detail::clock_options_helper<ClockOptions...>::template override_options(config<>{}, overridden_options_lambda),
		overridden_options
	);
}

template<typename ClockOptionsLambda>
constexpr auto calculate_clock_tree(ClockOptionsLambda options_lambda) noexcept
{
	using namespace mcutl::device::clock;
	constexpr auto clock_options = options_lambda();
	device_clock_tree clock_tree(clock_options);
	return clock_tree.choose_best<device_source_id>(clock_options.frequency_reqs);
}

template<typename... ClockOptions>
struct unpack_options
{
	static constexpr void get_option_lambdas() noexcept
	{
		static_assert(types::always_false<ClockOptions...>::value,
			"Clock options must be wrapped in 'config' structure");
	}
};

template<typename... ClockOptions>
struct unpack_options<config<ClockOptions...>>
{
	static constexpr auto get_option_lambdas() noexcept
	{
		constexpr auto best_tree_config_lambda = []() constexpr {
			constexpr auto options_lambda = []() constexpr {
				return detail::configure_clocks_impl<ClockOptions...>(); };
			constexpr auto best_tree_config = detail::calculate_clock_tree(options_lambda);
			static_assert(best_tree_config.is_valid(),
				"Unable to configure clock tree using provided options");
			return best_tree_config;
		};
		constexpr auto options_lambda = []() constexpr {
			return detail::configure_clocks_impl<ClockOptions...>(); };
		return std::pair(options_lambda, best_tree_config_lambda);
	}
};

} //namespace detail

template<typename ClockOptions>
void configure_clocks() MCUTL_NOEXCEPT
{
	constexpr auto lambdas = detail::unpack_options<ClockOptions>::get_option_lambdas();
	mcutl::device::clock::configure_clocks(lambdas.first, lambdas.second);
}

template<typename OldClockOptions, typename NewClockOptions>
void reconfigure_clocks() MCUTL_NOEXCEPT
{
	constexpr auto old_lambdas = detail::unpack_options<OldClockOptions>::get_option_lambdas();
	constexpr auto new_lambdas = detail::unpack_options<NewClockOptions>::get_option_lambdas();
	mcutl::device::clock::reconfigure_clocks(old_lambdas.first, old_lambdas.second,
		new_lambdas.first, new_lambdas.second);
}

template<typename ClockOptions>
[[nodiscard]] constexpr auto get_best_clock_tree() noexcept
{
	return detail::unpack_options<ClockOptions>::get_option_lambdas().second();
}

template<typename OldClockOptions, typename OverriddenOptions>
[[nodiscard]] constexpr auto get_best_reconfigured_clock_tree() noexcept
{
	return detail::unpack_options<OldClockOptions, OverriddenOptions>::get_option_lambdas().second();
}

struct clock_info
{
	uint64_t exact_frequency;
	uint64_t prescaler;
	uint64_t reverse_prescaler;
	uint64_t unscaled_frequency;
};

using clock_id = device::clock::device_source_id;

template<typename ClockOptions, auto ClockId>
[[nodiscard]] constexpr clock_info get_clock_info() noexcept
{
	constexpr auto best_tree = get_best_clock_tree<ClockOptions>();
	constexpr auto config = best_tree.template get_config_by_id(ClockId);
	static_assert(config.is_used(), "Selected clock ID is not used by provided configuration");
	
	constexpr auto parent_source_id = best_tree.template get_source_parent(ClockId, ClockId);
	constexpr auto unscaled_frequency = best_tree.template get_config_by_id(parent_source_id)
		.get_exact_frequency();
	
	return { config.get_exact_frequency(), config.get_prescaler_value(), config.get_reverse_prescaler(),
		unscaled_frequency };
}

template<typename OldClockOptions, typename NewClockOptions, auto ClockId>
[[nodiscard]] constexpr clock_info get_reconfigured_clock_info() noexcept
{
	constexpr auto config = get_best_reconfigured_clock_tree<
		OldClockOptions, NewClockOptions>().get_config_by_id(ClockId);
	static_assert(config.is_used(), "Selected clock ID is not used by provided configuration");
	return { config.get_exact_frequency(), config.get_prescaler_value(), config.get_reverse_prescaler() };
}

template<typename OldClockOptions, typename NewClockOptions>
using overridden_options_t = decltype(detail::override_options(std::declval<OldClockOptions>(),
	std::declval<NewClockOptions>()));

} //namespace mcutl::clock
