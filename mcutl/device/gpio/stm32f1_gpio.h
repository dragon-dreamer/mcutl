#pragma once

#include <array>
#include <limits>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include "mcutl/device/gpio/stm32_gpio.h"
#include "mcutl/gpio/gpio_defs.h"
#include "mcutl/exti/exti_defs.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::gpio
{

[[maybe_unused]] constexpr bool has_atomic_set_out_value = true;

struct afio_data
{
	std::array<uint16_t, 4> afio_exti_changed_bits {};
	std::array<uint16_t, 4> afio_exti_bit_values {};
	bool disconnected_from_exti = false;
	bool afio_settings_changed = false;
};

struct port_bit_data : afio_data
{
	uint64_t cr_changed_bits = 0;
	uint64_t cr_bit_values = 0;
	uint16_t dr_set_bits = 0;
	uint16_t dr_reset_bits = 0;
	
	bool invalid_output_options = false;
};

struct defines
{
	static constexpr uint64_t conf_and_mode_bit_count = 4ull;
	static constexpr uint64_t conf_bit_count = 2ull;
	static constexpr uint64_t mode_bit_count = 2ull;
	
	static constexpr uint32_t mode_bits_offset = 0;
	static constexpr uint32_t conf_bits_offset = 2;
	
	static constexpr uint32_t open_drain_conf = 0b01;
	static constexpr uint32_t push_pull_conf = 0b00;
	static constexpr uint32_t open_drain_alt_func_conf = 0b11;
	static constexpr uint32_t push_pull_alt_func_conf = 0b10;
	
	static constexpr uint32_t input_mode = 0b00;
	static constexpr uint32_t output_freq_2mhz_mode = 0b10;
	static constexpr uint32_t output_freq_10mhz_mode = 0b01;
	static constexpr uint32_t output_freq_50mhz_mode = 0b11;
	
	static constexpr uint32_t input_floating_conf = 0b01;
	static constexpr uint32_t input_pull_up_conf = 0b10;
	static constexpr uint32_t input_pull_down_conf = 0b10;
	static constexpr uint32_t input_analog_conf = 0b00;
};

template<uint32_t AfioExtiIndex, uint16_t AfioBits, uint16_t AfioMask>
struct afio_config
{
	static constexpr uint16_t afio_exti_index = AfioExtiIndex;
	static constexpr uint16_t afio_bits = AfioBits;
	static constexpr uint16_t afio_mask = AfioMask;
};

template<typename Pin> struct pin_afio_config {};
#ifdef GPIOA_BASE
template<> struct pin_afio_config<mcutl::gpio::gpioa<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PA, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PA, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PA, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PA, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PA, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PA, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PA, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PA, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PA, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PA, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PA, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PA, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PA, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PA, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PA, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioa<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PA, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOA_BASE

#ifdef GPIOB_BASE
template<> struct pin_afio_config<mcutl::gpio::gpiob<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PB, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PB, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PB, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PB, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PB, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PB, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PB, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PB, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PB, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PB, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PB, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PB, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PB, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PB, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PB, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiob<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PB, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOB_BASE

#ifdef GPIOC_BASE
template<> struct pin_afio_config<mcutl::gpio::gpioc<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PC, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PC, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PC, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PC, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PC, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PC, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PC, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PC, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PC, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PC, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PC, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PC, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PC, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PC, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PC, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioc<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PC, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOC_BASE

#ifdef GPIOD_BASE
template<> struct pin_afio_config<mcutl::gpio::gpiod<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PD, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PD, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PD, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PD, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PD, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PD, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PD, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PD, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PD, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PD, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PD, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PD, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PD, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PD, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PD, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiod<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PD, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOD_BASE

#ifdef GPIOE_BASE
template<> struct pin_afio_config<mcutl::gpio::gpioe<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PE, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PE, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PE, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PE, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PE, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PE, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PE, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PE, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PE, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PE, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PE, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PE, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PE, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PE, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PE, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpioe<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PE, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOE_BASE

#ifdef GPIOF_BASE
template<> struct pin_afio_config<mcutl::gpio::gpiof<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PF, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PF, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PF, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PF, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PF, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PF, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PF, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PF, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PF, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PF, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PF, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PF, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PF, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PF, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PF, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiof<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PF, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOF_BASE

#ifdef GPIOG_BASE
template<> struct pin_afio_config<mcutl::gpio::gpiog<0>>
	: afio_config<0, AFIO_EXTICR1_EXTI0_PG, AFIO_EXTICR1_EXTI0_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<1>>
	: afio_config<0, AFIO_EXTICR1_EXTI1_PG, AFIO_EXTICR1_EXTI1_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<2>>
	: afio_config<0, AFIO_EXTICR1_EXTI2_PG, AFIO_EXTICR1_EXTI2_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<3>>
	: afio_config<0, AFIO_EXTICR1_EXTI3_PG, AFIO_EXTICR1_EXTI3_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<4>>
	: afio_config<1, AFIO_EXTICR2_EXTI4_PG, AFIO_EXTICR2_EXTI4_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<5>>
	: afio_config<1, AFIO_EXTICR2_EXTI5_PG, AFIO_EXTICR2_EXTI5_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<6>>
	: afio_config<1, AFIO_EXTICR2_EXTI6_PG, AFIO_EXTICR2_EXTI6_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<7>>
	: afio_config<1, AFIO_EXTICR2_EXTI7_PG, AFIO_EXTICR2_EXTI7_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<8>>
	: afio_config<2, AFIO_EXTICR3_EXTI8_PG, AFIO_EXTICR3_EXTI8_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<9>>
	: afio_config<2, AFIO_EXTICR3_EXTI9_PG, AFIO_EXTICR3_EXTI9_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<10>>
	: afio_config<2, AFIO_EXTICR3_EXTI10_PG, AFIO_EXTICR3_EXTI10_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<11>>
	: afio_config<2, AFIO_EXTICR3_EXTI11_PG, AFIO_EXTICR3_EXTI11_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<12>>
	: afio_config<3, AFIO_EXTICR4_EXTI12_PG, AFIO_EXTICR4_EXTI12_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<13>>
	: afio_config<3, AFIO_EXTICR4_EXTI13_PG, AFIO_EXTICR4_EXTI13_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<14>>
	: afio_config<3, AFIO_EXTICR4_EXTI14_PG, AFIO_EXTICR4_EXTI14_Msk> {};
template<> struct pin_afio_config<mcutl::gpio::gpiog<15>>
	: afio_config<3, AFIO_EXTICR4_EXTI15_PG, AFIO_EXTICR4_EXTI15_Msk> {};
#endif //GPIOG_BASE

template<typename... PinConfig>
struct config_helper {};

enum class config_type
{
	value,
	input,
	output
};

template<typename PinConfig>
struct pin_helper
{
	static constexpr void apply(port_bit_data&, char) noexcept
	{
		static_assert(types::always_false<PinConfig>::value,
			"Unsupported pin configuration type");
	}
};

template<typename... Options>
struct output_options_helper
{
	static constexpr std::pair<bool, uint64_t> apply() noexcept
	{
		return { true, defines::output_freq_50mhz_mode << defines::mode_bits_offset };
	}
};

template<typename Option, typename... Options>
struct output_options_helper<Option, Options...>
{
	static constexpr std::pair<bool, uint64_t> apply() noexcept
	{
		constexpr bool valid = sizeof...(Options) == 0;
		if constexpr (std::is_same_v<Option, mcutl::gpio::out::opt::freq_2mhz>)
			return { valid, defines::output_freq_2mhz_mode << defines::mode_bits_offset };
		else if constexpr (std::is_same_v<Option, mcutl::gpio::out::opt::freq_10mhz>)
			return { valid, defines::output_freq_10mhz_mode << defines::mode_bits_offset };
		else if constexpr (std::is_same_v<Option, mcutl::gpio::out::opt::freq_50mhz>)
			return { valid, defines::output_freq_50mhz_mode << defines::mode_bits_offset };
		else
			return { false, 0 };
	}
};

template<typename Pin>
constexpr void apply_cr_bitmask(port_bit_data& data) noexcept
{
	uint64_t conf_mode_bit_mask = (1ull << defines::conf_and_mode_bit_count) - 1ull;
	uint64_t cr_changed_bits = conf_mode_bit_mask << (Pin::pin_number * defines::conf_and_mode_bit_count);
	data.cr_changed_bits |= cr_changed_bits;
}

template<typename Pin, typename OutputMode,
	typename OutValue, typename... OutputOptions>
struct pin_helper<mcutl::gpio::as_output<Pin, OutputMode, OutValue, OutputOptions...>>
{
	static constexpr config_type type = config_type::output;
	
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		apply_cr_bitmask<Pin>(data);
		
		uint64_t cr_bit_values = 0;
		if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::open_drain>)
			cr_bit_values = defines::open_drain_conf << defines::conf_bits_offset;
		else if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::open_drain_alt_func>)
			cr_bit_values = defines::open_drain_alt_func_conf << defines::conf_bits_offset;
		else if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::push_pull_alt_func>)
			cr_bit_values = defines::push_pull_alt_func_conf << defines::conf_bits_offset;
		else if constexpr (std::is_same_v<OutputMode, mcutl::gpio::out::push_pull>)
			cr_bit_values = defines::push_pull_conf << defines::conf_bits_offset;
		else
			unknown_output_mode<OutputMode>();
		
		constexpr auto options_parse_result = output_options_helper<OutputOptions...>::apply();
		static_assert(options_parse_result.first, "Unsupported or conflicting options for gpio::as_output");
		cr_bit_values |= options_parse_result.second;
		
		cr_bit_values <<= Pin::pin_number * defines::conf_and_mode_bit_count;
		data.cr_bit_values |= cr_bit_values;
		
		pin_helper<mcutl::gpio::to_value<Pin, OutValue>>::apply(data, port_letter);
	}
	
	template<typename T>
	static constexpr void unknown_output_mode() noexcept
	{
		static_assert(types::always_false<T>::value, "Unknown pin output mode");
	}
};

