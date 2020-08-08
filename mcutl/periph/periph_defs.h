#pragma once

#include <type_traits>

#include "mcutl/utils/type_helpers.h"

namespace mcutl::periph
{

namespace detail
{
struct peripheral_base {};

struct enable {};
struct disable {};
struct reset {};
struct undo_reset {};

template<typename Peripheral, typename ConfigType>
struct peripheral_configuration
{
	peripheral_configuration() = delete;
	
	static_assert(std::is_base_of_v<detail::peripheral_base, Peripheral>,
		"Invalid peripheral");
	
	using peripheral_type = Peripheral;
	using config_type = ConfigType;
};

template<typename Configurer>
struct router
{
	template<typename Configuration, typename ConfigStruct>
	static constexpr void execute(ConfigStruct& config) noexcept
	{
		if constexpr (std::is_same_v<Configuration, enable>)
			Configurer::enable(config);
		else if constexpr (std::is_same_v<Configuration, disable>)
			Configurer::disable(config);
		else if constexpr (std::is_same_v<Configuration, reset>)
			Configurer::reset(config);
		else if constexpr (std::is_same_v<Configuration, undo_reset>)
			Configurer::undo_reset(config);
	}
};

template<typename Peripheral>
struct peripheral_configurer
{
	template<typename Configuration, typename ConfigStruct>
	static constexpr void execute(ConfigStruct&) noexcept
	{
		static_assert(types::always_false<Configuration>::value,
			"Peripheral is not supported on selected MCU");
	}
};

template<typename PeripheralConfig>
struct peripheral_configurer_selector
{
	template<typename ConfigStruct>
	static constexpr auto configure(ConfigStruct& config) noexcept
	{
		return peripheral_configurer<typename PeripheralConfig::peripheral_type>
			::template execute<typename PeripheralConfig::config_type>(config);
	}
};

} //namespace detail

template<typename Peripheral>
struct enable : detail::peripheral_configuration<Peripheral, detail::enable> {};
template<typename Peripheral>
struct disable : detail::peripheral_configuration<Peripheral, detail::disable> {};
template<typename Peripheral>
struct reset : detail::peripheral_configuration<Peripheral, detail::reset> {};
template<typename Peripheral>
struct undo_reset : detail::peripheral_configuration<Peripheral, detail::undo_reset> {};

template<typename... Options>
struct config {};

struct no_periph : detail::peripheral_base {};

namespace detail
{

template<> struct peripheral_configurer<no_periph>
{
	template<typename Configuration, typename ConfigStruct>
	static constexpr void execute(ConfigStruct&) noexcept
	{
	}
};

} //namespace detail

} //namespace mcutl::periph
