#pragma once

#include <array>
#include <cstddef>
#include <stdint.h>
#include <type_traits>

#include "mcutl/clock/clock.h"
#include "mcutl/dma/dma.h"
#include "mcutl/device/device.h"
#include "mcutl/gpio/gpio.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/spi/spi_defs.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::spi
{

struct options : mcutl::spi::detail::options
{
	uint32_t spi_cr1 = 0u;
	uint32_t spi_cr2 = 0u;
	uint32_t spi_cr1_mask = 0u;
	uint32_t spi_cr2_mask = 0u;
	mcutl::interrupt::priority_t interrupt_priority = mcutl::interrupt::default_priority;
	mcutl::interrupt::priority_t interrupt_subpriority = mcutl::interrupt::default_priority;
	bool priority_conflict = false;
};

using supported_data_types = types::list<uint8_t, int8_t, uint16_t, std::byte>;

template<uint8_t SpiIndex, typename Peripheral, uint32_t Base,
	typename MisoPin, typename MosiPin, typename SckPin, typename NssPin,
	clock::device_source_id ClockId>
struct spi_base : mcutl::spi::detail::spi_base<
	SpiIndex, Peripheral, MisoPin, MosiPin, SckPin, NssPin,
	mcutl::spi::slave_management::none, supported_data_types>
{
	static constexpr uint32_t base = Base;
	static constexpr clock::device_source_id clock_id = ClockId;
	
	template<typename Device, typename OptionsLambda>
	static constexpr auto get_gpio_config(OptionsLambda options_lambda) noexcept
	{
		constexpr auto options = options_lambda();
		static_assert((options.spi_cr1 & SPI_CR1_MSTR) != 0,
			"Only master config is currently supported");
		
		using pin_cfg = mcutl::gpio::config<mcutl::gpio::enable_peripherals,
			mcutl::gpio::as_output<SckPin, mcutl::gpio::out::push_pull_alt_func>>;
		using pin_cfg_r = std::conditional_t<
			std::is_same_v<typename Device::receive_mode, mcutl::spi::empty_mode>,
			pin_cfg,
			types::push_back_t<pin_cfg, mcutl::gpio::as_input<MisoPin, mcutl::gpio::in::pull_up>>>;
		using pin_cfg_rt = std::conditional_t<
			std::is_same_v<typename Device::transmit_mode, mcutl::spi::empty_mode>,
			pin_cfg_r,
			types::push_back_t<pin_cfg_r, mcutl::gpio::as_output<MosiPin, mcutl::gpio::out::push_pull_alt_func>>>;
		using pin_cfg_rtss = std::conditional_t<
			options.slave_management == mcutl::spi::detail::slave_management_type::hardware_with_output,
			types::push_back_t<pin_cfg_rt, mcutl::gpio::as_output<NssPin, mcutl::gpio::out::push_pull_alt_func>>,
			pin_cfg_rt>;
		using pin_cfg_final = std::conditional_t<
			options.slave_management == mcutl::spi::detail::slave_management_type::hardware_without_output,
			types::push_back_t<pin_cfg_rtss, mcutl::gpio::as_input<NssPin, mcutl::gpio::in::pull_up>>,
			pin_cfg_rtss>;
		
		return pin_cfg_final{};
	}
};

} //namespace mcutl::device::spi

