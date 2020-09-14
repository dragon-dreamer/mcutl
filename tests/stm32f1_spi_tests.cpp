#define STM32F103xG
#define STM32F1

#include <stdint.h>
#include <type_traits>
#include <utility>

#include "mcutl/clock/clock.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph.h"
#include "mcutl/spi/spi.h"
#include "mcutl/tests/mcu.h"
#include "mcutl/utils/type_helpers.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "stm32f1_exti_interrupt_test_fixture.h"
#include "stm32f1_dma_test_fixture.h"

class spi_strict_test_fixture_base
	: public exti_interrupt_test_fixture
	, public dma_strict_test_fixture
{
};

template<typename Spi> struct spi_map {};
template<typename Periph, typename Interrupt, uint32_t Reg,
	uint32_t RxDma, uint32_t TxDma, typename ClockConfig>
struct spi_info
{
	using periph = Periph;
	using interrupt = Interrupt;
	static constexpr auto reg = Reg;
	static constexpr auto rx_dma = RxDma;
	static constexpr auto tx_dma = TxDma;
	using clock_config = ClockConfig;
};

using namespace mcutl::clock::literals;
using spi1_clock_config = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal,
	mcutl::clock::apb2<mcutl::clock::required_frequency<32_MHz>>,
	mcutl::clock::spi1<mcutl::clock::required_frequency<16_MHz>>
>;
using spi2_clock_config = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal,
	mcutl::clock::apb1<mcutl::clock::required_frequency<32_MHz>>,
	mcutl::clock::spi2<mcutl::clock::required_frequency<16_MHz>>
>;
using spi3_clock_config = mcutl::clock::config<
	mcutl::clock::internal_high_speed_crystal,
	mcutl::clock::apb1<mcutl::clock::required_frequency<32_MHz>>,
	mcutl::clock::spi3<mcutl::clock::required_frequency<16_MHz>>
>;

template<> struct spi_map<mcutl::spi::spi1>
	: spi_info<mcutl::periph::spi1, mcutl::interrupt::type::spi1, SPI1_BASE,
		DMA1_Channel2_BASE, DMA1_Channel3_BASE, spi1_clock_config> {};
template<> struct spi_map<mcutl::spi::spi2>
	: spi_info<mcutl::periph::spi2, mcutl::interrupt::type::spi2, SPI2_BASE,
		DMA1_Channel4_BASE, DMA1_Channel5_BASE, spi2_clock_config> {};
template<> struct spi_map<mcutl::spi::spi3>
	: spi_info<mcutl::periph::spi3, mcutl::interrupt::type::spi3, SPI3_BASE,
		DMA2_Channel1_BASE, DMA2_Channel2_BASE, spi3_clock_config> {};

template<typename Spi>
class spi_strict_test_fixture : public spi_strict_test_fixture_base
{
public:
	using spi = Spi;
	
public:
	static auto spi_reg() noexcept
	{
		return mcutl::memory::volatile_memory<SPI_TypeDef>(spi_map<spi>::reg);
	}
	
	static auto rx_dma() noexcept
	{
		return mcutl::memory::volatile_memory<DMA_Channel_TypeDef>(spi_map<spi>::rx_dma);
	}
	
	static auto tx_dma() noexcept
	{
		return mcutl::memory::volatile_memory<DMA_Channel_TypeDef>(spi_map<spi>::tx_dma);
	}
	
	template<typename DataType = uint8_t>
	static decltype(auto) create_dma_spi()
	{
		return mcutl::spi::create_spi_master<spi, DataType>(mcutl::spi::dma_receive_mode<>(),
			mcutl::spi::dma_transmit_mode<>());
	}
	
	void expect_clear_error_flags()
	{
		EXPECT_CALL(memory(), read(addr(&spi_reg()->CR1)));
		EXPECT_CALL(memory(), read(addr(&spi_reg()->DR)));
		EXPECT_CALL(memory(), read(addr(&spi_reg()->SR)));
		EXPECT_CALL(memory(), write(addr(&spi_reg()->SR), 0u));
		EXPECT_CALL(memory(), write(addr(&spi_reg()->CR1), ::testing::_))
			.WillOnce([this](auto addr, auto val) { EXPECT_EQ(val, memory().get(addr)); });
	}
	
