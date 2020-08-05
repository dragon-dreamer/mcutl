#pragma once

#include <stdint.h>

#include "mcutl/device/device.h"
#include "mcutl/interrupt/interrupt_defs.h"
#include "mcutl/instruction/instruction.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/math.h"

namespace mcutl::interrupt::type
{

struct wwdg : detail::interrupt_base<WWDG_IRQn> {};
struct pvd : detail::interrupt_base<PVD_IRQn> {};
struct tamper : detail::interrupt_base<TAMPER_IRQn> {};
struct rtc : detail::interrupt_base<RTC_IRQn> {};
struct rtc_alarm : detail::interrupt_base<RTC_Alarm_IRQn> {};
struct flash : detail::interrupt_base<FLASH_IRQn> {};
struct rcc : detail::interrupt_base<RCC_IRQn> {};
struct exti0 : detail::interrupt_base<EXTI0_IRQn> {};
struct exti1 : detail::interrupt_base<EXTI1_IRQn> {};
struct exti2 : detail::interrupt_base<EXTI2_IRQn> {};
struct exti3 : detail::interrupt_base<EXTI3_IRQn> {};
struct exti4 : detail::interrupt_base<EXTI4_IRQn> {};
struct exti9_5 : detail::interrupt_base<EXTI9_5_IRQn> {};
struct exti15_10 : detail::interrupt_base<EXTI15_10_IRQn> {};
struct dma1_ch1 : detail::interrupt_base<DMA1_Channel1_IRQn> {};
struct dma1_ch2 : detail::interrupt_base<DMA1_Channel2_IRQn> {};
struct dma1_ch3 : detail::interrupt_base<DMA1_Channel3_IRQn> {};
struct dma1_ch4 : detail::interrupt_base<DMA1_Channel4_IRQn> {};
struct dma1_ch5 : detail::interrupt_base<DMA1_Channel5_IRQn> {};
struct dma1_ch6 : detail::interrupt_base<DMA1_Channel6_IRQn> {};
struct dma1_ch7 : detail::interrupt_base<DMA1_Channel7_IRQn> {};
struct dma2_ch1 : detail::interrupt_base<DMA2_Channel1_IRQn> {};
struct dma2_ch2 : detail::interrupt_base<DMA2_Channel2_IRQn> {};
struct dma2_ch3 : detail::interrupt_base<DMA2_Channel3_IRQn> {};
struct adc1_2 : detail::interrupt_base<ADC1_2_IRQn> {};
struct can1_rx1 : detail::interrupt_base<CAN1_RX1_IRQn> {};
struct can1_sce : detail::interrupt_base<CAN1_SCE_IRQn> {};
struct tim1_cc : detail::interrupt_base<TIM1_CC_IRQn> {};
struct tim2 : detail::interrupt_base<TIM2_IRQn> {};
struct tim3 : detail::interrupt_base<TIM3_IRQn> {};
struct tim4 : detail::interrupt_base<TIM4_IRQn> {};
struct tim5 : detail::interrupt_base<TIM5_IRQn> {};
struct tim6 : detail::interrupt_base<TIM6_IRQn> {};
struct tim7 : detail::interrupt_base<TIM7_IRQn> {};
struct i2c1_ev : detail::interrupt_base<I2C1_EV_IRQn> {};
struct i2c1_er : detail::interrupt_base<I2C1_ER_IRQn> {};
struct i2c2_ev : detail::interrupt_base<I2C2_EV_IRQn> {};
struct i2c2_er : detail::interrupt_base<I2C2_ER_IRQn> {};
struct spi1 : detail::interrupt_base<SPI1_IRQn> {};
struct spi2 : detail::interrupt_base<SPI2_IRQn> {};
struct spi3 : detail::interrupt_base<SPI3_IRQn> {};
struct usart1 : detail::interrupt_base<USART1_IRQn> {};
struct usart2 : detail::interrupt_base<USART2_IRQn> {};
struct usart3 : detail::interrupt_base<USART3_IRQn> {};
struct uart4 : detail::interrupt_base<UART4_IRQn> {};
struct uart5 : detail::interrupt_base<UART5_IRQn> {};

#if defined(STM32F105xC) || defined(STM32F107xC) //Connectivity
struct can1_tx : detail::interrupt_base<CAN1_TX_IRQn> {};
struct can1_rx0 : detail::interrupt_base<CAN1_RX0_IRQn> {};
struct dma2_ch5 : detail::interrupt_base<DMA2_Channel5_IRQn> {};
struct eth : detail::interrupt_base<ETH_IRQn> {};
struct eth_wkup : detail::interrupt_base<ETH_WKUP_IRQn> {};
struct can2_tx : detail::interrupt_base<CAN2_TX_IRQn> {};
struct can2_rx0 : detail::interrupt_base<CAN2_RX0_IRQn> {};
struct can2_rx1 : detail::interrupt_base<CAN2_RX1_IRQn> {};
struct can2_sce : detail::interrupt_base<CAN2_SCE_IRQn> {};
struct otg_fs : detail::interrupt_base<OTG_FS_IRQn> {};
struct tim1_brk : detail::interrupt_base<TIM1_BRK_IRQn> {};
struct tim1_up : detail::interrupt_base<TIM1_UP_IRQn> {};
struct tim1_trg_com : detail::interrupt_base<TIM1_TRG_COM_IRQn> {};
struct otg_fs_wkup : detail::interrupt_base<OTG_FS_WKUP_IRQn> {};
struct dma2_ch4 : detail::interrupt_base<DMA2_Channel4_IRQn> {};
#elif defined(STM32F103xG) || defined(STM32F101xG) //XL-density
struct usb_hp_can1_tx : detail::interrupt_base<USB_HP_CAN1_TX_IRQn> {};
struct usb_lp_can1_rx0 : detail::interrupt_base<USB_LP_CAN1_RX0_IRQn> {};
struct tim1_brk_tim9 : detail::interrupt_base<TIM1_BRK_TIM9_IRQn> {};
struct tim1_up_tim10 : detail::interrupt_base<TIM1_UP_TIM10_IRQn> {};
struct tim1_trg_com_tim11 : detail::interrupt_base<TIM1_TRG_COM_TIM11_IRQn> {};
struct usb_wakeup : detail::interrupt_base<USBWakeUp_IRQn> {};
struct tim8_brk_tim12 : detail::interrupt_base<TIM8_BRK_TIM12_IRQn> {};
struct tim8_up_tim13 : detail::interrupt_base<TIM8_UP_TIM13_IRQn> {};
struct tim8_trg_com_tim14 : detail::interrupt_base<TIM8_TRG_COM_TIM14_IRQn> {};
struct tim8_cc : detail::interrupt_base<TIM8_CC_IRQn> {};
struct adc3 : detail::interrupt_base<ADC3_IRQn> {};
struct fsmc : detail::interrupt_base<FSMC_IRQn> {};
struct sdio : detail::interrupt_base<SDIO_IRQn> {};
struct dma2_ch4_5 : detail::interrupt_base<DMA2_Channel4_5_IRQn> {};
#else //Other devices
struct usb_hp_can1_tx : detail::interrupt_base<USB_HP_CAN1_TX_IRQn> {};
struct usb_lp_can1_rx0 : detail::interrupt_base<USB_LP_CAN1_RX0_IRQn> {};
struct tim1_brk : detail::interrupt_base<TIM1_BRK_IRQn> {};
struct tim1_up : detail::interrupt_base<TIM1_UP_IRQn> {};
struct tim1_trg_com : detail::interrupt_base<TIM1_TRG_COM_IRQn> {};
struct usb_wakeup : detail::interrupt_base<USBWakeUp_IRQn> {};
struct tim8_brk : detail::interrupt_base<TIM8_BRK_IRQn> {};
struct tim8_up : detail::interrupt_base<TIM8_UP_IRQn> {};
struct tim8_trg_com : detail::interrupt_base<TIM8_TRG_COM_IRQn> {};
struct tim8_cc : detail::interrupt_base<TIM8_CC_IRQn> {};
struct adc3 : detail::interrupt_base<ADC3_IRQn> {};
struct fsmc : detail::interrupt_base<FSMC_IRQn> {};
struct sdio : detail::interrupt_base<SDIO_IRQn> {};
struct dma2_ch4_5 : detail::interrupt_base<DMA2_Channel4_5_IRQn> {};
#endif

} //namespace mcutl::interrupt::type