namespace mcutl::spi
{

namespace detail
{

template<typename Spi>
struct dma_channel_helper
{
	static_assert(types::always_false<Spi>::value,
		"Selected SPI is not mapped to any DMA channels");
};

} //namespace detail

#ifdef SPI1
using spi1 = mcutl::device::spi::spi_base<1, mcutl::periph::spi1, SPI1_BASE,
	gpio::gpioa<6>, gpio::gpioa<7>, gpio::gpioa<5>, gpio::gpioa<4>,
	mcutl::device::clock::device_source_id::spi1>;

namespace detail
{

template<> struct dma_channel_helper<spi1>
{
	using rx = dma::dma1<2>;
	using tx = dma::dma1<3>;
};

template<>
struct interrupt_type_helper<spi1, interrupt::recieve_buffer_not_empty>
	: types::identity<mcutl::interrupt::type::spi1>
{
};

template<>
struct interrupt_type_helper<spi1, interrupt::transmit_buffer_empty>
	: types::identity<mcutl::interrupt::type::spi1>
{
};

template<>
struct interrupt_type_helper<spi1, interrupt::error>
	: types::identity<mcutl::interrupt::type::spi1>
{
};

} //namespace detail
#endif //SPI1

#ifdef SPI2
using spi2 = mcutl::device::spi::spi_base<2, mcutl::periph::spi2, SPI2_BASE,
	gpio::gpiob<14>, gpio::gpiob<15>, gpio::gpiob<13>, gpio::gpiob<12>,
	mcutl::device::clock::device_source_id::spi2>;

namespace detail
{

template<> struct dma_channel_helper<spi2>
{
	using rx = dma::dma1<4>;
	using tx = dma::dma1<5>;
};

template<>
struct interrupt_type_helper<spi2, interrupt::recieve_buffer_not_empty>
	: types::identity<mcutl::interrupt::type::spi2>
{
};

template<>
struct interrupt_type_helper<spi2, interrupt::transmit_buffer_empty>
	: types::identity<mcutl::interrupt::type::spi2>
{
};

template<>
struct interrupt_type_helper<spi2, interrupt::error>
	: types::identity<mcutl::interrupt::type::spi2>
{
};

} //namespace detail
#endif //SPI2

#ifdef SPI3
//FYI: PA15 is JTDI, PB3 is JTDO, PB4 is NJTRST
using spi3 = mcutl::device::spi::spi_base<3, mcutl::periph::spi3, SPI3_BASE,
	gpio::gpiob<4>, gpio::gpiob<5>, gpio::gpiob<3>, gpio::gpioa<15>,
	mcutl::device::clock::device_source_id::spi3>;

namespace detail
{

template<> struct dma_channel_helper<spi3>
{
	using rx = dma::dma2<1>;
	using tx = dma::dma2<2>;
};

template<>
struct interrupt_type_helper<spi3, interrupt::recieve_buffer_not_empty>
	: types::identity<mcutl::interrupt::type::spi3>
{
};

template<>
struct interrupt_type_helper<spi3, interrupt::transmit_buffer_empty>
	: types::identity<mcutl::interrupt::type::spi3>
{
};

template<>
struct interrupt_type_helper<spi3, interrupt::error>
	: types::identity<mcutl::interrupt::type::spi3>
{
};

} //namespace detail
#endif //SPI3

namespace detail
{

template<uint8_t SpiIndex, typename Peripheral, uint32_t Base,
	typename MisoPin, typename MosiPin, typename SckPin, typename NssPin,
	mcutl::device::clock::device_source_id ClockId>
struct spi_traits<mcutl::device::spi::spi_base<SpiIndex, Peripheral, Base,
		MisoPin, MosiPin, SckPin, NssPin, ClockId>>
	: mcutl::device::spi::spi_base<SpiIndex, Peripheral, Base,
		MisoPin, MosiPin, SckPin, NssPin, ClockId>
{
};

} //namespace detail

template<typename... DmaOptions>
struct dma_transmit_mode
{
	template<typename Master>
	using channel = typename mcutl::spi::detail::dma_channel_helper<typename Master::spi_type>::tx;
	
	template<typename Master, typename InitInfoLambda>
	static constexpr auto prepare(InitInfoLambda options_lambda) noexcept
	{
		constexpr auto options = options_lambda();
		static_assert(!options.transmit_buffer_empty_set_count 
			|| options.transmit_buffer_empty.disable,
			"Transmit interrupt must be disabled when transmitting via DMA");
		
		auto options_modified = options;
		options_modified.spi_cr2 |= SPI_CR2_TXDMAEN;
		options_modified.spi_cr2_mask |= SPI_CR2_TXDMAEN_Msk | SPI_CR2_TXEIE_Msk;
		return options_modified;
	}
	
	template<typename Master, typename OptionsLambda>
	static inline void configure(OptionsLambda) MCUTL_NOEXCEPT
	{
		dma::configure_channel<channel<Master>,
			dma::source<dma::data_size::byte, dma::address::memory, dma::pointer_increment::enabled>,
			dma::destination<dma::data_size::byte, dma::address::peripheral, dma::pointer_increment::disabled>,
			DmaOptions...
		>();
	}
	
