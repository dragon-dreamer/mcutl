#define STM32F103xE
#define STM32F1

#include <type_traits>

#include "mcutl/exti/exti.h"
#include "mcutl/gpio/gpio.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using gpio_strict_test_fixture = mcutl::tests::memory::strict_test_fixture_base;

TEST_F(gpio_strict_test_fixture, ComplexGpioConfigTest)
{
	::testing::InSequence s;
	
	constexpr uint32_t initial_cr_value = 0xffffffffu;
	constexpr uint32_t initial_exticr3_value = 0xffffffffu;
	memory().set(addr(&GPIOA->CRL), initial_cr_value);
	memory().set(addr(&GPIOA->CRH), initial_cr_value);
	memory().set(addr(&GPIOB->CRL), initial_cr_value);
	memory().set(addr(&(AFIO->EXTICR[2])), initial_exticr3_value);
	
	constexpr uint32_t exti3_mask = AFIO_EXTICR3_EXTI8 | AFIO_EXTICR3_EXTI11;
	constexpr uint32_t exti3_value = AFIO_EXTICR3_EXTI8_PA | AFIO_EXTICR3_EXTI11_PB;
	EXPECT_CALL(memory(), read(addr(&(AFIO->EXTICR[2]))));
	EXPECT_CALL(memory(), write(addr(&(AFIO->EXTICR[2])),
		(initial_exticr3_value & ~exti3_mask) | exti3_value));
	
	EXPECT_CALL(memory(), read(addr(&GPIOA->CRL)));
	
	constexpr uint32_t gpioa_crl_value
		= GPIO_CRL_CNF0_1 //0 - in, pull up
		| GPIO_CRL_CNF3_0 | GPIO_CRL_MODE3_0 | GPIO_CRL_MODE3_1 //3 - out, open drain, 50MHz
		| GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1 //5 - out, push-pull, 50 mhz
		| 0; //7 - in, analog
	constexpr uint32_t gpioa_crl_mask
		= GPIO_CRL_CNF0 | GPIO_CRL_MODE0
		| GPIO_CRL_CNF3 | GPIO_CRL_MODE3
		| GPIO_CRL_CNF5 | GPIO_CRL_MODE5
		| GPIO_CRL_CNF7 | GPIO_CRL_MODE7;
	
	EXPECT_CALL(memory(), write(addr(&GPIOA->CRL),
		(initial_cr_value & ~gpioa_crl_mask) | gpioa_crl_value));
	
	EXPECT_CALL(memory(), read(addr(&GPIOA->CRH)));
	
	constexpr uint32_t gpioa_crh_value
		= GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8_0 //8 - out, push-pull alt, 10MHz
		| GPIO_CRH_CNF11_0 | GPIO_CRH_CNF11_1 | GPIO_CRH_MODE11_1 //11 - out, open drain alt, 2MHz
		| GPIO_CRH_CNF14_1 //14 - in, pull-down
		| GPIO_CRH_CNF15_0; //15 - in, floating
	constexpr uint32_t gpioa_crh_mask
		= GPIO_CRH_CNF8 | GPIO_CRH_MODE8
		| GPIO_CRH_CNF11 | GPIO_CRH_MODE11
		| GPIO_CRH_CNF14 | GPIO_CRH_MODE14
		| GPIO_CRH_CNF15 | GPIO_CRH_MODE15;
	
	EXPECT_CALL(memory(), write(addr(&GPIOA->CRH),
		(initial_cr_value & ~gpioa_crh_mask) | gpioa_crh_value));
	
	EXPECT_CALL(memory(), write(addr(&GPIOA->BSRR), GPIO_BSRR_BR8 | GPIO_BSRR_BR14
		| GPIO_BSRR_BS0 | GPIO_BSRR_BS5));
	
	EXPECT_CALL(memory(), write(addr(&GPIOB->BSRR), GPIO_BSRR_BS1));
	
	mcutl::gpio::configure_gpio<
		mcutl::gpio::to_value<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>,
		mcutl::gpio::as_input<mcutl::gpio::gpioa<0>, mcutl::gpio::in::pull_up>,
		mcutl::gpio::as_output<mcutl::gpio::gpioa<3>, mcutl::gpio::out::open_drain>,
		mcutl::gpio::as_output<mcutl::gpio::gpioa<5>, mcutl::gpio::out::push_pull, mcutl::gpio::out::one>,
		mcutl::gpio::as_output<mcutl::gpio::gpioa<8>, mcutl::gpio::out::push_pull_alt_func,
			mcutl::gpio::out::zero, mcutl::gpio::out::opt::freq_10mhz>,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpioa<8>>,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiob<11>>,
		mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpioa<3>>,
		mcutl::gpio::as_output<mcutl::gpio::gpioa<11>, mcutl::gpio::out::open_drain_alt_func,
			mcutl::gpio::out::keep_value, mcutl::gpio::out::opt::freq_2mhz>,
		mcutl::gpio::as_input<mcutl::gpio::gpioa<7>, mcutl::gpio::in::analog>,
		mcutl::gpio::as_input<mcutl::gpio::gpioa<14>, mcutl::gpio::in::pull_down>,
		mcutl::gpio::as_input<mcutl::gpio::gpioa<15>, mcutl::gpio::in::floating>
	>();
}

