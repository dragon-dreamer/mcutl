#pragma once

#include <stdint.h>
#include <type_traits>
#include <utility>

#include "mcutl/device/device.h"
#include "mcutl/gpio/gpio_defs.h"
#include "mcutl/periph/periph.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::gpio
{

#ifdef GPIOA_BASE
template<uint32_t PinNumber>
using gpioa = pin<'a', PinNumber>;
#endif //GPIOA_BASE

#ifdef GPIOB_BASE
template<uint32_t PinNumber>
using gpiob = pin<'b', PinNumber>;
#endif //GPIOB_BASE

#ifdef GPIOC_BASE
template<uint32_t PinNumber>
using gpioc = pin<'c', PinNumber>;
#endif //GPIOC_BASE

#ifdef GPIOD_BASE
template<uint32_t PinNumber>
using gpiod = pin<'d', PinNumber>;
#endif //GPIOD_BASE

#ifdef GPIOE_BASE
template<uint32_t PinNumber>
using gpioe = pin<'e', PinNumber>;
#endif //GPIOE_BASE

#ifdef GPIOF_BASE
template<uint32_t PinNumber>
using gpiof = pin<'f', PinNumber>;
#endif //GPIOF_BASE

#ifdef GPIOG_BASE
template<uint32_t PinNumber>
using gpiog = pin<'g', PinNumber>;
#endif //GPIOG_BASE

namespace out::opt
{

struct freq_2mhz {};
struct freq_10mhz {};
struct freq_50mhz {};

} //namespace out::opt

namespace detail
{

template<> struct option_category<out::opt::freq_2mhz> : types::identity<out_option> {};
template<> struct option_category<out::opt::freq_10mhz> : types::identity<out_option> {};
template<> struct option_category<out::opt::freq_50mhz> : types::identity<out_option> {};

#ifdef GPIOA_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'a', PinNumber> { using type = periph::gpioa; };
#endif //GPIOA_BASE
#ifdef GPIOB_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'b', PinNumber> { using type = periph::gpiob; };
#endif //GPIOB_BASE
#ifdef GPIOC_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'c', PinNumber> { using type = periph::gpioc; };
#endif //GPIOC_BASE
#ifdef GPIOD_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'d', PinNumber> { using type = periph::gpiod; };
#endif //GPIOD_BASE
#ifdef GPIOE_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'e', PinNumber> { using type = periph::gpioe; };
#endif //GPIOE_BASE
#ifdef GPIOF_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'f', PinNumber> { using type = periph::gpiof; };
#endif //GPIOF_BASE
#ifdef GPIOG_BASE
template<uint32_t PinNumber> struct gpio_peripheral_map<'g', PinNumber> { using type = periph::gpiog; };
#endif //GPIOG_BASE

} //namespace detail

} //namespace mcutl::gpio

namespace mcutl::device::gpio
{

template<char PortLetter, uintptr_t PortBase, uint32_t... PinNumbers>
struct valid_gpio : valid_gpio_base<PortLetter, PinNumbers...>
{
};

template<char PortLetter, uintptr_t PortBase>
using valid_gpio_t = valid_gpio<PortLetter, PortBase, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>;

using available_regs_t = mcutl::types::pop_front_t<
	valid_gpio_list<
	void
#ifdef GPIOA_BASE
	, valid_gpio_t<'a', GPIOA_BASE>
#endif //GPIOA_BASE
#ifdef GPIOB_BASE
	, valid_gpio_t<'b', GPIOB_BASE>
#endif //GPIOB_BASE
#ifdef GPIOC_BASE
	, valid_gpio_t<'c', GPIOC_BASE>
#endif //GPIOC_BASE
#ifdef GPIOD_BASE
	, valid_gpio_t<'d', GPIOD_BASE>
#endif //GPIOD_BASE
#ifdef GPIOE_BASE
	, valid_gpio_t<'e', GPIOE_BASE>
#endif //GPIOE_BASE
#ifdef GPIOF_BASE
	, valid_gpio_t<'f', GPIOF_BASE>
#endif //GPIOF_BASE
#ifdef GPIOG_BASE
	, valid_gpio_t<'g', GPIOG_BASE>
#endif //GPIOG_BASE
>>;

template<char TargetPortLetter>
constexpr auto get_port_base_impl(valid_gpio_list<>) noexcept
{
	static_assert(types::value_always_false<TargetPortLetter>::value, "Port letter does not exist");
}

template<char TargetPortLetter, char PortLetter, uintptr_t PortBase, typename... AvailableRegs>
constexpr auto get_port_base_impl(valid_gpio_list<
	valid_gpio_t<PortLetter, PortBase>, AvailableRegs...>) noexcept
{
	if constexpr (TargetPortLetter == PortLetter)
		return PortBase;
	else
		return get_port_base_impl<TargetPortLetter>(valid_gpio_list<AvailableRegs...>{});
}

template<char PortLetter>
constexpr auto get_port_base() noexcept
{
	return get_port_base_impl<PortLetter>(available_regs_t{});
}

template<typename Pin>
struct pin_bit_mask_helper
{
	static constexpr uint32_t get_pin_bit_mask() noexcept
	{
		return (1 << Pin::pin_number);
	}
	
	static constexpr auto get_port_letter() noexcept
	{
		return Pin::port_letter;
	}
};

template<typename Pin, typename... Pins>
struct pin_port_letter_helper
{
	static constexpr auto get_first_pin_port_letter() noexcept
	{
		return Pin::port_letter;
	}
};

template<typename... PinConfigs>
constexpr uint32_t get_pin_bit_mask() noexcept
{
	if constexpr (sizeof...(PinConfigs) == 0)
	{
		return 0;
	}
	else
	{
		static_assert((... && (pin_port_letter_helper<PinConfigs...>::get_first_pin_port_letter()
			== pin_bit_mask_helper<PinConfigs>::get_port_letter())), "All port letters must be equal");
		return (0 | ... | pin_bit_mask_helper<PinConfigs>::get_pin_bit_mask());
	}
}

} //namespace mcutl::device::gpio
