#pragma once

#include <limits>
#include <numeric>
#include <ratio>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/utils/math.h"
#include "mcutl/utils/options_parser.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::timer
{

namespace direction
{

struct up {};
struct down {};

} //namespace direction

template<bool Stop>
struct stop_on_overflow {};

template<uint64_t Value>
struct prescaler {};

template<uint64_t Value>
struct reload_value {};

template<bool Enable>
struct enable {};

template<bool Enable>
struct enable_peripheral {};

template<bool Buffer>
struct buffer_reload_value {};

struct base_configuration_is_currently_present {};

template<typename ClockConfig, typename FrequencyHz, typename MaxErrorHz>
struct timer_frequency
{
	using clock_config_type = ClockConfig;
	using frequency = FrequencyHz;
	using max_error = MaxErrorHz;
};

template<typename ClockConfig, typename FrequencyHz>
using exact_timer_frequency = timer_frequency<ClockConfig, FrequencyHz, void>;

template<typename ClockConfig, typename FrequencyHz, typename MaxErrorHz, bool ExtraPrecision = false>
struct overflow_frequency : timer_frequency<ClockConfig, FrequencyHz, MaxErrorHz> {};

template<typename ClockConfig, typename FrequencyHz, bool ExtraPrecision = false>
using exact_overflow_frequency = overflow_frequency<ClockConfig, FrequencyHz, std::ratio<0>, ExtraPrecision>;

namespace interrupt
{

struct overflow {};

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};

} //namespace interrupt

namespace detail
{

template<uint64_t... SortedPrescalers>
using prescaler_sequence = std::integer_sequence<uint64_t, SortedPrescalers...>;

template<typename Limits, uint64_t... SortedPrescalers>
struct prescaler_traits_base
{
	using limits = Limits;
	using prescalers = prescaler_sequence<SortedPrescalers...>;
	static constexpr bool discrete = sizeof...(SortedPrescalers) != 0;
	
	static constexpr auto get_discrete_prescaler_array() noexcept
	{
		std::array<uint64_t, sizeof...(SortedPrescalers)> arr { SortedPrescalers... };
		return arr;
	}
};

struct timer_frequency_traits_base
{
	template<typename ClockConfig>
	static constexpr auto get_timer_frequency() noexcept
	{
		static_assert(types::always_false<ClockConfig>::value,
			"Unable to determine timer frequency");
		return std::ratio<1> {};
	}
};

template<uint8_t TimerIndex,
	typename CounterType,
	uint16_t MaxCounterValue,
	typename Peripheral,
	typename FrequencyTraits = timer_frequency_traits_base,
	typename PrescalerTraits = void,
	typename ReloadValueLimits = void>
struct timer_base
{
	static constexpr uint8_t index = TimerIndex;
	using counter_type = CounterType;
	static constexpr CounterType max_value = MaxCounterValue;
	using peripheral = Peripheral;
	using prescaler_traits = PrescalerTraits;
	using reload_value_limits = ReloadValueLimits;
	
	template<typename ClockConfig>
	static constexpr auto get_timer_frequency() noexcept
	{
		return FrequencyTraits::template get_timer_frequency<ClockConfig>();
	}
	