	template<typename Master, typename DataType>
	static inline void transmit(const DataType* data, uint16_t length) MCUTL_NOEXCEPT
	{
		dma::start_transfer<channel<Master>>(data, &mcutl::memory::volatile_memory<SPI_TypeDef,
			Master::spi_traits::base>()->DR, length);
	}
	
	template<typename Master>
	static void wait() MCUTL_NOEXCEPT
	{
		dma::wait_transfer<channel<Master>>();
		while (!mcutl::memory::get_register_bits<SPI_SR_TXE, &SPI_TypeDef::SR,
			Master::spi_traits::base>())
		{
		}
	}
	
	template<typename Master>
	static constexpr void clear_error_flags() noexcept
	{
	}
	
	template<typename Master, typename Data>
	static void set_data(Data) noexcept
	{
		static_assert(types::always_false<Master>::value,
			"Unable to set SPI data for DMA transmit mode");
	}
	
	template<typename Master, bool Enable>
	static void enable_source_increment() MCUTL_NOEXCEPT
	{
		dma::reconfigure_channel<channel<Master>,
			dma::source<dma::data_size::byte, dma::address::memory,
			std::conditional_t<Enable, dma::pointer_increment::enabled, dma::pointer_increment::disabled>>,
			dma::destination<dma::data_size::byte, dma::address::peripheral, dma::pointer_increment::disabled>
		>();
	}
};

template<typename... DmaOptions>
struct dma_receive_mode
{
	template<typename Master>
	using channel = typename mcutl::spi::detail::dma_channel_helper<typename Master::spi_type>::rx;
	
	template<typename Master, typename InitInfoLambda>
	static constexpr auto prepare(InitInfoLambda options_lambda) noexcept
	{
		constexpr auto options = options_lambda();
		static_assert(!options.recieve_buffer_not_empty_set_count 
			|| options.recieve_buffer_not_empty.disable,
			"Transmit interrupt must be disabled when transmitting via DMA");
		
		auto options_modified = options;
		options_modified.spi_cr2_mask |= SPI_CR2_RXNEIE_Msk;
		return options_modified;
	}
	
	template<typename Master, typename OptionsLambda>
	static inline void configure(OptionsLambda) MCUTL_NOEXCEPT
	{
		dma::configure_channel<channel<Master>,
			dma::source<dma::data_size::byte, dma::address::peripheral, dma::pointer_increment::disabled>,
			dma::destination<dma::data_size::byte, dma::address::memory, dma::pointer_increment::enabled>,
			DmaOptions...
		>();
	}
	
	template<typename Master, typename DataType>
	static void receive(DataType* data, uint16_t length) MCUTL_NOEXCEPT
	{
		mcutl::memory::set_register_bits<SPI_CR2_RXDMAEN_Msk, SPI_CR2_RXDMAEN,
			&SPI_TypeDef::CR2, Master::spi_traits::base>();
		dma::start_transfer<channel<Master>>(&mcutl::memory::volatile_memory<SPI_TypeDef,
			Master::spi_traits::base>()->DR, data, length);
	}
	
	template<typename Master>
	static void ignore_received_data() MCUTL_NOEXCEPT
	{
		mcutl::memory::set_register_bits<SPI_CR2_RXDMAEN_Msk, ~SPI_CR2_RXDMAEN,
			&SPI_TypeDef::CR2, Master::spi_traits::base>();
	}
	
	template<typename Master>
	static void wait() MCUTL_NOEXCEPT
	{
		if (mcutl::memory::get_register_bits<SPI_CR2_RXDMAEN, &SPI_TypeDef::CR2,
			Master::spi_traits::base>())
		{
			dma::wait_transfer<channel<Master>>();
		}
	}
	
	template<typename Master>
	static constexpr void clear_error_flags() noexcept
	{
	}
	
	template<typename Master>
	[[nodiscard]] static typename Master::data_type get_data() noexcept
	{
		static_assert(types::always_false<Master>::value,
			"Unable to get SPI data for DMA receive mode");
		return {};
	}
};

} //namespace mcutl::spi

namespace mcutl::device::spi
{

template<typename Derived>
class master
{
protected:
	template<typename OptionsLambda>
	static constexpr auto get_gpio_config(OptionsLambda options_lambda) noexcept
	{
		return Derived::spi_type::template get_gpio_config<Derived>(options_lambda);
	}
	