TEST_F(gpio_strict_test_fixture, EnableGpioPeripheralsTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), read(addr(&RCC->APB2ENR)));
	EXPECT_CALL(memory(), write(addr(&RCC->APB2ENR),
		RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPFEN));
	EXPECT_CALL(memory(), read(addr(&RCC->APB2ENR)));
	
	EXPECT_CALL(memory(), write(addr(&GPIOB->BSRR), GPIO_BSRR_BS1));
	EXPECT_CALL(memory(), write(addr(&GPIOC->BSRR), GPIO_BSRR_BR11));
	EXPECT_CALL(memory(), write(addr(&GPIOF->BSRR), GPIO_BSRR_BS5));
	
	mcutl::gpio::configure_gpio<
		mcutl::gpio::to_value<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>,
		mcutl::gpio::to_value<mcutl::gpio::gpioc<11>, mcutl::gpio::out::zero>,
		mcutl::gpio::to_value<mcutl::gpio::gpiof<5>, mcutl::gpio::out::one>,
		mcutl::gpio::enable_peripherals
	>();
}

TEST_F(gpio_strict_test_fixture, EnableGpioAfioPeripheralsTest)
{
	::testing::InSequence s;
	
	EXPECT_CALL(memory(), read(addr(&RCC->APB2ENR)));
	EXPECT_CALL(memory(), write(addr(&RCC->APB2ENR),
		RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPFEN | RCC_APB2ENR_AFIOEN));
	EXPECT_CALL(memory(), read(addr(&RCC->APB2ENR)));
	
	EXPECT_CALL(memory(), write(addr(&GPIOB->BSRR), GPIO_BSRR_BS1));
	EXPECT_CALL(memory(), write(addr(&GPIOC->BSRR), GPIO_BSRR_BR11));
	
	mcutl::gpio::configure_gpio<
		mcutl::gpio::to_value<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>,
		mcutl::gpio::to_value<mcutl::gpio::gpioc<11>, mcutl::gpio::out::zero>,
		mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpiof<5>>,
		mcutl::gpio::enable_peripherals
	>();
}

TEST_F(gpio_strict_test_fixture, SetOutValueTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOD->BSRR), GPIO_BSRR_BS12));
	mcutl::gpio::set_out_value<mcutl::gpio::gpiod<12>, mcutl::gpio::out::one>();
}

TEST_F(gpio_strict_test_fixture, ZeroOutValueTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOD->BSRR), GPIO_BSRR_BR0));
	mcutl::gpio::set_out_value<mcutl::gpio::gpiod<0>,
		mcutl::gpio::out::negate_t<mcutl::gpio::out::one>>();
}