	void expect_dma_wait(uint32_t tx_times, bool rx_dma_enable, uint32_t rx_times)
	{
		memory().set(addr(&tx_dma()->CNDTR), tx_times);
		memory().set(addr(&rx_dma()->CNDTR), rx_times);
		memory().allow_reads(addr(&rx_dma()->CCR));
		memory().allow_reads(addr(&tx_dma()->CCR));
		
		::testing::InSequence s;
		
		EXPECT_CALL(memory(), read(addr(&spi_reg()->CR2)))
			.WillOnce([rx_dma_enable] (auto) { return rx_dma_enable ? SPI_CR2_RXDMAEN : 0; });
		
		if (rx_dma_enable)
		{
			EXPECT_CALL(memory(), read(addr(&rx_dma()->CCR)))
				.WillOnce([] (auto) { return DMA_CCR_EN; });
			
			EXPECT_CALL(memory(), read(addr(&rx_dma()->CNDTR)))
				.Times(rx_times)
				.WillRepeatedly([this] (auto) {
					return --memory().get(addr(&rx_dma()->CNDTR));
				});
			
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
				::testing::IsEmpty()));
		}
		
		EXPECT_CALL(memory(), read(addr(&tx_dma()->CCR)))
			.WillOnce([] (auto) { return DMA_CCR_EN; });
		
		EXPECT_CALL(memory(), read(addr(&tx_dma()->CNDTR)))
			.Times(tx_times)
			.WillRepeatedly([this] (auto) {
				return --memory().get(addr(&tx_dma()->CNDTR));
			});
		
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
			::testing::IsEmpty()));
		
		EXPECT_CALL(memory(), read(addr(&spi_reg()->SR)))
			.WillOnce([] (auto) { return 0; })
			.WillOnce([] (auto) { return SPI_SR_TXE; });
		
		EXPECT_CALL(memory(), read(addr(&spi_reg()->SR)))
			.WillOnce([] (auto) { return SPI_SR_BSY; })
			.WillOnce([] (auto) { return 0; });
	}
	
	void expect_spi_dma_transmit(const void* data, uint16_t length)
	{
		::testing::Sequence s1, s2, s3;
		EXPECT_CALL(memory(), write(addr(&spi_reg()->CR2),
			memory().get(addr(&spi_reg()->CR2)) & ~SPI_CR2_RXDMAEN))
			.InSequence(s1, s2, s3);
		expect_start_transfer(spi_map<spi>::tx_dma, DMA_CCR_DIR,
			data, &spi_reg()->DR, length, s1, s2, s3);
	}
	
	void expect_spi_dma_transmit_receive(const void* tx_data, void* rx_data, uint16_t length)
	{
		::testing::Sequence s1, s2, s3;
		EXPECT_CALL(memory(), write(addr(&spi_reg()->CR2),
			memory().get(addr(&spi_reg()->CR2)) | SPI_CR2_RXDMAEN))
			.InSequence(s1, s2, s3);
		expect_start_transfer(spi_map<spi>::rx_dma, 0u,
			&spi_reg()->DR, rx_data, length, s1, s2, s3);
		expect_start_transfer(spi_map<spi>::tx_dma, DMA_CCR_DIR,
			tx_data, &spi_reg()->DR, length, s1, s2, s3);
	}
	
	void prepare_spi_memory_before_config()
	{
		this->memory().set(this->addr(&this->spi_reg()->CR1), initial_cr1);
		this->memory().allow_reads(this->addr(&this->spi_reg()->CR1));
		this->memory().set(this->addr(&this->spi_reg()->CR2), initial_cr2);
		this->memory().allow_reads(this->addr(&this->spi_reg()->CR2));
	}
	
	template<typename DoBeforeDmaConfig>
	void expect_configure_dma_spi(uint32_t spi_cr1, uint32_t spi_cr2,
		DoBeforeDmaConfig&& do_before_dma_config)
	{
		::testing::InSequence s;
		EXPECT_CALL(this->memory(), write(this->addr(&this->spi_reg()->CR1),
			initial_cr1 & ~SPI_CR1_SPE));
		EXPECT_CALL(this->memory(), write(this->addr(&this->spi_reg()->CR1), spi_cr1 | SPI_CR1_BR_Msk));
		EXPECT_CALL(this->memory(), write(this->addr(&this->spi_reg()->CR2),
			spi_cr2 | SPI_CR2_TXDMAEN));
		std::forward<DoBeforeDmaConfig>(do_before_dma_config)();
		this->expect_configure(spi_map<spi>::tx_dma, DMA_CCR_DIR | DMA_CCR_MINC,
			0, 0, 0);
		this->expect_configure(spi_map<spi>::rx_dma, DMA_CCR_MINC,
			0, 0, 0);
	}
	
	void expect_configure_dma_spi(uint32_t spi_cr1, uint32_t spi_cr2)
	{
		expect_configure_dma_spi(spi_cr1, spi_cr2, []{});
	}
	