namespace mcutl::device::interrupt
{

[[maybe_unused]] constexpr uint32_t maximum_priorities = 16;
[[maybe_unused]] constexpr bool has_atomic_enable = true;
[[maybe_unused]] constexpr bool has_atomic_disable = true;
[[maybe_unused]] constexpr bool has_atomic_set_priority = true;
[[maybe_unused]] constexpr bool has_atomic_clear_pending = true;
[[maybe_unused]] constexpr bool has_priorities = true;
[[maybe_unused]] constexpr bool has_subpriorities = true;
[[maybe_unused]] constexpr bool has_interrupt_context = true;
[[maybe_unused]] constexpr bool has_get_active = true;

template<uint32_t PriorityCount>
inline constexpr void validate_priority_count() noexcept
{
	static_assert(PriorityCount && math::is_power_of_2<PriorityCount>()
		&& PriorityCount <= (1 << __NVIC_PRIO_BITS),
		"PriorityCount must be power of 2 and have __NVIC_PRIO_BITS bits maximum");
}

template<uint32_t PriorityCount>
inline constexpr uint32_t get_priority_bitcount() noexcept
{
	return math::log2<PriorityCount>();
}

template<mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t SubPriority, uint32_t PriorityCount>
constexpr uint8_t get_priority_value() noexcept
{
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	static_assert(Priority >= 0 && Priority < (1 << priority_bitcount), "Invalid interrupt priority");
	static_assert(SubPriority == mcutl::interrupt::default_priority
		|| SubPriority < (1 << (8u - priority_bitcount)), "Invalid interrupt subpriority");
	
	uint8_t res = (Priority << (8u - priority_bitcount)) & 0xFFu;
	if constexpr (SubPriority != mcutl::interrupt::default_priority)
		res |= SubPriority;
	
	return res;
}

template<uint32_t PriorityCount>
void initialize_controller() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_grouping = 7 - get_priority_bitcount<PriorityCount>();
	mcutl::memory::set_register_bits<SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_PRIGROUP_Msk,
		(0x5FAu << SCB_AIRCR_VECTKEY_Pos) | (priority_grouping << SCB_AIRCR_PRIGROUP_Pos),
		&SCB_Type::AIRCR, SCB_BASE>();
}