	using data_length_type = uint16_t;
	
private:
	static constexpr void process_interrupt(uint32_t set_count,
		const mcutl::interrupt::detail::interrupt_info& info,
		options& opts, uint32_t cr2_bits) noexcept
	{
		if (!set_count)
			return;
		
		opts.spi_cr2_mask |= cr2_bits;
		
		if (!info.disable)
			opts.spi_cr2 |= cr2_bits;
		
		if (info.priority != mcutl::interrupt::default_priority)
		{
			if (opts.interrupt_priority != mcutl::interrupt::default_priority
				&& opts.interrupt_priority != info.priority)
			{
				opts.priority_conflict = true;
			}
			
			if (opts.interrupt_subpriority != mcutl::interrupt::default_priority
				&& opts.interrupt_subpriority != info.priority
				&& info.priority != mcutl::interrupt::default_priority)
			{
				opts.priority_conflict = true;
			}
			
			opts.interrupt_priority = info.priority;
			opts.interrupt_subpriority = info.subpriority;
		}
	}
	
	template<typename OptionsLambda>
	static constexpr auto process_options(OptionsLambda options_lambda) noexcept
	{
		using namespace mcutl::spi::detail;
		auto options = options_lambda();
		options.spi_cr1 |= SPI_CR1_MSTR;
		options.spi_cr1_mask |= SPI_CR1_MSTR_Msk | SPI_CR1_DFF_Msk;
		
		using data_type = typename Derived::data_type;
		if constexpr (std::is_same_v<data_type, uint16_t>)
			options.spi_cr1 |= SPI_CR1_DFF;
		
		static_assert(std::is_same_v<data_type, uint8_t> || std::is_same_v<data_type, int8_t>
			|| std::is_same_v<data_type, uint16_t>|| std::is_same_v<data_type, std::byte>,
			"Unsupported SPI data type, only uint8_t, int8_t, uint16_t and std::byte are supported");
		
		if (!!options.slave_management_set_count)
		{
			options.spi_cr2_mask |= SPI_CR2_SSOE_Msk;
			options.spi_cr1_mask |= SPI_CR1_SSM_Msk;
			switch (options.slave_management)
			{
			case slave_management_type::hardware_with_output:
				options.spi_cr2 |= SPI_CR2_SSOE;
				break;
			
			case slave_management_type::hardware_without_output:
				break;
			
			default: //software or none
				options.spi_cr1 |= SPI_CR1_SSM | SPI_CR1_SSI;
				options.spi_cr1_mask |= SPI_CR1_SSI_Msk;
				break;
			}
		}
		
		if (!!options.frame_format_set_count)
		{
			options.spi_cr1_mask |= SPI_CR1_LSBFIRST_Msk;
			if (options.frame_format == frame_format_type::lsb_first)
				options.spi_cr1 |= SPI_CR1_LSBFIRST;
		}
		
		if (!!options.clock_polarity_set_count)
		{
			options.spi_cr1_mask |= SPI_CR1_CPOL_Msk;
			if (options.clock_polarity == clock_polarity_type::idle_1)
				options.spi_cr1 |= SPI_CR1_CPOL;
		}
		
		if (!!options.clock_phase_set_count)
		{
			options.spi_cr1_mask |= SPI_CR1_CPHA_Msk;
			if (options.clock_phase == clock_phase_type::capture_second_clock)
				options.spi_cr1 |= SPI_CR1_CPHA;
		}
		
		process_interrupt(options.error_set_count, options.error, options,
			SPI_CR2_ERRIE);
		process_interrupt(options.transmit_buffer_empty_set_count,
			options.transmit_buffer_empty, options, SPI_CR2_TXEIE);
		process_interrupt(options.recieve_buffer_not_empty_set_count,
			options.recieve_buffer_not_empty, options, SPI_CR2_RXNEIE);
		
		return options;
	}
	
protected:
	template<typename OptionsLambda>
	static constexpr auto prepare(OptionsLambda options_lambda) noexcept
	{
		constexpr auto options = process_options(options_lambda);
		static_assert(!options.priority_conflict, "SPI interrupt priority conflict");
		return options;
	}
	
