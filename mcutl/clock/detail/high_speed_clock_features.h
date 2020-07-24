#pragma once

#include <type_traits>

#include "mcutl/clock/clock_defs.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::clock
{

template<uint64_t Frequency>
struct external_high_speed_crystal : detail::frequency_base<Frequency> {};

template<uint64_t Frequency>
struct external_high_speed_bypass : detail::frequency_base<Frequency> {};

struct internal_high_speed_crystal {};

namespace detail
{

constexpr oscillator_options_t operator |(oscillator_options_t lhs, oscillator_options_t rhs) noexcept
{
	using T = std::underlying_type_t<oscillator_options_t>;
	return static_cast<oscillator_options_t>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

constexpr oscillator_options_t& operator |=(oscillator_options_t& lhs, oscillator_options_t rhs) noexcept
{
	lhs = lhs | rhs;
	return lhs;
}

constexpr oscillator_options_t operator &(oscillator_options_t lhs, oscillator_options_t rhs) noexcept
{
	using T = std::underlying_type_t<oscillator_options_t>;
	return static_cast<oscillator_options_t>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

constexpr oscillator_options_t& operator &=(oscillator_options_t& lhs, oscillator_options_t rhs) noexcept
{
	lhs = lhs & rhs;
	return lhs;
}

template<typename OscillatorOption>
struct base_high_frequency_clock_processor : base_clock_option_processor
{
	template<typename... ResultOptions, typename ClockOptions>
	static constexpr auto override_options([[maybe_unused]] config<ResultOptions...> result,
		ClockOptions overridden_options) noexcept
	{
		constexpr auto clock_opts = overridden_options();
		if constexpr (clock_opts.oscillator_options != oscillator_options_t::unset)
			return result;
		else
			return config<ResultOptions..., OscillatorOption>{};
	}
};

template<auto OscillatorType, auto CurrentOptions>
constexpr bool is_high_frequency_option_compatible() noexcept
{
	if (OscillatorType == oscillator_options_t::external_bypass || OscillatorType == oscillator_options_t::external_crystal)
		return CurrentOptions == oscillator_options_t::unset || CurrentOptions == oscillator_options_t::internal;
	
	if (OscillatorType == oscillator_options_t::internal)
		return (CurrentOptions & oscillator_options_t::internal) != oscillator_options_t::internal;
	
	return OscillatorType == oscillator_options_t::unset;
}
	
template<uint64_t Frequency, auto OscillatorType, typename Limit, typename ClockOptions>
constexpr auto process_high_speed_frequency(ClockOptions clock_opts_lambda) noexcept
{
	constexpr auto clock_opts = clock_opts_lambda();
	static_assert(is_high_frequency_option_compatible<OscillatorType, clock_opts.oscillator_options>(),
		"Incompatible high frequency source definitions");
	static_assert(Limit::template is_valid<Frequency>(), "High frequency is out of allowed range");
	auto clock_opts_modified = clock_opts;
	clock_opts_modified.external_high_frequency = Frequency;
	clock_opts_modified.oscillator_options |= OscillatorType;
	return clock_opts_modified;
}

template<uint64_t Frequency, typename Limits>
struct clock_option_processor<external_high_speed_crystal<Frequency>, Limits>
	: base_high_frequency_clock_processor<external_high_speed_crystal<Frequency>>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_high_speed_frequency<Frequency, oscillator_options_t::external_crystal,
			typename Limits::external_crystal_frequency>(clock_opts_lambda);
	}
};

template<uint64_t Frequency, typename Limits>
struct clock_option_processor<external_high_speed_bypass<Frequency>, Limits>
	: base_high_frequency_clock_processor<external_high_speed_bypass<Frequency>>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_high_speed_frequency<Frequency, oscillator_options_t::external_bypass,
			typename Limits::external_bypass_frequency>(clock_opts_lambda);
	}
};

template<typename Limits>
struct clock_option_processor<internal_high_speed_crystal, Limits>
	: base_high_frequency_clock_processor<internal_high_speed_crystal>
{
	template<typename ClockOptions>
	static constexpr auto process(ClockOptions clock_opts_lambda) noexcept
	{
		return process_high_speed_frequency<0u, oscillator_options_t::internal,
			types::limits<0u, 0u>>(clock_opts_lambda);
	}
};

} //namespace detail

} //namespace mcutl::clock