private:
	static constexpr uint32_t initial_cr1 = 0xffffffff;
	static constexpr uint32_t initial_cr2 = 0xffffffff & ~(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);
	
public:
	const uint8_t* uint8_t_tx_data = reinterpret_cast<const uint8_t*>(0x12345678);
	const uint16_t* uint16_t_tx_data = reinterpret_cast<const uint16_t*>(0xabcdef);
	uint8_t* uint8_t_rx_data = reinterpret_cast<uint8_t*>(0x56789012);
	uint16_t* uint16_t_rx_data = reinterpret_cast<uint16_t*>(0xbcdef123);
	static constexpr uint16_t data_length = 16;
};

using spi_list = ::testing::Types<mcutl::spi::spi1, mcutl::spi::spi2, mcutl::spi::spi3>;

template<typename Spi> using spi_list_test_fixture = spi_strict_test_fixture<Spi>;
TYPED_TEST_SUITE(spi_list_test_fixture, spi_list);

TYPED_TEST(spi_list_test_fixture, TraitsTest)
{
	using spi = typename TestFixture::spi;
	
	EXPECT_TRUE(mcutl::spi::supports_software_slave_management<spi>);
	EXPECT_TRUE(mcutl::spi::supports_hardware_with_output_slave_management<spi>);
	EXPECT_TRUE(mcutl::spi::supports_hardware_without_output_slave_management<spi>);
	EXPECT_TRUE(mcutl::spi::supports_frame_format<spi>);
	EXPECT_TRUE(mcutl::spi::supports_error_interrupt<spi>);
	EXPECT_TRUE((std::is_same_v<mcutl::spi::peripheral_type<spi>, typename spi_map<spi>::periph>));
	EXPECT_TRUE((std::is_same_v<mcutl::spi::interrupt_type<spi, mcutl::spi::interrupt::error>,
		typename spi_map<spi>::interrupt>));
	EXPECT_TRUE((std::is_same_v<mcutl::spi::interrupt_type<spi, mcutl::spi::interrupt::transmit_buffer_empty>,
		typename spi_map<spi>::interrupt>));
	EXPECT_TRUE((std::is_same_v<mcutl::spi::interrupt_type<spi, mcutl::spi::interrupt::recieve_buffer_not_empty>,
		typename spi_map<spi>::interrupt>));
	EXPECT_TRUE((std::is_same_v<mcutl::spi::supported_data_types<spi>,
		mcutl::types::list<uint8_t, int8_t, uint16_t, std::byte>>));
}

TYPED_TEST(spi_list_test_fixture, PendingFlagsTest)
{
	using spi = typename TestFixture::spi;
	EXPECT_EQ((mcutl::spi::pending_flags_v<spi,
		mcutl::spi::interrupt::error>), SPI_SR_OVR | SPI_SR_MODF | SPI_SR_CRCERR | SPI_SR_UDR);
	EXPECT_EQ((mcutl::spi::pending_flags_v<spi,
		mcutl::spi::interrupt::transmit_buffer_empty>), SPI_SR_TXE);
	EXPECT_EQ((mcutl::spi::pending_flags_v<spi,
		mcutl::spi::interrupt::recieve_buffer_not_empty>), SPI_SR_RXNE);
	EXPECT_EQ((mcutl::spi::pending_flags_v<spi,
		mcutl::spi::interrupt::transmit_buffer_empty,
		mcutl::spi::interrupt::recieve_buffer_not_empty>), SPI_SR_TXE | SPI_SR_RXNE);
	EXPECT_EQ(mcutl::spi::pending_flags_v<spi>, 0u);
}