	template<typename OptionsLambda>
	static void configure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
	{
		constexpr auto options = options_lambda();
		
		if constexpr (!!options.initialize_pins_set_count)
			mcutl::gpio::configure_gpio<decltype(get_gpio_config(options_lambda))>();
		
		if constexpr (!!options.clear_error_flags_set_count)
			clear_error_flags();
		
		mcutl::memory::set_register_bits<SPI_CR1_SPE_Msk, ~SPI_CR1_SPE,
			&SPI_TypeDef::CR1, Derived::spi_traits::base>();
		mcutl::memory::set_register_bits<0xffffffffu & ~SPI_CR1_BR_Msk,
			options.spi_cr1, &SPI_TypeDef::CR1, Derived::spi_traits::base>();
		mcutl::memory::set_register_value<options.spi_cr2,
			&SPI_TypeDef::CR2, Derived::spi_traits::base>();
		
		if constexpr (!!options.enable_controller_interrupts_set_count)
		{
			if constexpr ((options.spi_cr2 & (SPI_CR2_ERRIE | SPI_CR2_RXNEIE | SPI_CR2_TXEIE)) != 0)
			{
				mcutl::interrupt::enable<
					mcutl::interrupt::interrupt<
						mcutl::spi::interrupt_type<typename Derived::spi_type, mcutl::spi::interrupt::error>,
						options.interrupt_priority,
						options.interrupt_subpriority
					>,
					options.priority_count_set_count ? options.priority_count : mcutl::interrupt::maximum_priorities
				>();
			}
		}
		
		if constexpr (!!options.disable_controller_interrupts_set_count)
		{
			if constexpr (!(options.spi_cr2 & (SPI_CR2_ERRIE | SPI_CR2_RXNEIE | SPI_CR2_TXEIE)))
			{
				mcutl::interrupt::disable<
					mcutl::spi::interrupt_type<typename Derived::spi_type, mcutl::spi::interrupt::error>
				>();
			}
		}
	}
	
	static void enable() MCUTL_NOEXCEPT
	{
		mcutl::memory::set_register_bits<SPI_CR1_SPE_Msk, SPI_CR1_SPE,
			&SPI_TypeDef::CR1, Derived::spi_traits::base>();
	}
	
	static void disable() MCUTL_NOEXCEPT
	{
		mcutl::memory::set_register_bits<SPI_CR1_SPE_Msk, ~SPI_CR1_SPE,
			&SPI_TypeDef::CR1, Derived::spi_traits::base>();
	}
	
	static void wait() MCUTL_NOEXCEPT
	{
		while (mcutl::memory::get_register_bits<SPI_SR_BSY,
			&SPI_TypeDef::SR, Derived::spi_traits::base>())
		{
		}
	}
	
	static constexpr void transmit_prepare() noexcept
	{
	}
	
	static constexpr void transmit_receive_prepare() noexcept
	{
	}
	