template<typename Pin, typename OutValue, typename... OutputOptions>
struct pin_helper<mcutl::gpio::to_value<Pin, OutValue, OutputOptions...>>
{
	static constexpr config_type type = config_type::value;
	
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		uint16_t dr_set_bits = 0, dr_reset_bits = 0;
		if constexpr (std::is_same_v<OutValue, mcutl::gpio::out::one>)
			dr_set_bits = 1u << Pin::pin_number;
		else if constexpr (std::is_same_v<OutValue, mcutl::gpio::out::zero>)
			dr_reset_bits = 1u << Pin::pin_number;
		
		if constexpr (sizeof...(OutputOptions) > 0)
		{
			static_assert(sizeof...(OutputOptions) == 1
					&& std::is_same_v<types::first_type_t<OutputOptions...>, mcutl::gpio::out::opt::atomic>,
				"Unsupported or conflicting options for gpio::to_value");
		}
		
		data.dr_set_bits |= dr_set_bits;
		data.dr_reset_bits |= dr_reset_bits;
	}
};

template<typename Pin, typename InputMode, typename... InputOptions>
struct pin_helper<mcutl::gpio::as_input<Pin, InputMode, InputOptions...>>
{
	static constexpr config_type type = config_type::input;
	
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		static_assert(sizeof...(InputOptions) == 0,
			"No additional gpio::as_input options are supported");
		