template<typename Interrupt>
void enable() noexcept
{
	mcutl::instruction::execute<instruction::type::dmb>();
	__COMPILER_BARRIER();
	mcutl::memory::set_register_array_value<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ISER, (Interrupt::irqn >> 5u), NVIC_BASE>();
	__COMPILER_BARRIER();
}

template<typename Interrupt, mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t SubPriority, uint32_t PriorityCount>
void set_priority() noexcept
{
	validate_priority_count<PriorityCount>();
	
	constexpr uint8_t priority_value = get_priority_value<Priority, SubPriority, PriorityCount>();
	mcutl::memory::set_register_array_value<priority_value,
		&NVIC_Type::IP, Interrupt::irqn, NVIC_BASE>();
	mcutl::instruction::execute<instruction::type::isb>();
}

template<typename Interrupt, uint32_t PriorityCount>
mcutl::interrupt::priority_t get_priority() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	if constexpr (!priority_bitcount)
	{
		return {};
	}
	else
	{
		return static_cast<mcutl::interrupt::priority_t>(
			mcutl::memory::get_register_array_bits<0xffu, &NVIC_Type::IP, Interrupt::irqn, NVIC_BASE>()
				>> (8u - priority_bitcount));
	}
}

template<typename Interrupt, uint32_t PriorityCount>
mcutl::interrupt::priority_t get_subpriority() noexcept
{
	validate_priority_count<PriorityCount>();
	constexpr uint32_t priority_bitcount = get_priority_bitcount<PriorityCount>();
	return static_cast<mcutl::interrupt::priority_t>(
		mcutl::memory::get_register_array_bits<0xffu, &NVIC_Type::IP, Interrupt::irqn, NVIC_BASE>()
			& (0xffu >> priority_bitcount));
}

template<typename Interrupt>
void disable() noexcept
{
	mcutl::memory::set_register_array_value<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ICER, (Interrupt::irqn >> 5u), NVIC_BASE>();
	mcutl::instruction::execute<instruction::type::dsb>();
	mcutl::instruction::execute<instruction::type::isb>();
}

template<typename Interrupt>
void clear_pending() noexcept
{
	mcutl::memory::set_register_array_value<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ICPR, (Interrupt::irqn >> 5u), NVIC_BASE>();
}

template<typename Interrupt>
bool is_pending() noexcept
{
	return mcutl::memory::get_register_array_flag<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ISPR, (Interrupt::irqn >> 5u), NVIC_BASE>();
}

template<typename Interrupt>
bool is_enabled() noexcept
{
	return mcutl::memory::get_register_array_flag<(1u << (Interrupt::irqn & 0x1Fu)),
		&NVIC_Type::ISER, (Interrupt::irqn >> 5u), NVIC_BASE>();
}

template<typename = void>
bool is_interrupt_context() noexcept
{
	return mcutl::memory::get_register_flag<SCB_ICSR_VECTACTIVE_Msk, &SCB_Type::ICSR, SCB_BASE>();
}

template<typename T = void>
mcutl::interrupt::irqn_t get_active() noexcept
{
	auto active = mcutl::memory::get_register_bits<SCB_ICSR_VECTACTIVE_Msk, &SCB_Type::ICSR, SCB_BASE>();
	return active ? active : mcutl::interrupt::unused_irqn;
}

inline void disable_all() noexcept
{
	mcutl::instruction::execute<instruction::type::cpsid_i>();
}

inline void enable_all() noexcept
{
	mcutl::instruction::execute<instruction::type::cpsie_i>();
}

template<typename Interrupt>
inline void enable_atomic() noexcept
{
	enable<Interrupt>();
}

template<typename Interrupt>
inline void disable_atomic() noexcept
{
	disable<Interrupt>();
}

template<typename Interrupt, mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t SubPriority, uint32_t PriorityCount>
inline void set_priority_atomic() noexcept
{
	set_priority<Interrupt, Priority, SubPriority, PriorityCount>();
}

template<typename Interrupt>
inline void clear_pending_atomic() noexcept
{
	clear_pending<Interrupt>();
}

} //namespace mcutl::device::interrupt