TEST_F(gpio_strict_test_fixture, SetOutValueAtomicTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOE->BSRR), GPIO_BSRR_BS13));
	mcutl::gpio::set_out_value<mcutl::gpio::gpioe<13>, mcutl::gpio::out::one>();
}

TEST_F(gpio_strict_test_fixture, ZeroOutValueAtomicTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOG->BSRR), GPIO_BSRR_BR14));
	mcutl::gpio::set_out_value<mcutl::gpio::gpiog<14>,
		mcutl::gpio::out::negate_t<mcutl::gpio::out::one>>();
}

TEST_F(gpio_strict_test_fixture, SetOneTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOD->BSRR), GPIO_BSRR_BS10));
	mcutl::gpio::set_one<mcutl::gpio::gpiod<10>>();
}

TEST_F(gpio_strict_test_fixture, SetZeroTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOD->BSRR), GPIO_BSRR_BR15));
	mcutl::gpio::set_zero<mcutl::gpio::gpiod<15>>();
}

TEST_F(gpio_strict_test_fixture, SetOneAtomicTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOE->BSRR), GPIO_BSRR_BS9));
	mcutl::gpio::set_one_atomic<mcutl::gpio::gpioe<9>>();
}

TEST_F(gpio_strict_test_fixture, SetZeroAtomicTest)
{
	EXPECT_CALL(memory(), write(addr(&GPIOG->BSRR), GPIO_BSRR_BR8));
	mcutl::gpio::set_zero_atomic<mcutl::gpio::gpiog<8>>();
}

TEST_F(gpio_strict_test_fixture, GetInputMaskTest)
{
	memory().set(addr(&GPIOA->IDR),
		mcutl::gpio::pin_bit_mask_v<
			mcutl::gpio::gpioa<1>,
			mcutl::gpio::gpioa<3>,
			mcutl::gpio::gpioa<5>,
			mcutl::gpio::gpioa<6>,
			mcutl::gpio::gpioa<14>
		>
	);
	
	EXPECT_CALL(memory(), read(addr(&GPIOA->IDR)));
	
	auto mask = mcutl::gpio::get_input_values_mask<false,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<5>,
		mcutl::gpio::gpioa<11>,
		mcutl::gpio::gpioa<14>
	>();
	
	EXPECT_EQ(mask, 
		(mcutl::gpio::pin_bit_mask_v<
			mcutl::gpio::gpioa<1>,
			mcutl::gpio::gpioa<5>,
			mcutl::gpio::gpioa<14>
		>)
	);
	
	EXPECT_CALL(memory(), read(addr(&GPIOA->IDR)));
	
	mask = mcutl::gpio::get_input_values_mask<true,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<5>,
		mcutl::gpio::gpioa<11>,
		mcutl::gpio::gpioa<14>
	>();
	
	EXPECT_EQ(mask, 
		(mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpioa<11>>)
	);
}

TEST_F(gpio_strict_test_fixture, GetOutputMaskTest)
{
	memory().set(addr(&GPIOA->ODR),
		mcutl::gpio::pin_bit_mask_v<
			mcutl::gpio::gpioa<1>,
			mcutl::gpio::gpioa<3>,
			mcutl::gpio::gpioa<5>,
			mcutl::gpio::gpioa<6>,
			mcutl::gpio::gpioa<14>
		>
	);
	
	EXPECT_CALL(memory(), read(addr(&GPIOA->ODR)));
	
	auto mask = mcutl::gpio::get_output_values_mask<false,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<5>,
		mcutl::gpio::gpioa<11>,
		mcutl::gpio::gpioa<14>
	>();
	
	EXPECT_EQ(mask, 
		(mcutl::gpio::pin_bit_mask_v<
			mcutl::gpio::gpioa<1>,
			mcutl::gpio::gpioa<5>,
			mcutl::gpio::gpioa<14>
		>)
	);
	
	EXPECT_CALL(memory(), read(addr(&GPIOA->ODR)));
	
	mask = mcutl::gpio::get_output_values_mask<true,
		mcutl::gpio::gpioa<1>,
		mcutl::gpio::gpioa<5>,
		mcutl::gpio::gpioa<11>,
		mcutl::gpio::gpioa<14>
	>();
	
	EXPECT_EQ(mask, 
		(mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpioa<11>>)
	);
}