		apply_cr_bitmask<Pin>(data);
		
		uint64_t cr_bit_values = 0;
		if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::floating>)
		{
			cr_bit_values = defines::input_floating_conf << defines::conf_bits_offset;
		}
		else if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::pull_up>)
		{
			cr_bit_values = defines::input_pull_up_conf << defines::conf_bits_offset;
			data.dr_set_bits |= 1u << Pin::pin_number;
		}
		else if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::pull_down>)
		{
			cr_bit_values = defines::input_pull_down_conf << defines::conf_bits_offset;
			data.dr_reset_bits |= 1u << Pin::pin_number;
		}
		else if constexpr (std::is_same_v<InputMode, mcutl::gpio::in::analog>)
		{
			cr_bit_values = defines::input_analog_conf << defines::conf_bits_offset;
		}
		else
		{
			unknown_input_mode<InputMode>();
		}
		
		cr_bit_values <<= Pin::pin_number * defines::conf_and_mode_bit_count;
		data.cr_bit_values |= cr_bit_values;
	}
	
	template<typename T>
	static constexpr void unknown_input_mode() noexcept
	{
		static_assert(types::always_false<T>::value, "Unknown pin input mode");
	}
};

template<typename... PinConfig>
struct config_helper<mcutl::gpio::config<PinConfig...>>
{
	static void configure() MCUTL_NOEXCEPT
	{
		config_helper<mcutl::gpio::config<PinConfig...>,
			available_regs_t>::configure();
	}
	