TYPED_TEST(spi_list_test_fixture, GetPendingFlagsTest)
{
	using spi = typename TestFixture::spi;
	this->memory().allow_reads(this->addr(&this->spi_reg()->SR));
	
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi,
		mcutl::spi::interrupt::error,
		mcutl::spi::interrupt::transmit_buffer_empty,
		mcutl::spi::interrupt::recieve_buffer_not_empty>()), 0u);
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi>()), 0u);
	
	this->memory().set(this->addr(&this->spi_reg()->SR), SPI_SR_MODF);
	
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi,
		mcutl::spi::interrupt::error>()), SPI_SR_MODF);
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi,
		mcutl::spi::interrupt::transmit_buffer_empty>()), 0u);
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi>()), 0u);
	
	this->memory().set(this->addr(&this->spi_reg()->SR), SPI_SR_TXE | SPI_SR_RXNE);
	
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi,
		mcutl::spi::interrupt::error>()), 0u);
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi,
		mcutl::spi::interrupt::transmit_buffer_empty>()), SPI_SR_TXE);
	EXPECT_EQ((mcutl::spi::get_pending_flags<spi,
		mcutl::spi::interrupt::recieve_buffer_not_empty>()), SPI_SR_RXNE);
}

TYPED_TEST(spi_list_test_fixture, EnableTest)
{
	static constexpr uint32_t initial_cr1 = 0xffffffffu & ~SPI_CR1_SPE;
	this->memory().allow_reads(this->addr(&this->spi_reg()->CR1));
	this->memory().set(this->addr(&this->spi_reg()->CR1), initial_cr1);
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->spi_reg()->CR1),
		initial_cr1 | SPI_CR1_SPE));
	
	auto obj = this->create_dma_spi();
	obj.enable();
}

TYPED_TEST(spi_list_test_fixture, DisableTest)
{
	static constexpr uint32_t initial_cr1 = 0xffffffffu;
	this->memory().allow_reads(this->addr(&this->spi_reg()->CR1));
	this->memory().set(this->addr(&this->spi_reg()->CR1), initial_cr1);
	
	::testing::InSequence s;
	this->expect_dma_wait(10, true, 10);
	EXPECT_CALL(this->memory(),
		write(this->addr(&this->spi_reg()->CR1),
			initial_cr1 & ~SPI_CR1_SPE));
	
	auto obj = this->create_dma_spi();
	obj.disable();
}

TYPED_TEST(spi_list_test_fixture, WaitTest1)
{
	this->expect_dma_wait(10, true, 10);
	auto obj = this->create_dma_spi();
	obj.wait();
}

TYPED_TEST(spi_list_test_fixture, WaitTest2)
{
	this->expect_dma_wait(10, false, 10);
	auto obj = this->create_dma_spi();
	obj.wait();
}

TYPED_TEST(spi_list_test_fixture, GpioConfigTest1)
{
	using spi = typename TestFixture::spi;
	
	auto obj = this->create_dma_spi();
	if constexpr (std::is_same_v<spi, mcutl::spi::spi1>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)::template gpio_config<>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<5>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpioa<6>, mcutl::gpio::in::pull_up>,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<7>, mcutl::gpio::out::push_pull_alt_func>>
			>));
	}
	else if constexpr (std::is_same_v<spi, mcutl::spi::spi2>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)::template gpio_config<>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<13>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpiob<14>, mcutl::gpio::in::pull_up>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<15>, mcutl::gpio::out::push_pull_alt_func>>
			>));
	}
	else if constexpr (std::is_same_v<spi, mcutl::spi::spi3>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)::template gpio_config<>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<3>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpiob<4>, mcutl::gpio::in::pull_up>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<5>, mcutl::gpio::out::push_pull_alt_func>>
			>));
	}
}

TYPED_TEST(spi_list_test_fixture, GpioConfigTest2)
{
	using spi = typename TestFixture::spi;
	
	auto obj = this->create_dma_spi();
	if constexpr (std::is_same_v<spi, mcutl::spi::spi1>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)
			::template gpio_config<mcutl::spi::slave_management::hardware_with_output>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<5>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpioa<6>, mcutl::gpio::in::pull_up>,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<7>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<4>, mcutl::gpio::out::push_pull_alt_func>>
			>));
	}
	else if constexpr (std::is_same_v<spi, mcutl::spi::spi2>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)
			::template gpio_config<mcutl::spi::slave_management::hardware_with_output>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<13>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpiob<14>, mcutl::gpio::in::pull_up>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<15>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<12>, mcutl::gpio::out::push_pull_alt_func>>
			>));
	}
	else if constexpr (std::is_same_v<spi, mcutl::spi::spi3>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)
			::template gpio_config<mcutl::spi::slave_management::hardware_with_output>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<3>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpiob<4>, mcutl::gpio::in::pull_up>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<5>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<15>, mcutl::gpio::out::push_pull_alt_func>>
			>));
	}
}