TEST_F(gpio_strict_test_fixture, GetInputBitTest)
{
	memory().set(addr(&GPIOC->IDR),
		mcutl::gpio::pin_bit_mask_v<
			mcutl::gpio::gpioc<1>,
			mcutl::gpio::gpioc<3>,
			mcutl::gpio::gpioc<5>,
			mcutl::gpio::gpioc<6>,
			mcutl::gpio::gpioc<14>
		>
	);
	
	EXPECT_CALL(memory(), read(addr(&GPIOC->IDR)));
	EXPECT_TRUE(mcutl::gpio::get_input_bit<mcutl::gpio::gpioc<1>>());
	EXPECT_CALL(memory(), read(addr(&GPIOC->IDR)));
	EXPECT_FALSE(mcutl::gpio::get_input_bit<mcutl::gpio::gpioc<2>>());
	EXPECT_CALL(memory(), read(addr(&GPIOC->IDR)));
	EXPECT_TRUE(mcutl::gpio::get_input_bit<mcutl::gpio::gpioc<3>>());
	EXPECT_CALL(memory(), read(addr(&GPIOC->IDR)));
	EXPECT_FALSE(mcutl::gpio::get_input_bit<mcutl::gpio::gpioc<4>>());
	EXPECT_CALL(memory(), read(addr(&GPIOC->IDR)));
	EXPECT_TRUE(mcutl::gpio::get_input_bit<mcutl::gpio::gpioc<5>>());
}

TEST_F(gpio_strict_test_fixture, GetOutputBitTest)
{
	memory().set(addr(&GPIOD->ODR),
		mcutl::gpio::pin_bit_mask_v<
			mcutl::gpio::gpiod<1>,
			mcutl::gpio::gpiod<3>,
			mcutl::gpio::gpiod<5>,
			mcutl::gpio::gpiod<6>,
			mcutl::gpio::gpiod<14>
		>
	);
	
	EXPECT_CALL(memory(), read(addr(&GPIOD->ODR)));
	EXPECT_TRUE(mcutl::gpio::get_output_bit<mcutl::gpio::gpiod<1>>());
	EXPECT_CALL(memory(), read(addr(&GPIOD->ODR)));
	EXPECT_FALSE(mcutl::gpio::get_output_bit<mcutl::gpio::gpiod<2>>());
	EXPECT_CALL(memory(), read(addr(&GPIOD->ODR)));
	EXPECT_TRUE(mcutl::gpio::get_output_bit<mcutl::gpio::gpiod<3>>());
	EXPECT_CALL(memory(), read(addr(&GPIOD->ODR)));
	EXPECT_FALSE(mcutl::gpio::get_output_bit<mcutl::gpio::gpiod<4>>());
	EXPECT_CALL(memory(), read(addr(&GPIOD->ODR)));
	EXPECT_TRUE(mcutl::gpio::get_output_bit<mcutl::gpio::gpiod<14>>());
	EXPECT_CALL(memory(), read(addr(&GPIOD->ODR)));
	EXPECT_FALSE(mcutl::gpio::get_output_bit<mcutl::gpio::gpiod<15>>());
}