	static void clear_error_flags() MCUTL_NOEXCEPT
	{
		auto cr1 = mcutl::memory::get_register_bits<&SPI_TypeDef::CR1, Derived::spi_traits::base>();
		//Clear OVR flag and UDR flag
		[[maybe_unused]] auto temp = mcutl::memory::get_register_bits<&SPI_TypeDef::DR,
			Derived::spi_type::base>();
		temp = mcutl::memory::get_register_bits<&SPI_TypeDef::SR, Derived::spi_traits::base>();
		//Clear CRCERR flag
		mcutl::memory::set_register_value<0u, &SPI_TypeDef::SR, Derived::spi_traits::base>();
		//Previous write to SR and the following write to CR1 will clear MODF, too
		mcutl::memory::set_register_value<&SPI_TypeDef::CR1, Derived::spi_traits::base>(cr1);
	}
};

template<typename FrequencyOption>
struct spi_prescaler_selector
{
	static constexpr uint32_t select(uint64_t) noexcept
	{
		static_assert(types::always_false<FrequencyOption>::value,
			"Unknown SPI frequency option");
		return 0;
	}
};

static constexpr std::array<uint32_t, 8> valid_prescalers {
	2, 4, 8, 16, 32, 64, 128, 256
};

template<uint64_t Frequency>
struct spi_prescaler_selector<mcutl::clock::max_frequency<Frequency>>
{
	static constexpr uint32_t select(uint64_t unscaled_frequency) noexcept
	{
		uint64_t best_frequency = 0;
		uint32_t best_prescaler = 0;
		for (size_t i = 0; i != valid_prescalers.size(); ++i)
		{
			uint64_t frequency = unscaled_frequency / valid_prescalers[i];
			if (frequency > best_frequency && frequency <= Frequency)
			{
				best_frequency = frequency;
				best_prescaler = valid_prescalers[i];
			}
		}
		return best_prescaler;
	}
};

template<uint64_t Frequency>
struct spi_prescaler_selector<mcutl::clock::min_frequency<Frequency>>
{
	static constexpr uint32_t select(uint64_t unscaled_frequency) noexcept
	{
		uint64_t best_frequency = 0;
		uint32_t best_prescaler = 0;
		for (size_t i = 0; i != valid_prescalers.size(); ++i)
		{
			uint64_t frequency = unscaled_frequency / valid_prescalers[i];
			if (frequency > best_frequency && frequency >= Frequency)
			{
				best_frequency = frequency;
				best_prescaler = valid_prescalers[i];
			}
		}
		return best_prescaler;
	}
};

template<uint64_t Frequency>
struct spi_prescaler_selector<mcutl::clock::required_frequency<Frequency>>
{
	static constexpr uint32_t select(uint64_t unscaled_frequency) noexcept
	{
		uint64_t best_prescaler = 0;
		for (size_t i = 0; i != valid_prescalers.size(); ++i)
		{
			uint64_t frequency = unscaled_frequency / valid_prescalers[i];
			if (frequency == Frequency)
			{
				best_prescaler = valid_prescalers[i];
				break;
			}
		}
		return best_prescaler;
	}
};

template<typename Interrupt>
struct interrupt_flag
{
	static_assert(types::always_false<Interrupt>::value,
		"Unknown SPI interrupt type");
};

template<>
struct interrupt_flag<mcutl::spi::interrupt::error>
{
	static constexpr uint32_t value = SPI_SR_OVR | SPI_SR_MODF | SPI_SR_CRCERR | SPI_SR_UDR;
};

template<>
struct interrupt_flag<mcutl::spi::interrupt::transmit_buffer_empty>
{
	static constexpr uint32_t value = SPI_SR_TXE;
};

template<>
struct interrupt_flag<mcutl::spi::interrupt::recieve_buffer_not_empty>
{
	static constexpr uint32_t value = SPI_SR_RXNE;
};

template<typename Spi, typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = (0u | ... | interrupt_flag<Interrupts>::value);

template<typename Spi, typename... Interrupts>
uint32_t get_pending_flags() MCUTL_NOEXCEPT
{
	constexpr auto base = mcutl::spi::detail::spi_traits<Spi>::base;
	return mcutl::memory::get_register_bits<pending_flags_v<Spi, Interrupts...>,
		&SPI_TypeDef::SR, base>();
}

template<typename Spi>
[[maybe_unused]] constexpr bool supports_software_slave_management = true;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_hardware_with_output_slave_management = true;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_hardware_without_output_slave_management = true;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_frame_format = true;
template<typename Spi>
[[maybe_unused]] constexpr bool supports_error_interrupt = true;

} //namespace mcutl::device::spi

namespace mcutl::spi
{

template<typename Spi, typename CurrentClockConfig, typename FrequencyOption>
void change_prescaler() MCUTL_NOEXCEPT
{
	constexpr auto unscaled_spi_frequency = clock::get_clock_info<
		CurrentClockConfig, detail::spi_traits<Spi>::clock_id>().get_unscaled_frequency();
	
	constexpr auto new_prescaler = device::spi::spi_prescaler_selector<FrequencyOption>
		::select(unscaled_spi_frequency);
	
	static_assert(new_prescaler != 0, "Unable to select prescaler with specified frequency requirements");
	
	constexpr auto new_prescaler_bits = device::clock::get_spi_prescaler_bits<
		new_prescaler>().prescaler_bits;
	
	device::clock::set_spi_prescaler<new_prescaler_bits, detail::spi_traits<Spi>::base>();
}

} //namespace mcutl::spi
