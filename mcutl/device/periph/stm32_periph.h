#pragma once

#include <stdint.h>

#include "mcutl/device/device.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph_defs.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::periph
{

struct periph_config
{
	uint16_t ahb_enr = 0;
	uint32_t apb1_enr = 0;
	uint32_t apb2_enr = 0;
	uint32_t apb1_rst = 0;
	uint32_t apb2_rst = 0;
	
	uint16_t ahb_enr_changed = 0;
	uint32_t apb1_enr_changed = 0;
	uint32_t apb2_enr_changed = 0;
	uint32_t apb1_rst_changed = 0;
	uint32_t apb2_rst_changed = 0;
	
	bool ahb_enr_conflict = false;
	bool apb1_enr_conflict = false;
	bool apb2_enr_conflict = false;
	bool apb1_rst_conflict = false;
	bool apb2_rst_conflict = false;
	
	template<uint16_t Bits>
	constexpr auto set_ahb_enr_bits() noexcept
	{
		ahb_enr_conflict = ahb_enr_conflict || (ahb_enr_changed & Bits);
		ahb_enr_changed |= Bits;
		ahb_enr |= Bits;
	}
	
	template<uint16_t Bits>
	constexpr auto reset_ahb_enr_bits() noexcept
	{
		ahb_enr_conflict = ahb_enr_conflict || (ahb_enr_changed & Bits);
		ahb_enr_changed |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto set_apb1_enr_bits() noexcept
	{
		apb1_enr_conflict = apb1_enr_conflict || (apb1_enr_changed & Bits);
		apb1_enr_changed |= Bits;
		apb1_enr |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto reset_apb1_enr_bits() noexcept
	{
		apb1_enr_conflict = apb1_enr_conflict || (apb1_enr_changed & Bits);
		apb1_enr_changed |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto set_apb1_rst_bits() noexcept
	{
		apb1_rst_conflict = apb1_rst_conflict || (apb1_rst_changed & Bits);
		apb1_rst_changed |= Bits;
		apb1_rst |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto clear_apb1_rst_bits() noexcept
	{
		apb1_rst_conflict = apb1_rst_conflict || (apb1_rst_changed & Bits);
		apb1_rst_changed |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto set_apb2_enr_bits() noexcept
	{
		apb2_enr_conflict = apb2_enr_conflict || (apb2_enr_changed & Bits);
		apb2_enr_changed |= Bits;
		apb2_enr |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto reset_apb2_enr_bits() noexcept
	{
		apb2_enr_conflict = apb2_enr_conflict || (apb2_enr_changed & Bits);
		apb2_enr_changed |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto set_apb2_rst_bits() noexcept
	{
		apb2_rst_conflict = apb2_rst_conflict || (apb2_rst_changed & Bits);
		apb2_rst_changed |= Bits;
		apb2_rst |= Bits;
	}
	
	template<uint32_t Bits>
	constexpr auto clear_apb2_rst_bits() noexcept
	{
		apb2_rst_conflict = apb2_rst_conflict || (apb2_rst_changed & Bits);
		apb2_rst_changed |= Bits;
	}
};

template<typename PeripheralConfigLambda>
constexpr void validate_config(PeripheralConfigLambda config_lambda) noexcept
{
	constexpr auto config = config_lambda();
	static_assert(!config.ahb_enr_conflict, "AHB peripheral config conflicting definitions");
	static_assert(!config.apb1_enr_conflict, "APB1 peripheral config conflicting definitions");
	static_assert(!config.apb2_enr_conflict, "APB2 peripheral config conflicting definitions");
	static_assert(!config.apb1_rst_conflict, "APB1 peripheral reset config conflicting definitions");
	static_assert(!config.apb2_rst_conflict, "APB2 peripheral reset config conflicting definitions");
}

template<typename PeripheralConfigLambda>
void configure_peripheral(PeripheralConfigLambda config_lambda) MCUTL_NOEXCEPT
{
	static constexpr auto config = config_lambda();
	if constexpr (!!config.ahb_enr_changed)
	{
		memory::set_register_bits<config.ahb_enr_changed, config.ahb_enr,
			&RCC_TypeDef::AHBENR, RCC_BASE>();
		[[maybe_unused]] auto value = memory::get_register_bits<&RCC_TypeDef::AHBENR, RCC_BASE>();
	}
	if constexpr (!!config.apb1_enr_changed)
	{
		memory::set_register_bits<config.apb1_enr_changed, config.apb1_enr,
			&RCC_TypeDef::APB1ENR, RCC_BASE>();
		[[maybe_unused]] auto value = memory::get_register_bits<&RCC_TypeDef::APB1ENR, RCC_BASE>();
	}
	if constexpr (!!config.apb2_enr_changed)
	{
		memory::set_register_bits<config.apb2_enr_changed, config.apb2_enr,
			&RCC_TypeDef::APB2ENR, RCC_BASE>();
		[[maybe_unused]] auto value = memory::get_register_bits<&RCC_TypeDef::APB2ENR, RCC_BASE>();
	}
	memory::set_register_bits<config.apb1_rst_changed, config.apb1_rst,
		&RCC_TypeDef::APB1RSTR, RCC_BASE>();
	memory::set_register_bits<config.apb2_rst_changed, config.apb2_rst,
		&RCC_TypeDef::APB2RSTR, RCC_BASE>();
}

template<uint32_t EnableDisableBit, uint32_t ResetBit>
struct ahb_peripheral_configurer_base
{
	static constexpr void enable(periph_config& config) noexcept
	{
		config.set_ahb_enr_bits<EnableDisableBit>();
	}
	static constexpr void disable(periph_config& config) noexcept
	{
		config.reset_ahb_enr_bits<EnableDisableBit>();
	}
	static constexpr void reset() noexcept
	{
		check();
	}
	static constexpr void undo_reset() noexcept
	{
		check();
	}
	template<typename T = void>
	static constexpr void check() noexcept
	{
		static_assert(types::always_false<T>::value,
			"Reset is not supported by selected AHB peripheral");
	}
};

template<uint32_t EnableDisableBit, uint32_t ResetBit>
struct apb1_peripheral_configurer_base
{
	static constexpr void enable(periph_config& config) noexcept
	{
		config.set_apb1_enr_bits<EnableDisableBit>();
	}
	static constexpr void disable(periph_config& config) noexcept
	{
		config.reset_apb1_enr_bits<EnableDisableBit>();
	}
	static constexpr void reset(periph_config& config) noexcept
	{
		config.set_apb1_rst_bits<ResetBit>();
	}
	static constexpr void undo_reset(periph_config& config) noexcept
	{
		config.clear_apb1_rst_bits<ResetBit>();
	}
};

template<uint32_t EnableDisableBit, uint32_t ResetBit>
struct apb2_peripheral_configurer_base
{
	static constexpr void enable(periph_config& config) noexcept
	{
		config.set_apb2_enr_bits<EnableDisableBit>();
	}
	static constexpr void disable(periph_config& config) noexcept
	{
		config.reset_apb2_enr_bits<EnableDisableBit>();
	}
	static constexpr void reset(periph_config& config) noexcept
	{
		config.set_apb2_rst_bits<EnableDisableBit>();
	}
	static constexpr void undo_reset(periph_config& config) noexcept
	{
		config.clear_apb2_rst_bits<EnableDisableBit>();
	}
};

} //namespace mcutl::device::periph

namespace mcutl::periph
{

#ifdef ADC1_BASE
struct adc1 : detail::peripheral_base {};
#endif //ADC1_BASE
#ifdef ADC2_BASE
struct adc2 : detail::peripheral_base {};
#endif //ADC2_BASE
#ifdef ADC3_BASE
struct adc3 : detail::peripheral_base {};
#endif //ADC3_BASE

#ifdef AFIO_BASE
struct afio : detail::peripheral_base {};
#endif //AFIO_BASE

#ifdef BKP_BASE
struct bkp : detail::peripheral_base {};
#endif //BKP_BASE

#ifdef CAN1_BASE
struct can1 : detail::peripheral_base {};
#endif //CAN1_BASE
#ifdef CAN2_BASE
struct can2 : detail::peripheral_base {};
#endif //CAN2_BASE

#ifdef CRC_BASE
struct crc : detail::peripheral_base {};
#endif //CRC_BASE

#ifdef DAC_BASE
struct dac : detail::peripheral_base {};
#endif //DAC_BASE

#ifdef DMA1_BASE
struct dma1 : detail::peripheral_base {};
#endif //DMA1_BASE
#ifdef DMA2_BASE
struct dma2 : detail::peripheral_base {};
#endif //DMA2_BASE

#ifdef RCC_AHBENR_ETHMACRXEN
struct ethmacrx : detail::peripheral_base {};
#endif //RCC_AHBENR_ETHMACRXEN
#ifdef RCC_AHBENR_ETHMACTXEN
struct ethmactx : detail::peripheral_base {};
#endif //RCC_AHBENR_ETHMACTXEN
#ifdef RCC_AHBENR_ETHMACEN
struct ethmac : detail::peripheral_base {};
#endif //RCC_AHBENR_ETHMACEN

#ifdef RCC_AHBENR_FLITFEN
struct flitf : detail::peripheral_base {};
#endif //RCC_AHBENR_FLITFEN
#ifdef FSMC_BASE
struct fsmc : detail::peripheral_base {};
#endif //FSMC_BASE

#ifdef GPIOA_BASE
struct gpioa : detail::peripheral_base {};
#endif //GPIOA_BASE
#ifdef GPIOB_BASE
struct gpiob : detail::peripheral_base {};
#endif //GPIOB_BASE
#ifdef GPIOC_BASE
struct gpioc : detail::peripheral_base {};
#endif //GPIOC_BASE
#ifdef GPIOD_BASE
struct gpiod : detail::peripheral_base {};
#endif //GPIOD_BASE
#ifdef GPIOE_BASE
struct gpioe : detail::peripheral_base {};
#endif //GPIOE_BASE
#ifdef GPIOF_BASE
struct gpiof : detail::peripheral_base {};
#endif //GPIOF_BASE
#ifdef GPIOG_BASE
struct gpiog : detail::peripheral_base {};
#endif //GPIOG_BASE

#ifdef I2C1_BASE
struct i2c1 : detail::peripheral_base {};
#endif //I2C1_BASE
#ifdef I2C2_BASE
struct i2c2 : detail::peripheral_base {};
#endif //I2C2_BASE

#ifdef USB_OTG_FS_PERIPH_BASE
struct otgfs : detail::peripheral_base {};
#endif //USB_OTG_FS_PERIPH_BASE

#ifdef PWR_BASE
struct pwr : detail::peripheral_base {};
#endif //PWR_BASE

#ifdef SDIO_BASE
struct sdio : detail::peripheral_base {};
#endif //SDIO_BASE
#ifdef SRAM_BASE
struct sram : detail::peripheral_base {};
#endif //SRAM_BASE
#ifdef SPI1_BASE
struct spi1 : detail::peripheral_base {};
#endif //SPI1_BASE
#ifdef SPI2_BASE
struct spi2 : detail::peripheral_base {};
#endif //SPI2_BASE
#ifdef SPI3_BASE
struct spi3 : detail::peripheral_base {};
#endif //SPI3_BASE

#ifdef TIM1_BASE
struct timer1 : detail::peripheral_base {};
#endif //TIM1_BASE
#ifdef TIM2_BASE
struct timer2 : detail::peripheral_base {};
#endif //TIM2_BASE
#ifdef TIM3_BASE
struct timer3 : detail::peripheral_base {};
#endif //TIM3_BASE
#ifdef TIM4_BASE
struct timer4 : detail::peripheral_base {};
#endif //TIM4_BASE
#ifdef TIM5_BASE
struct timer5 : detail::peripheral_base {};
#endif //TIM5_BASE
#ifdef TIM6_BASE
struct timer6 : detail::peripheral_base {};
#endif //TIM6_BASE
#ifdef TIM7_BASE
struct timer7 : detail::peripheral_base {};
#endif //TIM7_BASE
#ifdef TIM8_BASE
struct timer8 : detail::peripheral_base {};
#endif //TIM8_BASE
#ifdef TIM9_BASE
struct timer9 : detail::peripheral_base {};
#endif //TIM9_BASE
#ifdef TIM10_BASE
struct timer10 : detail::peripheral_base {};
#endif //TIM10_BASE
#ifdef TIM11_BASE
struct timer11 : detail::peripheral_base {};
#endif //TIM11_BASE
#ifdef TIM12_BASE
struct timer12 : detail::peripheral_base {};
#endif //TIM12_BASE
#ifdef TIM13_BASE
struct timer13 : detail::peripheral_base {};
#endif //TIM13_BASE
#ifdef TIM14_BASE
struct timer14 : detail::peripheral_base {};
#endif //TIM14_BASE

#ifdef USART1_BASE
struct usart1 : detail::peripheral_base {};
#endif //USART1_BASE
#ifdef USART2_BASE
struct usart2 : detail::peripheral_base {};
#endif //USART2_BASE
#ifdef USART3_BASE
struct usart3 : detail::peripheral_base {};
#endif //USART3_BASE
#ifdef UART4_BASE
struct uart4 : detail::peripheral_base {};
#endif //UART4_BASE
#ifdef UART5_BASE
struct uart5 : detail::peripheral_base {};
#endif //UART5_BASE

#ifdef USB_BASE
struct usb : detail::peripheral_base {};
#endif //USB_BASE
#ifdef WWDG_BASE
struct wwdg : detail::peripheral_base {};
#endif //WWDG_BASE

namespace detail
{
#ifdef RCC_AHBENR_SDIOEN
template<> struct peripheral_configurer<sdio>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_SDIOEN, 0u>> {};
#endif //RCC_AHBENR_SDIOEN
#ifdef RCC_AHBENR_FSMCEN
template<> struct peripheral_configurer<fsmc>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_FSMCEN, 0u>> {};
#endif //RCC_AHBENR_FSMCEN
#ifdef RCC_AHBENR_CRCEN
template<> struct peripheral_configurer<crc>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_CRCEN, 0u>> {};
#endif //RCC_AHBENR_CRCEN
#ifdef RCC_AHBENR_FLITFEN
template<> struct peripheral_configurer<flitf>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_FLITFEN, 0u>> {};
#endif //RCC_AHBENR_FLITFEN
#ifdef RCC_AHBENR_SRAMEN
template<> struct peripheral_configurer<sram>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_SRAMEN, 0u>> {};
#endif //RCC_AHBENR_SRAMEN
#ifdef RCC_AHBENR_DMA2EN
template<> struct peripheral_configurer<dma2>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_DMA2EN, 0u>> {};
#endif //RCC_AHBENR_DMA2EN
#ifdef RCC_AHBENR_DMA1EN
template<> struct peripheral_configurer<dma1>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_DMA1EN, 0u>> {};
#endif //RCC_AHBENR_DMA1EN
#ifdef RCC_AHBENR_ETHMACRXEN
template<> struct peripheral_configurer<ethmacrx>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_ETHMACRXEN, 0u>> {};
#endif //RCC_AHBENR_ETHMACRXEN
#ifdef RCC_AHBENR_ETHMACTXEN
template<> struct peripheral_configurer<ethmactx>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_ETHMACTXEN, 0u>> {};
#endif //RCC_AHBENR_ETHMACTXEN
#ifdef RCC_AHBENR_ETHMACEN
template<> struct peripheral_configurer<ethmac>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_ETHMACEN, 0u>> {};
#endif //RCC_AHBENR_ETHMACEN
#ifdef RCC_AHBENR_OTGFSEN
template<> struct peripheral_configurer<otgfs>
	: router<device::periph::ahb_peripheral_configurer_base<RCC_AHBENR_OTGFSEN, 0u>> {};
#endif //RCC_AHBENR_OTGFSEN

#ifdef RCC_APB1ENR_DACEN
template<> struct peripheral_configurer<dac>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_DACEN, RCC_APB1RSTR_DACRST>> {};
#endif //RCC_APB1ENR_DACEN
#ifdef RCC_APB1ENR_PWREN
template<> struct peripheral_configurer<pwr>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_PWREN, RCC_APB1RSTR_PWRRST>> {};
#endif //RCC_APB1ENR_PWREN
#ifdef RCC_APB1ENR_BKPEN
template<> struct peripheral_configurer<bkp>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_BKPEN, RCC_APB1RSTR_BKPRST>> {};
#endif //RCC_APB1ENR_BKPEN
#ifdef RCC_APB1ENR_CAN1EN
template<> struct peripheral_configurer<can1>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_CAN1EN, RCC_APB1RSTR_CAN1RST>> {};
#endif //RCC_APB1ENR_CAN1EN
#ifdef RCC_APB1ENR_CAN2EN
template<> struct peripheral_configurer<can2>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_CAN2EN, RCC_APB1RSTR_CAN2RST>> {};
#endif //RCC_APB1ENR_CAN2EN
#ifdef RCC_APB1ENR_USBEN
template<> struct peripheral_configurer<usb>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_USBEN, RCC_APB1RSTR_USBRST>> {};
#endif //RCC_APB1ENR_USBEN
#ifdef RCC_APB1ENR_I2C1EN
template<> struct peripheral_configurer<i2c1>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_I2C1EN, RCC_APB1RSTR_I2C1RST>> {};
#endif //RCC_APB1ENR_I2C1EN
#ifdef RCC_APB1ENR_I2C2EN
template<> struct peripheral_configurer<i2c2>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_I2C2EN, RCC_APB1RSTR_I2C2RST>> {};
#endif //RCC_APB1ENR_I2C2EN
#ifdef RCC_APB1ENR_UART5EN
template<> struct peripheral_configurer<uart5>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_UART5EN, RCC_APB1RSTR_UART5RST>> {};
#endif //RCC_APB1ENR_UART5EN
#ifdef RCC_APB1ENR_UART4EN
template<> struct peripheral_configurer<uart4>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_UART4EN, RCC_APB1RSTR_UART4RST>> {};
#endif //RCC_APB1ENR_UART4EN
#ifdef RCC_APB1ENR_USART3EN
template<> struct peripheral_configurer<usart3>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_USART3EN, RCC_APB1RSTR_USART3RST>> {};
#endif //RCC_APB1ENR_USART3EN
#ifdef RCC_APB1ENR_USART2EN
template<> struct peripheral_configurer<usart2>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_USART2EN, RCC_APB1RSTR_USART2RST>> {};
#endif //RCC_APB1ENR_USART2EN
#ifdef RCC_APB1ENR_SPI3EN
template<> struct peripheral_configurer<spi3>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_SPI3EN, RCC_APB1RSTR_SPI3RST>> {};
#endif //RCC_APB1ENR_SPI3EN
#ifdef RCC_APB1ENR_SPI2EN
template<> struct peripheral_configurer<spi2>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_SPI2EN, RCC_APB1RSTR_SPI2RST>> {};
#endif //RCC_APB1ENR_SPI2EN
#ifdef RCC_APB1ENR_WWDGEN
template<> struct peripheral_configurer<wwdg>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_WWDGEN, RCC_APB1RSTR_WWDGRST>> {};
#endif //RCC_APB1ENR_WWDGEN
#ifdef RCC_APB1ENR_TIM14EN
template<> struct peripheral_configurer<timer14> 
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM14EN, RCC_APB1RSTR_TIM14RST>> {};
#endif //RCC_APB1ENR_TIM14EN
#ifdef RCC_APB1ENR_TIM13EN
template<> struct peripheral_configurer<timer13>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM13EN, RCC_APB1RSTR_TIM13RST>> {};
#endif //RCC_APB1ENR_TIM13EN
#ifdef RCC_APB1ENR_TIM12EN
template<> struct peripheral_configurer<timer12>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM12EN, RCC_APB1RSTR_TIM12RST>> {};
#endif //RCC_APB1ENR_TIM12EN
#ifdef RCC_APB1ENR_TIM7EN
template<> struct peripheral_configurer<timer7>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM7EN, RCC_APB1RSTR_TIM7RST>> {};
#endif //RCC_APB1ENR_TIM7EN
#ifdef RCC_APB1ENR_TIM6EN
template<> struct peripheral_configurer<timer6>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM6EN, RCC_APB1RSTR_TIM6RST>> {};
#endif //RCC_APB1ENR_TIM6EN
#ifdef RCC_APB1ENR_TIM5EN
template<> struct peripheral_configurer<timer5>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM5EN, RCC_APB1RSTR_TIM5RST>> {};
#endif //RCC_APB1ENR_TIM5EN
#ifdef RCC_APB1ENR_TIM4EN
template<> struct peripheral_configurer<timer4>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM4EN, RCC_APB1RSTR_TIM4RST>> {};
#endif //RCC_APB1ENR_TIM4EN
#ifdef RCC_APB1ENR_TIM3EN
template<> struct peripheral_configurer<timer3>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM3EN, RCC_APB1RSTR_TIM3RST>> {};
#endif //RCC_APB1ENR_TIM3EN
#ifdef RCC_APB1ENR_TIM2EN
template<> struct peripheral_configurer<timer2>
	: router<device::periph::apb1_peripheral_configurer_base<RCC_APB1ENR_TIM2EN, RCC_APB1RSTR_TIM2RST>> {};
#endif //RCC_APB1ENR_TIM2EN

#ifdef RCC_APB2ENR_TIM11EN
template<> struct peripheral_configurer<timer11>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_TIM11EN, RCC_APB2RSTR_TIM11RST>> {};
#endif //RCC_APB2ENR_TIM11EN
#ifdef RCC_APB2ENR_TIM10EN
template<> struct peripheral_configurer<timer10>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_TIM10EN, RCC_APB2RSTR_TIM10RST>> {};
#endif //RCC_APB2ENR_TIM10EN
#ifdef RCC_APB2ENR_TIM9EN
template<> struct peripheral_configurer<timer9>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_TIM9EN, RCC_APB2RSTR_TIM9RST>> {};
#endif //RCC_APB2ENR_TIM9EN
#ifdef RCC_APB2ENR_ADC3EN
template<> struct peripheral_configurer<adc3>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_ADC3EN, RCC_APB2RSTR_ADC3RST>> {};
#endif //RCC_APB2ENR_ADC3EN
#ifdef RCC_APB2ENR_USART1EN
template<> struct peripheral_configurer<usart1>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_USART1EN, RCC_APB2RSTR_USART1RST>> {};
#endif //RCC_APB2ENR_USART1EN
#ifdef RCC_APB2ENR_TIM8EN
template<> struct peripheral_configurer<timer8>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_TIM8EN, RCC_APB2RSTR_TIM8RST>> {};
#endif //RCC_APB2ENR_TIM8EN
#ifdef RCC_APB2ENR_SPI1EN
template<> struct peripheral_configurer<spi1>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_SPI1EN, RCC_APB2RSTR_SPI1RST>> {};
#endif //RCC_APB2ENR_SPI1EN
#ifdef RCC_APB2ENR_TIM1EN
template<> struct peripheral_configurer<timer1>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_TIM1EN, RCC_APB2RSTR_TIM1RST>> {};
#endif //RCC_APB2ENR_TIM1EN
#ifdef RCC_APB2ENR_ADC2EN
template<> struct peripheral_configurer<adc2>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_ADC2EN, RCC_APB2RSTR_ADC2RST>> {};
#endif //RCC_APB2ENR_ADC2EN
#ifdef RCC_APB2ENR_ADC1EN
template<> struct peripheral_configurer<adc1>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_ADC1EN, RCC_APB2RSTR_ADC1RST>> {};
#endif //RCC_APB2ENR_ADC1EN
#ifdef RCC_APB2ENR_IOPAEN
template<> struct peripheral_configurer<gpioa>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPAEN, RCC_APB2RSTR_IOPARST>> {};
#endif //RCC_APB2ENR_IOPAEN
#ifdef RCC_APB2ENR_IOPBEN
template<> struct peripheral_configurer<gpiob>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPBEN, RCC_APB2RSTR_IOPBRST>> {};
#endif //RCC_APB2ENR_IOPBEN
#ifdef RCC_APB2ENR_IOPCEN
template<> struct peripheral_configurer<gpioc>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPCEN, RCC_APB2RSTR_IOPCRST>> {};
#endif //RCC_APB2ENR_IOPCEN
#ifdef RCC_APB2ENR_IOPDEN
template<> struct peripheral_configurer<gpiod>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPDEN, RCC_APB2RSTR_IOPDRST>> {};
#endif //RCC_APB2ENR_IOPDEN
#ifdef RCC_APB2ENR_IOPEEN
template<> struct peripheral_configurer<gpioe>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPEEN, RCC_APB2RSTR_IOPERST>> {};
#endif //RCC_APB2ENR_IOPEEN
#ifdef RCC_APB2ENR_IOPFEN
template<> struct peripheral_configurer<gpiof>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPFEN, RCC_APB2RSTR_IOPFRST>> {};
#endif //RCC_APB2ENR_IOPFEN
#ifdef RCC_APB2ENR_IOPGEN
template<> struct peripheral_configurer<gpiog>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_IOPGEN, RCC_APB2RSTR_IOPGRST>> {};
#endif //RCC_APB2ENR_IOPGEN
#ifdef RCC_APB2ENR_AFIOEN
template<> struct peripheral_configurer<afio>
	: router<device::periph::apb2_peripheral_configurer_base<RCC_APB2ENR_AFIOEN, RCC_APB2RSTR_AFIORST>> {};
#endif //RCC_APB2ENR_AFIOEN
} //namespace detail

} //namespace mcutl::periph