TEST_F(gpio_strict_test_fixture, IsOutputHighTest)
{
	EXPECT_CALL(memory(), read(addr(&GPIOE->CRH)));
	EXPECT_FALSE(mcutl::gpio::is_output<mcutl::gpio::gpioe<12>>());
	
	memory().set(addr(&GPIOE->CRH), GPIO_CRH_MODE12_0);
	EXPECT_CALL(memory(), read(addr(&GPIOE->CRH)));
	EXPECT_TRUE(mcutl::gpio::is_output<mcutl::gpio::gpioe<12>>());
	
	memory().set(addr(&GPIOE->CRH), GPIO_CRH_MODE12_1);
	EXPECT_CALL(memory(), read(addr(&GPIOE->CRH)));
	EXPECT_TRUE(mcutl::gpio::is_output<mcutl::gpio::gpioe<12>>());
	
	memory().set(addr(&GPIOE->CRH), GPIO_CRH_MODE12_0 | GPIO_CRH_MODE12_1);
	EXPECT_CALL(memory(), read(addr(&GPIOE->CRH)));
	EXPECT_TRUE(mcutl::gpio::is_output<mcutl::gpio::gpioe<12>>());
}

TEST_F(gpio_strict_test_fixture, IsOutputLowTest)
{
	EXPECT_CALL(memory(), read(addr(&GPIOG->CRL)));
	EXPECT_FALSE(mcutl::gpio::is_output<mcutl::gpio::gpiog<1>>());
	
	memory().set(addr(&GPIOG->CRL), GPIO_CRL_MODE1_0);
	EXPECT_CALL(memory(), read(addr(&GPIOG->CRL)));
	EXPECT_TRUE(mcutl::gpio::is_output<mcutl::gpio::gpiog<1>>());
	
	memory().set(addr(&GPIOG->CRL), GPIO_CRL_MODE1_1);
	EXPECT_CALL(memory(), read(addr(&GPIOG->CRL)));
	EXPECT_TRUE(mcutl::gpio::is_output<mcutl::gpio::gpiog<1>>());
	
	memory().set(addr(&GPIOG->CRL), GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1);
	EXPECT_CALL(memory(), read(addr(&GPIOG->CRL)));
	EXPECT_TRUE(mcutl::gpio::is_output<mcutl::gpio::gpiog<1>>());
}

TEST_F(gpio_strict_test_fixture, IsInPullUpTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF0_1); //0 - in, pull up
	memory().set(addr(&GPIOB->IDR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<0>>);
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(::testing::AtLeast(1));
	EXPECT_CALL(memory(), read(addr(&GPIOB->IDR))).Times(::testing::AtLeast(1));
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::as_input<mcutl::gpio::gpiob<0>, mcutl::gpio::in::pull_up>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_input<mcutl::gpio::gpiob<3>, mcutl::gpio::in::pull_up>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_input<mcutl::gpio::gpiob<0>, mcutl::gpio::in::pull_down>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_input<mcutl::gpio::gpiob<0>, mcutl::gpio::in::floating>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_input<mcutl::gpio::gpiob<0>, mcutl::gpio::in::analog>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<0>, mcutl::gpio::out::push_pull>>()));
}

TEST_F(gpio_strict_test_fixture, IsOutOpenDrainTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(::testing::AtLeast(1));
	EXPECT_CALL(memory(), read(addr(&GPIOB->ODR))).Times(::testing::AtLeast(1));
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<15>, mcutl::gpio::out::open_drain>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>,
		mcutl::gpio::out::open_drain, mcutl::gpio::out::keep_value, mcutl::gpio::out::opt::freq_10mhz>>()));
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>,
		mcutl::gpio::out::open_drain, mcutl::gpio::out::zero, mcutl::gpio::out::opt::freq_50mhz>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>,
		mcutl::gpio::out::open_drain, mcutl::gpio::out::one, mcutl::gpio::out::opt::freq_50mhz>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::as_input<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down>>()));
}

