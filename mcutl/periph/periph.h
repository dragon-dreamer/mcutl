#pragma once

#include "mcutl/periph/periph_defs.h"
#include "mcutl/device/periph/device_periph.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::periph
{

namespace detail
{

template<typename PeripheralConfig, typename ConfigStruct>
constexpr void process_peripheral_config(ConfigStruct& config) noexcept
{
	peripheral_configurer_selector<PeripheralConfig>::configure(config);
}
	
template<typename... PeripheralConfig>
constexpr auto get_config() noexcept
{
	device::periph::periph_config config {};
	(..., process_peripheral_config<PeripheralConfig>(config));
	return config;
}

template<typename... PeripheralConfig>
struct config_helper
{
	static constexpr auto get_and_validate_config() noexcept
	{
		device::periph::validate_config([] () constexpr { return get_config<PeripheralConfig...>(); });
		return get_config<PeripheralConfig...>();
	}
};

template<typename... PeripheralConfig>
struct config_helper<config<PeripheralConfig...>>
{
	static constexpr auto get_and_validate_config() noexcept
	{
		return config_helper<PeripheralConfig...>::get_and_validate_config();
	}
};

} //namespace detail

template<typename... PeripheralConfig>
void configure_peripheral() MCUTL_NOEXCEPT
{
	device::periph::configure_peripheral(
		[] () constexpr { return detail::config_helper<PeripheralConfig...>::get_and_validate_config(); });
}

} //namespace mcutl::periph