	template<char PortLetter>
	static constexpr port_bit_data get_port_bit_data() noexcept
	{
		return config_helper<mcutl::gpio::config<PinConfig...>,
			available_regs_t>::template get_port_bit_data<PortLetter>();
	}
};

template<typename... PinConfig, typename... ValidPorts>
struct config_helper<mcutl::gpio::config<PinConfig...>,
	valid_gpio_list<ValidPorts...>>
{
	template<char PortLetter>
	static constexpr port_bit_data get_port_bit_data() noexcept
	{
		port_bit_data data;
		(..., pin_helper<PinConfig>::apply(data, PortLetter));
		return data;
	}
	
	template<char PortLetter>
	static void configure() MCUTL_NOEXCEPT
	{
		constexpr auto data = get_port_bit_data<PortLetter>();
		constexpr auto port_base = get_port_base<PortLetter>();
		
		mcutl::memory::set_register_bits<
			static_cast<uint32_t>(data.cr_changed_bits & (std::numeric_limits<uint32_t>::max)()),
			static_cast<uint32_t>(data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()),
			&GPIO_TypeDef::CRL, port_base>();
		mcutl::memory::set_register_bits<
			static_cast<uint32_t>((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()),
			static_cast<uint32_t>((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()),
			&GPIO_TypeDef::CRH, port_base>();
		
		if constexpr (data.dr_set_bits || data.dr_reset_bits)
		{
			mcutl::memory::set_register_value<
				(static_cast<uint32_t>(data.dr_set_bits) << GPIO_BSRR_BS0_Pos)
					| static_cast<uint32_t>(data.dr_reset_bits) << GPIO_BSRR_BR0_Pos,
				&GPIO_TypeDef::BSRR, port_base>();
		}
	}
	
	template<char PortLetter>
	static constexpr void get_port_bit_data(port_bit_data& data) noexcept
	{
		(..., pin_helper<PinConfig>::apply(data, PortLetter));
	}
	
	static constexpr afio_data get_afio_data() noexcept
	{
		port_bit_data data {};
		(..., get_port_bit_data<ValidPorts::port_letter>(data));
		return data;
	}
	
	static void configure_afio() MCUTL_NOEXCEPT
	{
		constexpr auto data = get_afio_data();
		mcutl::memory::set_register_array_bits<data.afio_exti_changed_bits[0],
			data.afio_exti_bit_values[0], &AFIO_TypeDef::EXTICR, 0, AFIO_BASE>();
		mcutl::memory::set_register_array_bits<data.afio_exti_changed_bits[1],
			data.afio_exti_bit_values[1], &AFIO_TypeDef::EXTICR, 1, AFIO_BASE>();
		mcutl::memory::set_register_array_bits<data.afio_exti_changed_bits[2],
			data.afio_exti_bit_values[2], &AFIO_TypeDef::EXTICR, 2, AFIO_BASE>();
		mcutl::memory::set_register_array_bits<data.afio_exti_changed_bits[3],
			data.afio_exti_bit_values[3], &AFIO_TypeDef::EXTICR, 3, AFIO_BASE>();
	}
	
	static void configure() MCUTL_NOEXCEPT
	{
		configure_afio();
		(..., configure<ValidPorts::port_letter>());
	}
};

template<typename PinConfig>
struct gpio_peripheral_control {};

template<typename... PinConfig>
struct gpio_peripheral_control<mcutl::gpio::config<PinConfig...>>
{
	static void enable() MCUTL_NOEXCEPT
	{
		if constexpr ((... || std::is_same_v<
			typename PinConfig::tag, mcutl::gpio::detail::exti_tag>))
		{
			mcutl::periph::configure_peripheral<
				mcutl::periph::enable<mcutl::periph::afio>,
				mcutl::periph::enable<mcutl::gpio::to_periph<typename PinConfig::pin>>...>();
		}
		else
		{
			mcutl::periph::configure_peripheral<mcutl::periph::enable<
				mcutl::gpio::to_periph<typename PinConfig::pin>>...>();
		}
	}
};

template<bool EnablePeripheralsRequested, typename PinConfig>
void configure_gpio() MCUTL_NOEXCEPT
{
	if constexpr (EnablePeripheralsRequested)
		gpio_peripheral_control<PinConfig>::enable();
	
	config_helper<PinConfig>::configure();
}

template<bool NegateBits, typename... Pins>
auto get_input_values_mask() MCUTL_NOEXCEPT
{
	constexpr auto port_letter = pin_port_letter_helper<Pins...>::get_first_pin_port_letter();
	constexpr auto port_base = get_port_base<port_letter>();
	constexpr auto pin_bit_mask = get_pin_bit_mask<Pins...>();
	if constexpr (NegateBits)
		return ~mcutl::memory::get_register_bits<&GPIO_TypeDef::IDR, port_base>() & pin_bit_mask;
	else
		return mcutl::memory::get_register_bits<pin_bit_mask, &GPIO_TypeDef::IDR, port_base>();
}

template<bool NegateBits, typename... Pins>
auto get_output_values_mask() MCUTL_NOEXCEPT
{
	constexpr auto port_letter = pin_port_letter_helper<Pins...>::get_first_pin_port_letter();
	constexpr auto port_base = get_port_base<port_letter>();
	constexpr auto pin_bit_mask = get_pin_bit_mask<Pins...>();
	if constexpr (NegateBits)
		return ~mcutl::memory::get_register_bits<&GPIO_TypeDef::ODR, port_base>() & pin_bit_mask;
	else
		return mcutl::memory::get_register_bits<pin_bit_mask, &GPIO_TypeDef::ODR, port_base>();
}

template<typename Pin>
bool is_output() MCUTL_NOEXCEPT
{
	constexpr auto port_letter = Pin::port_letter;
	constexpr auto port_base = get_port_base<port_letter>();
	if constexpr (Pin::pin_number < 8)
	{
		constexpr uint32_t pin_mode_mask = (GPIO_CRL_MODE0 << (Pin::pin_number * defines::conf_and_mode_bit_count));
		return mcutl::memory::get_register_bits<pin_mode_mask, &GPIO_TypeDef::CRL, port_base>() != defines::input_mode;
	}
	else
	{
		constexpr uint32_t pin_mode_mask = (GPIO_CRH_MODE8 << ((Pin::pin_number - 8) * defines::conf_and_mode_bit_count));
		return mcutl::memory::get_register_bits<pin_mode_mask, &GPIO_TypeDef::CRH, port_base>() != defines::input_mode;
	}
}

template<uint16_t AfioExtiChangedBits, uint16_t AfioExtiBitValues, size_t Index>
inline bool check_afio_connected() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_array_bits<AfioExtiChangedBits,
		&AFIO_TypeDef::EXTICR, Index, AFIO_BASE>() == AfioExtiBitValues;
}

template<typename PinConfig>
bool is() MCUTL_NOEXCEPT
{
	constexpr auto data = config_helper<mcutl::gpio::config<PinConfig>, available_regs_t>
		::template get_port_bit_data<PinConfig::pin::port_letter>();
	
	[[maybe_unused]] constexpr auto port_base = get_port_base<PinConfig::pin::port_letter>();
	if constexpr ((data.cr_changed_bits & (std::numeric_limits<uint32_t>::max)()) != 0)
	{
		if (mcutl::memory::get_register_bits<data.cr_changed_bits, &GPIO_TypeDef::CRL, port_base>()
			!= (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
	}
	else if constexpr (((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()) != 0)
	{
		if (mcutl::memory::get_register_bits<
			static_cast<uint32_t>((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()),
			&GPIO_TypeDef::CRH, port_base>()
				!= ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
	}
	
	if constexpr (data.dr_set_bits != 0 || data.dr_reset_bits != 0)
	{
		constexpr auto pin_type = pin_helper<PinConfig>::type;
		if constexpr (pin_type == config_type::input)
		{
			if (mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
				&GPIO_TypeDef::IDR, port_base>() != data.dr_set_bits)
			{
				return false;
			}
		}
		else if constexpr (pin_type == config_type::output)
		{
			if (mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
				&GPIO_TypeDef::ODR, port_base>() != data.dr_set_bits)
			{
				return false;
			}
		}
		else
		{
			auto dr = is_output<typename PinConfig::pin>() ? &GPIO_TypeDef::ODR : &GPIO_TypeDef::IDR;
			if (mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
				port_base>(dr) != data.dr_set_bits)
			{
				return false;
			}
		}
	}
	
	using afio_config_t = device::gpio::pin_afio_config<typename PinConfig::pin>;
	if constexpr (data.disconnected_from_exti)
	{
		if (check_afio_connected<afio_config_t::afio_mask,
			afio_config_t::afio_bits, afio_config_t::afio_exti_index>())
		{
			return false;
		}
	}
	else if constexpr (data.afio_settings_changed)
	{
		if (!check_afio_connected<afio_config_t::afio_mask,
			afio_config_t::afio_bits, afio_config_t::afio_exti_index>())
		{
			return false;
		}
	}
	
	return true;
}

struct port_bit_data_ex : port_bit_data
{
	uint32_t pin_type_set_count = 0;
	uint32_t pin_mode_set_count = 0;
	uint32_t dr_set_count = 0;
	uint32_t exti_line = 0;
	uint32_t connected_exti_line_set_count = 0;
	uint32_t disconnected_exti_line_set_count = 0;
	bool input_category = false;
};

template<typename... Options>
struct exti_helper
{
	static_assert(sizeof...(Options) == 0, "No additional EXTI options are supported");
};

template<typename Pin, typename Option>
struct single_option_helper
{
	static constexpr void apply(port_bit_data_ex&) noexcept
	{
		static_assert(types::always_false<Option>::value, "Unsupported gpio::has() option");
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::keep_value>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		++data.dr_set_count;
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::one>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		++data.dr_set_count;
		data.dr_set_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::zero>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		++data.dr_set_count;
		data.dr_reset_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
constexpr void apply_pin_type_option(port_bit_data_ex& data, uint64_t conf) noexcept
{
	apply_cr_bitmask<Pin>(data);
	++data.pin_type_set_count;
	data.cr_bit_values |= conf << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::open_drain>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::open_drain_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::open_drain_alt_func>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::open_drain_alt_func_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::push_pull>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::push_pull_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::push_pull_alt_func>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::push_pull_alt_func_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::analog>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_analog_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::floating>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_floating_conf << defines::conf_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::pull_down>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_pull_down_conf << defines::conf_bits_offset);
		++data.dr_set_count;
		data.dr_reset_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::in::pull_up>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_type_option<Pin>(data, defines::input_pull_up_conf << defines::conf_bits_offset);
		++data.dr_set_count;
		data.dr_set_bits |= (1u << Pin::pin_number);
	}
};

template<typename Pin>
constexpr void apply_pin_frequency_option(port_bit_data_ex& data, uint64_t mode) noexcept
{
	apply_cr_bitmask<Pin>(data);
	++data.pin_mode_set_count;
	data.cr_bit_values |= mode << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::opt::freq_2mhz>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_frequency_option<Pin>(data, defines::output_freq_2mhz_mode << defines::mode_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::opt::freq_10mhz>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_frequency_option<Pin>(data, defines::output_freq_10mhz_mode << defines::mode_bits_offset);
	}
};

template<typename Pin>
struct single_option_helper<Pin, mcutl::gpio::out::opt::freq_50mhz>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		apply_pin_frequency_option<Pin>(data, defines::output_freq_50mhz_mode << defines::mode_bits_offset);
	}
};

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct single_option_helper<Pin, mcutl::gpio::connect_to_exti_line<Pin, ExtiLine, Options...>>
	: exti_helper<Options...>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		data.exti_line = ExtiLine;
		++data.connected_exti_line_set_count;
	}
};

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct single_option_helper<Pin, mcutl::gpio::disconnect_from_exti_line<Pin, ExtiLine, Options...>>
	: exti_helper<Options...>
{
	static constexpr void apply(port_bit_data_ex& data) noexcept
	{
		data.exti_line = ExtiLine;
		++data.disconnected_exti_line_set_count;
	}
};

template<typename Pin, typename Option, typename... Options>
constexpr port_bit_data_ex get_options_description() noexcept
{
	port_bit_data_ex data {};
	using options_category = mcutl::gpio::detail::option_category_t<Option>;
	single_option_helper<Pin, Option>::apply(data);
	(..., single_option_helper<Pin, Options>::apply(data));
	if constexpr (std::is_same_v<options_category, mcutl::gpio::detail::in_option>)
	{
		data.input_category = true;
		apply_pin_frequency_option<Pin>(data,
			defines::input_mode << defines::mode_bits_offset);
	}
	
	return data;
}

template<typename Pin>
constexpr uint64_t get_mode_bitmask() noexcept
{
	uint64_t mode_bit_mask = ((1ull << defines::mode_bit_count) - 1ull) << defines::mode_bits_offset;
	return mode_bit_mask << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin>
constexpr uint64_t get_conf_bitmask() noexcept
{
	uint64_t conf_bit_mask = ((1ull << defines::conf_bit_count) - 1ull) << defines::conf_bits_offset;
	return conf_bit_mask << (Pin::pin_number * defines::conf_and_mode_bit_count);
}

template<typename Pin, typename Option, typename... Options>
bool has() MCUTL_NOEXCEPT
{
	constexpr port_bit_data_ex data = get_options_description<Pin, Option, Options...>();
	[[maybe_unused]] constexpr auto port_base = get_port_base<Pin::port_letter>();
	
	static_assert(data.pin_mode_set_count <= 1, "Pin frequency options conflict");
	static_assert(data.dr_set_count <= 1, "Pin value options conflict");
	static_assert(data.pin_type_set_count <= 1, "Pin type options conflict");
	static_assert(data.connected_exti_line_set_count <= 1
		&& data.disconnected_exti_line_set_count <= 1, "Multiple EXTI lines are not supported");
	static_assert(!data.connected_exti_line_set_count
		|| !data.disconnected_exti_line_set_count, "Unable to check both connected and disconnected EXTI lines");
	
	if constexpr (data.connected_exti_line_set_count
		&& data.exti_line != mcutl::gpio::detail::exti_map<Pin>::get_default_line()
		&& data.exti_line != mcutl::gpio::default_exti_line)
	{
		return false;
	}
	else if constexpr (data.connected_exti_line_set_count)
	{
		using afio_config_t = device::gpio::pin_afio_config<Pin>;
		if (!check_afio_connected<afio_config_t::afio_mask,
			afio_config_t::afio_bits, afio_config_t::afio_exti_index>())
		{
			return false;
		}
	}
	else if constexpr (data.disconnected_exti_line_set_count
		&& (data.exti_line == mcutl::gpio::detail::exti_map<Pin>::get_default_line()
			|| data.exti_line == mcutl::gpio::default_exti_line))
	{
		using afio_config_t = device::gpio::pin_afio_config<Pin>;
		if (check_afio_connected<afio_config_t::afio_mask,
			afio_config_t::afio_bits, afio_config_t::afio_exti_index>())
		{
			return false;
		}
	}
	else if constexpr (!data.cr_changed_bits && (data.dr_set_bits || data.dr_reset_bits))
	{
		return is_output<Pin>() && mcutl::memory::get_register_bits<data.dr_set_bits | data.dr_reset_bits,
			&GPIO_TypeDef::ODR, port_base>() == data.dr_set_bits;
	}
	else if constexpr (data.cr_changed_bits != 0)
	{
		if constexpr ((data.cr_changed_bits & (std::numeric_limits<uint32_t>::max)()) != 0)
		{
			auto cr = mcutl::memory::get_register_bits<data.cr_changed_bits,
				&GPIO_TypeDef::CRL, port_base>();
			if constexpr (data.pin_mode_set_count && data.pin_type_set_count)
			{
				if (cr != (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()))
					return false;
			}
			else if constexpr (data.pin_type_set_count)
			{
				//Out mode, no frequency option supplied
				if ((cr & ~static_cast<uint32_t>(get_mode_bitmask<Pin>()))
						!= (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)())
					|| !(cr & static_cast<uint32_t>(get_mode_bitmask<Pin>())))
				{
					return false;
				}
			}
			else
			{
				if ((cr & ~static_cast<uint32_t>(get_conf_bitmask<Pin>()))
						!= (data.cr_bit_values & (std::numeric_limits<uint32_t>::max)()))
				{
					return false;
				}
			}
		}
		else
		{
			auto cr = mcutl::memory::get_register_bits<
				static_cast<uint32_t>((data.cr_changed_bits >> 32u) & (std::numeric_limits<uint32_t>::max)()),
				&GPIO_TypeDef::CRH, port_base>();
			if constexpr (data.pin_mode_set_count && data.pin_type_set_count)
			{
				if (cr != ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()))
					return false;
			}
			else if constexpr (data.pin_type_set_count)
			{
				//Out mode, no frequency option supplied
				if ((cr & ~static_cast<uint32_t>(get_mode_bitmask<Pin>() >> 32u))
						!= ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)())
					|| !(cr & static_cast<uint32_t>(get_mode_bitmask<Pin>() >> 32u)))
				{
					return false;
				}
			}
			else
			{
				if ((cr & ~static_cast<uint32_t>(get_conf_bitmask<Pin>() >> 32u))
						!= ((data.cr_bit_values >> 32u) & (std::numeric_limits<uint32_t>::max)()))
				{
					return false;
				}
			}
		}
		
		if constexpr (data.dr_set_bits || data.dr_reset_bits)
		{
			constexpr auto dr = data.input_category ? &GPIO_TypeDef::IDR : &GPIO_TypeDef::ODR;
			if (mcutl::memory::get_register_bits<
				data.dr_set_bits | data.dr_reset_bits, dr, port_base>() != data.dr_set_bits)
			{
				return false;
			}
		}
	}
	else if constexpr (data.dr_set_count != 0)
	{
		return is_output<Pin>();
	}
	
	return true;
}

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct pin_helper<mcutl::gpio::connect_to_exti_line<Pin, ExtiLine, Options...>>
	: exti_helper<Options...>
{
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		using config_t = pin_afio_config<Pin>;
		data.afio_exti_changed_bits[config_t::afio_exti_index] |= config_t::afio_mask;
		data.afio_exti_bit_values[config_t::afio_exti_index] |= config_t::afio_bits;
		data.afio_settings_changed = true;
	}
};

template<typename Pin, uint32_t ExtiLine, typename... Options>
struct pin_helper<mcutl::gpio::disconnect_from_exti_line<Pin, ExtiLine, Options...>>
	: exti_helper<Options...>
{
	static constexpr void apply(port_bit_data& data, char port_letter) noexcept
	{
		if (port_letter != Pin::port_letter)
			return;
		
		data.disconnected_from_exti = true;
		data.afio_settings_changed = true;
	}
};

} //namespace mcutl::device::gpio

namespace mcutl::gpio::detail
{

template<char PortLetter, uint32_t PinNumber>
struct exti_map<pin<PortLetter, PinNumber>> : exti_map_helper<PinNumber> {};

} //namespace mcutl::gpio::detail