TEST_F(gpio_strict_test_fixture, IsOutValueTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF0_0 | GPIO_CRL_MODE0_0 | GPIO_CRL_MODE0_1); //0 - out, open drain, 50MHz
	memory().set(addr(&GPIOB->ODR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<0>>);
	
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(::testing::AtLeast(1));
	EXPECT_CALL(memory(), read(addr(&GPIOB->ODR))).Times(::testing::AtLeast(1));
	
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::to_value<mcutl::gpio::gpiob<0>, mcutl::gpio::out::one>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::to_value<mcutl::gpio::gpiob<0>, mcutl::gpio::out::zero>>()));
}

TEST_F(gpio_strict_test_fixture, IsInValueTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF0_1); //0 - in, pull up
	memory().set(addr(&GPIOB->IDR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<0>>);
	
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(::testing::AtLeast(1));
	EXPECT_CALL(memory(), read(addr(&GPIOB->IDR))).Times(::testing::AtLeast(1));
	
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::to_value<mcutl::gpio::gpiob<0>, mcutl::gpio::out::one>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::to_value<mcutl::gpio::gpiob<0>, mcutl::gpio::out::zero>>()));
}

TEST_F(gpio_strict_test_fixture, HasInOutLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1); //1 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL)));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::keep_value>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL)));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::keep_value>()));
}

TEST_F(gpio_strict_test_fixture, HasInOutHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH)));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::keep_value>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH)));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::keep_value>()));
}

TEST_F(gpio_strict_test_fixture, HasValueLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1); //1 - in, pull down
	memory().set(addr(&GPIOB->ODR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<1>>);
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(2);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::zero>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(3);
	EXPECT_CALL(memory(), read(addr(&GPIOB->ODR))).Times(2);
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::keep_value>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::zero>()));
}

TEST_F(gpio_strict_test_fixture, HasValueHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //1 - in, pull down
	memory().set(addr(&GPIOB->ODR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<11>>);
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(2);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::zero>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(3);
	EXPECT_CALL(memory(), read(addr(&GPIOB->ODR))).Times(2);
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::keep_value>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::zero>()));
}

TEST_F(gpio_strict_test_fixture, HasOutFreqLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1); //1 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(3);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::opt::freq_10mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::opt::freq_2mhz>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(3);
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::opt::freq_10mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::opt::freq_2mhz>()));
}

TEST_F(gpio_strict_test_fixture, HasOutFreqHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(3);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::opt::freq_10mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::opt::freq_2mhz>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(3);
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::opt::freq_10mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::opt::freq_2mhz>()));
}

TEST_F(gpio_strict_test_fixture, HasOutModeLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1); //1 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain_alt_func>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain_alt_func>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull>()));
}

TEST_F(gpio_strict_test_fixture, HasOutModeHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull>()));
}

TEST_F(gpio_strict_test_fixture, HasOutModeAndFreqLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1);  //1 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(6);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_2mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_10mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz>()));
}

TEST_F(gpio_strict_test_fixture, HasOutModeAndFreqHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(6);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_2mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_10mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz>()));
}

TEST_F(gpio_strict_test_fixture, HasOutModeAndFreqAndValueLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1); //1 - in, pull up
	memory().set(addr(&GPIOB->IDR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<1>>);
	memory().set(addr(&GPIOB->ODR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<1>>);
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(8);
	EXPECT_CALL(memory(), read(addr(&GPIOB->ODR))).Times(2);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::keep_value>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::zero>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_2mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_10mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
}

TEST_F(gpio_strict_test_fixture, HasOutModeAndFreqAndValueHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull up
	memory().set(addr(&GPIOB->IDR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<11>>);
	memory().set(addr(&GPIOB->ODR), mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<11>>);
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(8);
	EXPECT_CALL(memory(), read(addr(&GPIOB->ODR))).Times(2);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::keep_value>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::zero>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_2mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::opt::freq_10mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::open_drain_alt_func,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::out::push_pull,
		mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>()));
}