	template<uint64_t Prescaler>
	static constexpr uint64_t select_next_prescaler() noexcept
	{
		static_assert(!std::is_same_v<prescaler_traits, void>,
			"Timer prescaler is not supported");
		
		if constexpr (!std::is_same_v<prescaler_traits, void>)
		{
			if constexpr (prescaler_traits::discrete)
			{
				constexpr auto prescalers = prescaler_traits::get_discrete_prescaler_array();
				for (auto prescaler : prescalers)
				{
					if (Prescaler < prescaler)
						return prescaler;
				}
				
				return 0;
			}
			else
			{
				constexpr uint64_t next = Prescaler + 1;
				if constexpr (prescaler_traits::limits::template is_valid<next>())
					return next;
				else
					return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	
	template<uint64_t Prescaler>
	static constexpr uint64_t select_prev_prescaler() noexcept
	{
		static_assert(!std::is_same_v<prescaler_traits, void>,
			"Timer prescaler is not supported");
		
		if constexpr (!std::is_same_v<prescaler_traits, void>)
		{
			if constexpr (prescaler_traits::discrete)
			{
				constexpr auto prescalers = prescaler_traits::get_discrete_prescaler_array();
				for (size_t i = prescalers.size(); i; --i)
				{
					if (Prescaler > prescalers[i - 1])
						return prescalers[i - 1];
				}
				
				return 0;
			}
			else
			{
				constexpr uint64_t prev = Prescaler - 1;
				if constexpr (prescaler_traits::limits::template is_valid<prev>())
					return prev;
				else
					return 0;
			}
		}
		else
		{
			return 0u;
		}
	}
	
	template<uint64_t Prescaler>
	static constexpr bool has_prescaler() noexcept
	{
		static_assert(!std::is_same_v<prescaler_traits, void>,
			"Timer prescaler is not supported");
		
		if constexpr (!std::is_same_v<prescaler_traits, void>)
		{
			if constexpr (prescaler_traits::discrete)
			{
				constexpr auto prescalers = prescaler_traits::get_discrete_prescaler_array();
				for (auto prescaler : prescalers)
				{
					if (prescaler == Prescaler)
						return true;
				}
				return false;
			}
			else
			{
				return prescaler_traits::limits::template is_valid<Prescaler>();
			}
		}
		else
		{
			return false;
		}
	}
	
	template<typename Prescaler>
	static constexpr uint64_t select_nearest_prescaler() noexcept
	{
		static_assert(!std::is_same_v<prescaler_traits, void>,
			"Timer prescaler is not supported");
		
		if constexpr (Prescaler::den == 1 && has_prescaler<Prescaler::num>())
		{
			return Prescaler::num;
		}
		else
		{
			constexpr auto prev = has_prescaler<Prescaler::num / Prescaler::den>()
				? Prescaler::num / Prescaler::den
				: select_prev_prescaler<Prescaler::num / Prescaler::den>();
			constexpr auto next = select_next_prescaler<prev>();
			static_assert(prev || next, "Unable to find suitable prescaler");
			if (!next)
				return prev;
			if (!prev)
				return next;
			using next_diff = std::ratio_subtract<std::ratio<next>, Prescaler>;
			using prev_diff = std::ratio_subtract<Prescaler, std::ratio<prev>>;
			if constexpr (std::ratio_less_v<next_diff, prev_diff>)
				return next;
			else
				return prev;
		}
	}
	
	static constexpr bool is_discrete_prescaler() noexcept
	{
		if constexpr (!std::is_same_v<prescaler_traits, void>)
			return prescaler_traits::discrete;
		else
			return false;
	}
};

template<typename Timer>
struct timer_traits
{
	static_assert(types::always_false<Timer>::value, "Unknown timer type");
};

template<uint8_t TimerIndex,
	typename CounterType, uint16_t MaxCounterValue,
	typename Peripheral, typename FrequencyTraits,
	typename PrescalerTraits, typename ReloadValueLimits>
struct timer_traits<timer_base<TimerIndex, CounterType, MaxCounterValue, Peripheral,
	FrequencyTraits, PrescalerTraits, ReloadValueLimits>>
	: timer_base<TimerIndex, CounterType, MaxCounterValue,
		Peripheral, FrequencyTraits, PrescalerTraits, ReloadValueLimits>
{
};

template<typename Timer, typename Interrupt>
struct interrupt_type_helper
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown timer or interrupt type");
};

template<typename Timer, typename Option>
struct options_parser
{
	template<typename Options>
	static constexpr void parse(Options&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unknown timer option");
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda) noexcept
	{
	}
};

struct count_mode
{
	enum value : uint8_t
	{
		default_mode,
		count_up,
		count_down,
		end
	};
};

constexpr uint64_t unset_value = (std::numeric_limits<uint64_t>::max)();
	
struct options
{
	mcutl::interrupt::detail::interrupt_info overflow {};
	count_mode::value mode = count_mode::default_mode;
	bool stop_on_overflow = false;
	uint64_t prescaler = unset_value;
	uint64_t reload_value = unset_value;
	bool buffer_reload_value = false;
	bool enable = false;
	bool enable_peripheral = false;
	uint64_t priority_count = 0;
	
	uint32_t overflow_set_count = 0;
	uint32_t count_mode_set_count = 0;
	uint32_t stop_on_overflow_set_count = 0;
	uint32_t prescaler_set_count = 0;
	uint32_t reload_value_set_count = 0;
	uint32_t buffer_reload_value_set_count = 0;
	uint32_t enable_set_count = 0;
	uint32_t enable_peripheral_set_count = 0;
	uint32_t base_configuration_set_count = 0;
	uint32_t enable_controller_interrupts_set_count = 0;
	uint32_t disable_controller_interrupts_set_count = 0;
	uint32_t priority_count_set_count = 0;
};

template<typename Timer>
struct options_parser<Timer, direction::up>
	: opts::base_option_parser<count_mode::count_up,
		&options::mode, &options::count_mode_set_count> {};
template<typename Timer>
struct options_parser<Timer, direction::down>
	: opts::base_option_parser<count_mode::count_down,
		&options::mode, &options::count_mode_set_count> {};
template<typename Timer, bool Stop>
struct options_parser<Timer, stop_on_overflow<Stop>>
	: opts::base_option_parser<Stop,
		&options::stop_on_overflow, &options::stop_on_overflow_set_count> {};
template<typename Timer, bool BufferReloadValue>
struct options_parser<Timer, buffer_reload_value<BufferReloadValue>>
	: opts::base_option_parser<BufferReloadValue,
		&options::buffer_reload_value, &options::buffer_reload_value_set_count> {};
template<typename Timer, bool Enable>
struct options_parser<Timer, enable<Enable>>
	: opts::base_option_parser<Enable, &options::enable, &options::enable_set_count> {};
template<typename Timer, bool Enable>
struct options_parser<Timer, enable_peripheral<Enable>>
	: opts::base_option_parser<Enable, &options::enable_peripheral, &options::enable_peripheral_set_count> {};
template<typename Timer>
struct options_parser<Timer, base_configuration_is_currently_present>
	: opts::base_option_parser<0, nullptr, &options::base_configuration_set_count> {};

template<typename Interrupt>
struct interrupt_map : mcutl::interrupt::detail::map<Interrupt> {};

template<> struct interrupt_map<interrupt::overflow>
	: mcutl::interrupt::detail::map_base<&options::overflow_set_count, &options::overflow> {};

template<typename Timer>
struct options_parser<Timer, interrupt::overflow>
	: opts::base_option_parser<0, nullptr, &options::overflow_set_count> {};

template<typename Timer, typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct options_parser<Timer, mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: mcutl::interrupt::detail::interrupt_parser<Interrupt, interrupt_map, Priority, SubPriority> {};

template<typename Timer, typename Interrupt>
struct options_parser<Timer, mcutl::interrupt::disabled<Interrupt>>
	: mcutl::interrupt::detail::interrupt_parser<mcutl::interrupt::disabled<Interrupt>, interrupt_map> {};

template<typename Timer>
struct options_parser<Timer, interrupt::enable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &options::enable_controller_interrupts_set_count> {};

template<typename Timer>
struct options_parser<Timer, interrupt::disable_controller_interrupts>
	: opts::base_option_parser<0, nullptr, &options::disable_controller_interrupts_set_count> {};

template<typename Timer, auto PriorityCount>
struct options_parser<Timer, mcutl::interrupt::priority_count<PriorityCount>>
	: opts::base_option_parser<PriorityCount, &options::priority_count,
		&options::priority_count_set_count> {};

template<typename Timer, uint64_t Prescaler>
struct options_parser<Timer, prescaler<Prescaler>>
	: opts::base_option_parser<Prescaler, &options::prescaler, &options::prescaler_set_count>
{
	using base_type = opts::base_option_parser<Prescaler, &options::prescaler,
		&options::prescaler_set_count>;
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda options_lambda) noexcept
	{
		base_type::validate(options_lambda);
		static_assert(timer_traits<Timer>::template has_prescaler<options_lambda().prescaler>(),
			"Invalid prescaler value for the timer");
	}
};

template<typename Timer, uint64_t ReloadValue>
struct options_parser<Timer, reload_value<ReloadValue>>
	: opts::base_option_parser<ReloadValue, &options::reload_value, &options::reload_value_set_count>
{
	using base_type = opts::base_option_parser<ReloadValue, &options::reload_value,
		&options::reload_value_set_count>;
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda options_lambda) noexcept
	{
		base_type::validate(options_lambda);
		using limits = typename timer_traits<Timer>::reload_value_limits;
		static_assert(!std::is_same_v<limits, void>, "Reload value for the timer is not supported");
		if constexpr (!std::is_same_v<limits, void>)
		{
			static_assert(limits::template is_valid<options_lambda().reload_value>(),
				"Invalid reload value value for the timer");
		}
	}
};

template<typename Timer, typename ClockConfig, typename FrequencyHz, typename MaxErrorHz>
struct options_parser<Timer, timer_frequency<ClockConfig, FrequencyHz, MaxErrorHz>>
	: opts::base_option_parser<0, nullptr, &options::prescaler_set_count>
{
	struct prescaler_result
	{
		uint64_t prescaler;
		bool precise;
		bool valid;
	};
	
	static constexpr prescaler_result get_prescaler_value() noexcept
	{
		using timer_frequency = decltype(timer_traits<Timer>
			::template get_timer_frequency<ClockConfig>());
		
		using prescaler = std::ratio_divide<timer_frequency, FrequencyHz>;
		using integral_prescaler = std::ratio<timer_traits<Timer>
			::template select_nearest_prescaler<prescaler>()>;
		
		using real_frequency = std::ratio_divide<timer_frequency, integral_prescaler>;
		using frequency_diff = std::conditional_t<
			std::ratio_greater_equal_v<real_frequency, FrequencyHz>,
			std::ratio_subtract<real_frequency, FrequencyHz>,
			std::ratio_subtract<FrequencyHz, real_frequency>
		>;
		
		prescaler_result result {};
		result.precise = std::ratio_less_equal_v<frequency_diff, MaxErrorHz>;
		result.valid = timer_traits<Timer>::template has_prescaler<integral_prescaler::num>();
		result.prescaler = integral_prescaler::num;
		return result;
	}
	
	template<typename Options>
	static constexpr void parse(Options& options) noexcept
	{
		using prescaler_traits = typename timer_traits<Timer>::prescaler_traits;
		static_assert(!std::is_same_v<prescaler_traits, void>,
			"Prescaler for the timer is not supported");
		if constexpr (!std::is_same_v<prescaler_traits, void>)
		{
			constexpr auto result = get_prescaler_value();
			static_assert(!result.valid || result.precise,
				"Unable to calculate prescaler for selected max error value");
			static_assert(result.valid,
				"Unable to calculate prescaler for selected timer frequency");
		
			options.prescaler = result.prescaler;
			++options.prescaler_set_count;
		}
	}
};

template<typename Timer, typename ClockConfig, typename FrequencyHz,
	typename MaxErrorHz, bool ExtraPrecision>
struct options_parser<Timer, overflow_frequency<ClockConfig,
	FrequencyHz, MaxErrorHz, ExtraPrecision>>
{
	struct prescaler_and_reload
	{
		uint64_t prescaler_value = unset_value;
		uint64_t reload_value = unset_value;
		long double error = 0;
	};
	
	using timer_frequency_value = decltype(timer_traits<Timer>
		::template get_timer_frequency<ClockConfig>());
	
	template<uint64_t Prescaler>
	static constexpr void calculate_values(prescaler_and_reload& values) noexcept
	{
		if constexpr (timer_traits<Timer>::template has_prescaler<Prescaler>())
		{
			using reload_value = std::ratio_divide<
				std::ratio_divide<timer_frequency_value, std::ratio<Prescaler>>,
				FrequencyHz>;
			constexpr auto integer_reload_value = math::round<reload_value>();
			if constexpr (!!integer_reload_value)
			{
				using real_frequency = std::ratio_divide<
					std::ratio_divide<timer_frequency_value, std::ratio<integer_reload_value>>,
					std::ratio<Prescaler>>;
				using frequency_diff = std::conditional_t<
					std::ratio_greater_equal_v<real_frequency, FrequencyHz>,
					std::ratio_subtract<real_frequency, FrequencyHz>,
					std::ratio_subtract<FrequencyHz, real_frequency>
				>;
				constexpr bool is_valid = timer_traits<Timer>
					::reload_value_limits::template is_valid<integer_reload_value>()
					&& std::ratio_less_equal_v<frequency_diff, MaxErrorHz>;
				
				if constexpr (is_valid)
				{
					constexpr auto error = static_cast<long double>(frequency_diff::num)
						/ static_cast<long double>(frequency_diff::den);
					if (error < values.error || values.prescaler_value == unset_value)
					{
						values.error = error;
						values.prescaler_value = Prescaler;
						values.reload_value = integer_reload_value;
					}
				}
			}
		}
	}
	
	template<uint64_t... Prescalers>
	static constexpr prescaler_and_reload calculate_values(prescaler_sequence<Prescalers...>) noexcept
	{
		prescaler_and_reload values {};
		(..., calculate_values<Prescalers>(values));
		return values;
	}
	
	static constexpr prescaler_and_reload calculate_values(uint64_t max_prescaler) noexcept
	{
		prescaler_and_reload values {};
		auto target_frequency = static_cast<long double>(FrequencyHz::num) / FrequencyHz::den;
		auto max_error_hz = static_cast<long double>(MaxErrorHz::num) / MaxErrorHz::den;
		uint64_t min_prescaler = timer_traits<Timer>::prescaler_traits::limits::min_value;
		for (uint64_t prescaler = min_prescaler; prescaler <= max_prescaler; ++prescaler)
		{
			auto integer_reload_value = static_cast<uint64_t>(
				static_cast<long double>(timer_frequency_value::num)
				/ timer_frequency_value::den
				/ prescaler
				/ target_frequency
				+ 0.5
			);
			
			if (!integer_reload_value)
				continue;
			
			auto real_frequency = static_cast<long double>(timer_frequency_value::num)
				/ timer_frequency_value::den
				/ integer_reload_value
				/ prescaler;
			
			auto frequency_diff = real_frequency > target_frequency
				? real_frequency - target_frequency
				: target_frequency - real_frequency;
			
			bool is_valid =
				integer_reload_value >= timer_traits<Timer>::reload_value_limits::min_value
				&& integer_reload_value <= timer_traits<Timer>::reload_value_limits::max_value
				&& frequency_diff <= max_error_hz;
				
			if (is_valid)
			{
				if (frequency_diff < values.error || values.prescaler_value == unset_value)
				{
					values.error = frequency_diff;
					values.prescaler_value = prescaler;
					values.reload_value = integer_reload_value;
				}
			}
		}
		
		return values;
	}
	
	static constexpr prescaler_and_reload calculate_values_discrete_prescaler() noexcept
	{
		constexpr auto values = calculate_values(
			typename timer_traits<Timer>::prescaler_traits::prescalers {});
		static_assert(values.prescaler_value != unset_value,
			"Unable to calculate prescaler and reload value for the requirements present");
		return values;
	}
	
	template<uint64_t MaxPrescaler>
	static constexpr prescaler_and_reload calculate_values_variable_prescaler() noexcept
	{
		if constexpr (ExtraPrecision)
			return calculate_values(std::make_integer_sequence<uint64_t, MaxPrescaler + 1>());
		else
			return calculate_values(MaxPrescaler);
	}
	
	static constexpr prescaler_and_reload calculate_values_variable_prescaler() noexcept
	{
		using freq_quotent = std::ratio_divide<timer_frequency_value, FrequencyHz>;
		
		constexpr uint64_t max_prescaler = math::min_value<uint64_t,
			math::sqrt(freq_quotent::num / freq_quotent::den + 1) + 1,
			timer_traits<Timer>::prescaler_traits::limits::max_value
		>();
		
		constexpr auto values = calculate_values_variable_prescaler<max_prescaler>();
		static_assert(values.prescaler_value != unset_value,
			"Unable to calculate prescaler and reload value for the requirements present");
		return values;
	}
	
	static constexpr prescaler_and_reload calculate_values() noexcept
	{
		using reload_value_limits = typename timer_traits<Timer>::reload_value_limits;
		static_assert(!std::is_same_v<reload_value_limits, void>,
			"Reload value for the timer is not supported");
		if constexpr (!std::is_same_v<reload_value_limits, void>)
		{
			if constexpr (timer_traits<Timer>::is_discrete_prescaler())
				return calculate_values_discrete_prescaler();
			else
				return calculate_values_variable_prescaler();
		}
		else
		{
			return {};
		}
	}
	
	template<typename Options>
	static constexpr void parse(Options& options) noexcept
	{
		constexpr auto values = calculate_values();
		options.prescaler = values.prescaler_value;
		options.reload_value = values.reload_value;
		++options.prescaler_set_count;
		++options.reload_value_set_count;
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda options_lambda) noexcept
	{
		constexpr auto options = options_lambda();
		static_assert(options.prescaler_set_count < 2,
			"Duplicate or conflicting timer prescaler configuration options");
		static_assert(options.reload_value_set_count < 2,
			"Duplicate or conflicting timer reload value configuration options");
	}
};

} //namespace detail

enum class count_direction
{
	up,
	down,
	other
};

template<typename Timer>
using peripheral_type = typename detail::timer_traits<Timer>::peripheral;

template<typename Timer>
using counter_type = typename detail::timer_traits<Timer>::counter_type;

template<typename Timer>
[[maybe_unused]] constexpr auto max_value = detail::timer_traits<Timer>::max_value;

template<typename Timer, typename Interrupt>
using interrupt_type = typename detail::interrupt_type_helper<Timer, Interrupt>::type;

template<typename Timer, uint64_t Prescaler>
[[maybe_unused]] constexpr bool prescaler_supported
	= detail::timer_traits<Timer>::template has_prescaler<Prescaler>();

template<typename Timer>
using reload_value_limits = typename detail::timer_traits<Timer>::reload_value_limits;

} //namespace mcutl::timer