TYPED_TEST(spi_list_test_fixture, GpioConfigTest3)
{
	using spi = typename TestFixture::spi;
	
	auto obj = mcutl::spi::create_spi_master<spi>(mcutl::spi::empty_mode(),
		mcutl::spi::dma_transmit_mode<>());
	if constexpr (std::is_same_v<spi, mcutl::spi::spi1>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)
			::template gpio_config<mcutl::spi::slave_management::hardware_without_output>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<5>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_output<mcutl::gpio::gpioa<7>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpioa<4>, mcutl::gpio::in::pull_up>>
			>));
	}
	else if constexpr (std::is_same_v<spi, mcutl::spi::spi2>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)
			::template gpio_config<mcutl::spi::slave_management::hardware_without_output>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<13>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<15>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpiob<12>, mcutl::gpio::in::pull_up>>
			>));
	}
	else if constexpr (std::is_same_v<spi, mcutl::spi::spi3>)
	{
		EXPECT_TRUE((std::is_same_v<typename decltype(obj)
			::template gpio_config<mcutl::spi::slave_management::hardware_without_output>,
			mcutl::gpio::config<
				mcutl::gpio::enable_peripherals,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<3>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_output<mcutl::gpio::gpiob<5>, mcutl::gpio::out::push_pull_alt_func>,
				mcutl::gpio::as_input<mcutl::gpio::gpioa<15>, mcutl::gpio::in::pull_up>>
			>));
	}
}

TYPED_TEST(spi_list_test_fixture, ClearErrorFlagsTest)
{
	this->expect_clear_error_flags();
	auto spi = this->create_dma_spi();
	spi.clear_error_flags();
}

TYPED_TEST(spi_list_test_fixture, DmaTransmitTest)
{
	constexpr uint32_t initial_cr2 = 0xffffffff;
	this->memory().set(this->addr(&this->spi_reg()->CR2), initial_cr2);
	this->memory().allow_reads(this->addr(&this->spi_reg()->CR2));
	
	this->expect_spi_dma_transmit(this->uint8_t_tx_data, this->data_length);
	
	auto spi8 = this->create_dma_spi();
	spi8.transmit(this->uint8_t_tx_data, this->data_length);
	
	this->expect_spi_dma_transmit(this->uint16_t_tx_data, this->data_length);
	
	auto spi16 = this->template create_dma_spi<uint16_t>();
	spi16.transmit(this->uint16_t_tx_data, this->data_length);
}

TYPED_TEST(spi_list_test_fixture, DmaTransmitReceiveTest)
{
	constexpr uint32_t initial_cr2 = 0xffffffff & ~SPI_CR2_RXDMAEN;
	this->memory().set(this->addr(&this->spi_reg()->CR2), initial_cr2);
	this->memory().allow_reads(this->addr(&this->spi_reg()->CR2));
	
	this->expect_spi_dma_transmit_receive(this->uint8_t_tx_data,
		this->uint8_t_rx_data, this->data_length);
	
	auto spi8 = this->create_dma_spi();
	spi8.transmit_receive(this->uint8_t_tx_data, this->uint8_t_rx_data,
		this->data_length);
	
	this->expect_spi_dma_transmit_receive(this->uint16_t_tx_data,
		this->uint16_t_rx_data, this->data_length);
	
	auto spi16 = this->template create_dma_spi<uint16_t>();
	spi16.transmit_receive(this->uint16_t_tx_data, this->uint16_t_rx_data,
		this->data_length);
}

TYPED_TEST(spi_list_test_fixture, ConfigureTest1)
{
	this->prepare_spi_memory_before_config();
	this->expect_configure_dma_spi(SPI_CR1_MSTR, 0);
	
	auto spi8 = this->create_dma_spi();
	spi8.configure();
}

TYPED_TEST(spi_list_test_fixture, ConfigureTest2)
{
	this->prepare_spi_memory_before_config();
	this->expect_configure_dma_spi(
		SPI_CR1_MSTR | SPI_CR1_CPHA | SPI_CR1_CPOL | SPI_CR1_DFF | SPI_CR1_LSBFIRST
		| SPI_CR1_SSM | SPI_CR1_SSI, 0);
	
	auto spi16 = this->template create_dma_spi<uint16_t>();
	spi16.template configure<mcutl::spi::frame_format::lsb_first,
		mcutl::spi::cpol_1, mcutl::spi::cpha_1,
		mcutl::spi::slave_management::software>();
}

