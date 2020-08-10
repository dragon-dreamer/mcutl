#pragma once

#include <stdint.h>

#include "mcutl/utils/class_limitations.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::low_power
{

namespace wakeup_mode
{

struct wait_for_event : types::static_class {};
struct wait_for_interrupt : types::static_class {};

} //namespace wait_mode

namespace sleep_mode
{

struct core_stop : types::static_class {};
struct adc_noise_reduction : types::static_class {};
struct stop_all_clocks : types::static_class {};
struct power_off : types::static_class {};

} //namespace mode

namespace detail
{

template<typename Option>
struct options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown low power option");
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda) noexcept
	{
	}
};

struct low_power_options_base
{
};

} //namespace detail

} //namespace mcutl::low_power
