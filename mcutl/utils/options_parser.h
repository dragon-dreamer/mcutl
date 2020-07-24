#pragma once

namespace mcutl::opts
{

template<auto OptionValue, auto OptionValueField, auto OptionValueSetCountField>
struct base_option_parser
{
	template<typename Option>
	static constexpr void parse(Option& options) noexcept
	{
		if constexpr (OptionValueField != nullptr)
			options.*OptionValueField = OptionValue;
		
		if constexpr (OptionValueSetCountField != nullptr)
			++(options.*OptionValueSetCountField);
	}
	
	template<typename OptionsLambda>
	static constexpr void validate([[maybe_unused]] OptionsLambda options_lambda) noexcept
	{
		if constexpr (OptionValueSetCountField != nullptr)
		{
			static_assert(options_lambda().*OptionValueSetCountField < 2,
				"Duplicate or conflicting peripheral configuration options");
		}
	}
};

template<auto OptionValue, auto Index, auto OptionValueArray, auto OptionValueSetCountArray>
struct base_array_option_parser
{
	template<typename Option>
	static constexpr void parse(Option& options) noexcept
	{
		(options.*OptionValueArray)[Index] = OptionValue;
		if constexpr (OptionValueSetCountArray != nullptr)
			++((options.*OptionValueSetCountArray)[Index]);
	}
	
	template<typename OptionsLambda>
	static constexpr void validate(OptionsLambda options_lambda) noexcept
	{
		if constexpr (OptionValueSetCountArray != nullptr)
		{
			static_assert((options_lambda().*OptionValueSetCountArray)[Index] < 2,
				"Duplicate or conflicting peripheral configuration options");
		}
	}
};

template<typename Result, template<typename, typename> class OptionsParser,
	typename Peripheral, typename... Options>
constexpr auto parse_options() noexcept
{
	Result result {};
	(..., OptionsParser<Peripheral, Options>::template parse(result));
	return result;
}

template<typename Result, template<typename, typename> class OptionsParser,
	typename Peripheral, typename... Options>
constexpr auto parse_and_validate_options() noexcept
{
	(..., OptionsParser<Peripheral, Options>::template validate([]() constexpr {
		return parse_options<Result, OptionsParser, Peripheral, Options...>(); }));
	return parse_options<Result, OptionsParser, Peripheral, Options...>();
}

} //namespace mcutl::opts