TYPED_TEST(spi_list_test_fixture, ConfigureTest3)
{
	this->prepare_spi_memory_before_config();
	::testing::InSequence s;
	this->expect_clear_error_flags();
	this->expect_configure_dma_spi(SPI_CR1_MSTR, SPI_CR2_SSOE);
	
	auto spi8 = this->template create_dma_spi();
	spi8.template configure<mcutl::spi::slave_management::hardware_with_output,
		mcutl::spi::clear_error_flags>();
}

TYPED_TEST(spi_list_test_fixture, ConfigureTest4)
{
	this->prepare_spi_memory_before_config();
	this->expect_configure_dma_spi(SPI_CR1_MSTR, SPI_CR2_ERRIE);
	
	auto spi8 = this->template create_dma_spi();
	spi8.template configure<mcutl::spi::interrupt::error>();
}

TYPED_TEST(spi_list_test_fixture, ConfigureTest5)
{
	using spi = typename TestFixture::spi;
	
	this->prepare_spi_memory_before_config();
	::testing::InSequence s;
	this->expect_configure_dma_spi(SPI_CR1_MSTR, SPI_CR2_ERRIE,
	[this] {
		this->expect_enable_interrupt(spi_map<spi>::interrupt::irqn, 2u << 2, 3u);
	});
	
	auto spi8 = this->template create_dma_spi();
	spi8.template configure<
		mcutl::interrupt::interrupt<mcutl::spi::interrupt::error, 2, 3>,
		mcutl::interrupt::priority_count<4>,
		mcutl::spi::interrupt::enable_controller_interrupts>();
}

TYPED_TEST(spi_list_test_fixture, ConfigureTest6)
{
	using spi = typename TestFixture::spi;
	
	this->prepare_spi_memory_before_config();
	::testing::InSequence s;
	this->expect_configure_dma_spi(SPI_CR1_MSTR, 0,
	[this] {
		this->expect_disable_interrupt(spi_map<spi>::interrupt::irqn);
	});
	
	auto spi8 = this->template create_dma_spi();
	spi8.template configure<
		mcutl::interrupt::disabled<mcutl::spi::interrupt::error>,
		mcutl::spi::interrupt::disable_controller_interrupts>();
}

TYPED_TEST(spi_list_test_fixture, EnableSourceIncrementTest)
{
	using spi = typename TestFixture::spi;
	
	uint32_t initial_ccr = DMA_CCR_DIR;
	uint32_t new_ccr = initial_ccr | DMA_CCR_MINC;
	
	this->expect_configure(spi_map<spi>::tx_dma, new_ccr, 0, 0, 0, initial_ccr);
	
	auto spi8 = this->create_dma_spi();
	spi8.template enable_source_pointer_increment<true>();
}

TYPED_TEST(spi_list_test_fixture, DisableSourceIncrementTest)
{
	using spi = typename TestFixture::spi;
	
	uint32_t initial_ccr = DMA_CCR_DIR | DMA_CCR_MINC;
	uint32_t new_ccr = initial_ccr & ~DMA_CCR_MINC;
	
	this->expect_configure(spi_map<spi>::tx_dma, new_ccr, 0, 0, 0, initial_ccr);
	
	auto spi8 = this->create_dma_spi();
	spi8.template enable_source_pointer_increment<false>();
}

TYPED_TEST(spi_list_test_fixture, ChangePrescalerTest)
{
	using spi = typename TestFixture::spi;
	
	constexpr uint32_t initial_cr1 = 0xffffffffu & ~SPI_CR1_BR_Msk;
	this->memory().set(this->addr(&this->spi_reg()->CR1), initial_cr1);
	this->memory().allow_reads(this->addr(&this->spi_reg()->CR1));
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->spi_reg()->CR1),
		initial_cr1 | SPI_CR1_BR_0 | SPI_CR1_BR_1));
	
	mcutl::spi::change_prescaler<spi, typename spi_map<spi>::clock_config,
		mcutl::clock::required_frequency<2_MHz>>();
	
	EXPECT_CALL(this->memory(), write(this->addr(&this->spi_reg()->CR1),
		initial_cr1 | SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2));
	
	mcutl::spi::change_prescaler<spi, typename spi_map<spi>::clock_config,
		mcutl::clock::max_frequency<200_KHz>>();
}
