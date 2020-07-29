#pragma once

#include <type_traits>
#include <stdint.h>

#include "mcutl/utils/type_helpers.h"

namespace mcutl::gpio
{

template<char PortLetter, uint32_t PinNumber>
struct pin
{
	static constexpr char port_letter = PortLetter;
	static constexpr uint32_t pin_number = PinNumber;
};

namespace out
{

struct push_pull {};
struct open_drain {};
struct push_pull_alt_func {};
struct open_drain_alt_func {};

struct one : std::bool_constant<true> {};
struct zero : std::bool_constant<false> {};
struct keep_value {};

namespace opt
{

struct atomic {};

} //namespace opt

template<typename Value>
struct negate
{
	static_assert(types::always_false<Value>::value,
		"Unable to negate unknown pin value");
};
template<> struct negate<one> { using type = zero; };
template<> struct negate<zero> { using type = one; };

template<typename Value>
using negate_t = typename negate<Value>::type;

} //namespace out

namespace detail
{

template<typename Pin, typename... Options>
struct config_base
{
	config_base() = delete;
	using pin = Pin;
	using options = types::list<Options...>;
};

} //namespace detail

template<typename Pin, typename OutputMode,
	typename OutValue = out::keep_value, typename... OutputOptions>
struct as_output : detail::config_base<Pin, OutputOptions...>
{
	using output_mode = OutputMode;
	using out_value = OutValue;
	
	static_assert(std::is_same_v<out_value, out::one>
		|| std::is_same_v<out_value, out::zero>
		|| std::is_same_v<out_value, out::keep_value>, "Invalid GPIO output value");
};

template<typename Pin, typename OutValue, typename... OutputOptions>
struct to_value : detail::config_base<Pin, OutputOptions...>
{
	using out_value = OutValue;
	
	static_assert(std::is_same_v<out_value, out::one>
		|| std::is_same_v<out_value, out::zero>, "Invalid GPIO output value");
};

namespace in
{

struct analog {};
struct floating {};
struct pull_up {};
struct pull_down {};

} //namespace in

template<typename Pin, typename InputMode, typename... InputOptions>
struct as_input : detail::config_base<Pin, InputOptions...>
{
	using input_mode = InputMode;
};

struct enable_peripherals {};

namespace detail
{

struct out_option {};
struct in_option {};
struct unknown_option_category {};

template<typename Option>
struct option_category : types::identity<unknown_option_category> {};

template<typename Option>
using option_category_t = typename option_category<Option>::type;

template<> struct option_category<out::keep_value> : types::identity<out_option> {};
template<> struct option_category<out::one> : types::identity<out_option> {};
template<> struct option_category<out::zero> : types::identity<out_option> {};
template<> struct option_category<out::open_drain> : types::identity<out_option> {};
template<> struct option_category<out::open_drain_alt_func> : types::identity<out_option> {};
template<> struct option_category<out::push_pull> : types::identity<out_option> {};
template<> struct option_category<out::push_pull_alt_func> : types::identity<out_option> {};
template<> struct option_category<in::analog> : types::identity<in_option> {};
template<> struct option_category<in::floating> : types::identity<in_option> {};
template<> struct option_category<in::pull_down> : types::identity<in_option> {};
template<> struct option_category<in::pull_up> : types::identity<in_option> {};

template<char PortLetter, uint32_t PinNumber> struct gpio_peripheral_map
{
	static_assert(types::value_always_false<PortLetter>::value,
		"No peripheral mapping for the pin specified");
};

} //namespace detail

template<typename... PinConfig>
struct config : types::list<PinConfig...> {};

template<typename Pin>
using to_periph = typename detail::gpio_peripheral_map<
	Pin::port_letter, Pin::pin_number>::type;

} //namespace mcutl::gpio

namespace mcutl::device::gpio
{

template<char PortLetter, uint32_t... PinNumbers>
struct valid_gpio_base
{
	static constexpr char port_letter = PortLetter;
};

template<typename... RegBases>
struct valid_gpio_list
{
};

} //namespace mcutl::device::gpio