TEST_F(gpio_strict_test_fixture, HasInModeLowTest)
{
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_1); //1 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(4);
	EXPECT_CALL(memory(), read(addr(&GPIOB->IDR))).Times(2);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::analog>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::floating>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::pull_down>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::pull_up>()));
	
	memory().set(addr(&GPIOB->CRL), GPIO_CRL_CNF1_0 | GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1); //1 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRL))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::analog>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::floating>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::pull_down>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::pull_up>()));
}

TEST_F(gpio_strict_test_fixture, HasInModeHighTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull down
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(4);
	EXPECT_CALL(memory(), read(addr(&GPIOB->IDR))).Times(2);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::analog>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::floating>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_up>()));
	
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1); //11 - out, open drain, 50MHz
	EXPECT_CALL(memory(), read(addr(&GPIOB->CRH))).Times(4);
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::analog>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::floating>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_up>()));
}

TEST_F(gpio_strict_test_fixture, ExtiGpioLinesTest)
{
	EXPECT_EQ(mcutl::gpio::default_exti_line_type<mcutl::gpio::gpioa<3>>::value, 3u);
	EXPECT_EQ(mcutl::gpio::default_exti_line_type<mcutl::gpio::gpiof<15>>::value, 15u);
	EXPECT_EQ((mcutl::gpio::exti_lines_bit_mask_v<mcutl::gpio::gpiof<15>,
		mcutl::gpio::gpioa<3>>), (1u << 3) | (1u << 15));
	
	using test_config = mcutl::gpio::config<
		mcutl::gpio::to_value<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>,
		mcutl::gpio::as_input<mcutl::gpio::gpioa<0>, mcutl::gpio::in::pull_up>,
		mcutl::gpio::as_output<mcutl::gpio::gpioa<3>, mcutl::gpio::out::open_drain>,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpioa<8>>,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiob<11>>,
		mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpioa<3>>
	>;
	EXPECT_EQ(mcutl::gpio::exti_lines_bit_mask_v<test_config>, (1u << 3) | (1u << 11) | (1u << 8));
}

TEST_F(gpio_strict_test_fixture, IsExtiConnectedTest)
{
	memory().set(addr(&(AFIO->EXTICR[3])), AFIO_EXTICR4_EXTI15_PD);
	memory().allow_reads(addr(&(AFIO->EXTICR[3])));
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiod<15>>>()));
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpiod<15>>>()));
	
	memory().set(addr(&(AFIO->EXTICR[3])), AFIO_EXTICR4_EXTI15_PA);
	EXPECT_FALSE((mcutl::gpio::is<mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiod<15>>>()));
	EXPECT_TRUE((mcutl::gpio::is<mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpiod<15>>>()));
}

TEST_F(gpio_strict_test_fixture, HasExtiTest)
{
	memory().set(addr(&GPIOB->CRH), GPIO_CRH_CNF11_1); //11 - in, pull down
	memory().allow_reads(addr(&GPIOB->CRH));
	memory().allow_reads(addr(&GPIOB->IDR));
	memory().allow_reads(addr(&(AFIO->EXTICR[2])));
	memory().set(addr(&(AFIO->EXTICR[2])), AFIO_EXTICR3_EXTI11_PF);
	
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiob<11>>>()));
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down,
		mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpiob<11>>>()));
	
	memory().set(addr(&(AFIO->EXTICR[2])), AFIO_EXTICR3_EXTI11_PB);
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiob<11>>>()));
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>, mcutl::gpio::in::pull_down,
		mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpiob<11>>>()));
}

TEST_F(gpio_strict_test_fixture, HasExtiUnsupportedPinLineTest)
{
	EXPECT_FALSE((mcutl::gpio::has<mcutl::gpio::gpiob<11>,
		mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpiob<11>, 12>>()));
	
	EXPECT_TRUE((mcutl::gpio::has<mcutl::gpio::gpiob<11>,
		mcutl::gpio::disconnect_from_exti_line<mcutl::gpio::gpiob<11>, 12>>()));
}
