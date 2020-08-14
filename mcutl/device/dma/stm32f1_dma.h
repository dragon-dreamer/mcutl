#pragma once

#include <stdint.h>
#include <type_traits>

#include "mcutl/dma/dma_defs.h"
#include "mcutl/device/device.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/instruction/instruction.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::dma
{

template<uint32_t ChannelNumber>
using dma1 = channel<1u, ChannelNumber>;

#ifdef DMA2
template<uint32_t ChannelNumber>
using dma2 = channel<2u, ChannelNumber>;
#endif //DMA2

} //namespace mcutl::dma

namespace mcutl::dma::detail
{

template<uint32_t ChannelNumber>
struct dma_channel_validator<dma1<ChannelNumber>>
{
	static constexpr void validate() noexcept
	{
		static_assert(ChannelNumber && ChannelNumber <= 7, "Invalid DMA1 channel number");
	}
};

#ifdef DMA2
template<uint32_t ChannelNumber>
struct dma_channel_validator<dma2<ChannelNumber>>
{
	static constexpr void validate() noexcept
	{
		static_assert(ChannelNumber && ChannelNumber <= 5, "Invalid DMA2 channel number");
	}
};
#endif //DMA2

template<uint32_t DmaIndex, uint32_t ChannelNumber>
struct interrupt_helper
{
	static_assert(types::value_always_false<ChannelNumber>::value,
		"Unsupported DMA peripheral or channel number");
};

template<> struct interrupt_helper<1, 1> : types::identity<mcutl::interrupt::type::dma1_ch1> {};
template<> struct interrupt_helper<1, 2> : types::identity<mcutl::interrupt::type::dma1_ch2> {};
template<> struct interrupt_helper<1, 3> : types::identity<mcutl::interrupt::type::dma1_ch3> {};
template<> struct interrupt_helper<1, 4> : types::identity<mcutl::interrupt::type::dma1_ch4> {};
template<> struct interrupt_helper<1, 5> : types::identity<mcutl::interrupt::type::dma1_ch5> {};
template<> struct interrupt_helper<1, 6> : types::identity<mcutl::interrupt::type::dma1_ch6> {};
template<> struct interrupt_helper<1, 7> : types::identity<mcutl::interrupt::type::dma1_ch7> {};
template<> struct interrupt_helper<2, 1> : types::identity<mcutl::interrupt::type::dma2_ch1> {};
template<> struct interrupt_helper<2, 2> : types::identity<mcutl::interrupt::type::dma2_ch2> {};
template<> struct interrupt_helper<2, 3> : types::identity<mcutl::interrupt::type::dma2_ch3> {};

#if defined(STM32F105xC) || defined(STM32F107xC) //Connectivity
template<> struct interrupt_helper<2, 4> : types::identity<mcutl::interrupt::type::dma2_ch4> {};
template<> struct interrupt_helper<2, 5> : types::identity<mcutl::interrupt::type::dma2_ch5> {};
#else //Other devices
template<> struct interrupt_helper<2, 4> : types::identity<mcutl::interrupt::type::dma2_ch4_5> {};
template<> struct interrupt_helper<2, 5> : types::identity<mcutl::interrupt::type::dma2_ch4_5> {};
#endif

template<typename Interrupt, uint32_t DmaIndex, uint32_t ChannelNumber>
struct interrupt_type_helper<Interrupt, channel<DmaIndex, ChannelNumber>>
	: interrupt_helper<DmaIndex, ChannelNumber> {};

template<uint32_t DmaIndex, uint32_t ChannelNumber>
using interrupt_helper_t = typename interrupt_helper<DmaIndex, ChannelNumber>::type;

template<uint32_t DmaIndex>
struct dma_peripheral_helper
{
	static_assert(types::value_always_false<DmaIndex>::value,
		"Unsupported DMA peripheral");
};

template<> struct dma_peripheral_helper<1> : types::identity<mcutl::periph::dma1> {};
template<> struct dma_peripheral_helper<2> : types::identity<mcutl::periph::dma2> {};

template<uint32_t DmaIndex, uint32_t ChannelNumber>
struct peripheral_type_helper<channel<DmaIndex, ChannelNumber>>
	: dma_peripheral_helper<DmaIndex> {};

} //namespace mcutl::dma::detail

namespace mcutl::device::dma
{

using size_type = uint16_t;
[[maybe_unused]] constexpr bool supports_priority_levels = true;
[[maybe_unused]] constexpr bool supports_byte_transfer = true;
[[maybe_unused]] constexpr bool supports_halfword_transfer = true;
[[maybe_unused]] constexpr bool supports_word_transfer = true;
[[maybe_unused]] constexpr bool supports_circular_mode = true;
[[maybe_unused]] constexpr bool supports_memory_to_memory_transfer = true;
[[maybe_unused]] constexpr bool supports_memory_to_periph_transfer = true;
[[maybe_unused]] constexpr bool supports_periph_to_memory_transfer = true;
[[maybe_unused]] constexpr bool supports_periph_to_periph_transfer = true;
[[maybe_unused]] constexpr bool supports_transfer_complete_interrupt = true;
[[maybe_unused]] constexpr bool supports_half_transfer_interrupt = true;
[[maybe_unused]] constexpr bool supports_transfer_error_interrupt = true;
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags = true;

struct transfer_options : mcutl::dma::detail::transfer_options
{
};

template<uint32_t DmaIndex, uint32_t ChannelNumber>
struct channel_reg_mapping {};

template<> struct channel_reg_mapping<1, 1> : std::integral_constant<uint32_t, DMA1_Channel1_BASE> {};
template<> struct channel_reg_mapping<1, 2> : std::integral_constant<uint32_t, DMA1_Channel2_BASE> {};
template<> struct channel_reg_mapping<1, 3> : std::integral_constant<uint32_t, DMA1_Channel3_BASE> {};
template<> struct channel_reg_mapping<1, 4> : std::integral_constant<uint32_t, DMA1_Channel4_BASE> {};
template<> struct channel_reg_mapping<1, 5> : std::integral_constant<uint32_t, DMA1_Channel5_BASE> {};
template<> struct channel_reg_mapping<1, 6> : std::integral_constant<uint32_t, DMA1_Channel6_BASE> {};
template<> struct channel_reg_mapping<1, 7> : std::integral_constant<uint32_t, DMA1_Channel7_BASE> {};
#ifdef DMA2
template<> struct channel_reg_mapping<2, 1> : std::integral_constant<uint32_t, DMA2_Channel1_BASE> {};
template<> struct channel_reg_mapping<2, 2> : std::integral_constant<uint32_t, DMA2_Channel2_BASE> {};
template<> struct channel_reg_mapping<2, 3> : std::integral_constant<uint32_t, DMA2_Channel3_BASE> {};
template<> struct channel_reg_mapping<2, 4> : std::integral_constant<uint32_t, DMA2_Channel4_BASE> {};
template<> struct channel_reg_mapping<2, 5> : std::integral_constant<uint32_t, DMA2_Channel5_BASE> {};
#endif //DMA2

template<uint32_t DmaIndex, uint32_t ChannelNumber, uint32_t BitMask, uint32_t BitValues,
	__IO uint32_t DMA_Channel_TypeDef::*Reg>
void set_dma_register_bits() MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_bits<BitMask, BitValues, Reg,
		channel_reg_mapping<DmaIndex, ChannelNumber>::value>();
}

template<uint32_t DmaIndex, uint32_t ChannelNumber, uint32_t BitMask, 
	__IO uint32_t DMA_Channel_TypeDef::*Reg>
void set_dma_register_bits(uint32_t bit_values) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_bits<BitMask, Reg,
		channel_reg_mapping<DmaIndex, ChannelNumber>::value>(bit_values);
}

template<uint32_t DmaIndex, uint32_t ChannelNumber,
	__IO uint32_t DMA_Channel_TypeDef::*Reg>
uint32_t get_dma_register_bits() MCUTL_NOEXCEPT
{
	return mcutl::memory::get_register_bits<Reg,
		channel_reg_mapping<DmaIndex, ChannelNumber>::value>();
}

template<uint32_t DmaIndex, uint32_t ChannelNumber, uint32_t BitMask, uint32_t BitValues>
void set_dma_ccr_bits() MCUTL_NOEXCEPT
{
	set_dma_register_bits<DmaIndex, ChannelNumber, BitMask,
		BitValues, &DMA_Channel_TypeDef::CCR>();
}

template<uint32_t DmaIndex, uint32_t ChannelNumber>
void set_dma_cpar(uint32_t value) MCUTL_NOEXCEPT
{
	set_dma_register_bits<DmaIndex, ChannelNumber,
		mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CPAR>(value);
}

template<uint32_t DmaIndex, uint32_t ChannelNumber>
void set_dma_cmar(uint32_t value) MCUTL_NOEXCEPT
{
	set_dma_register_bits<DmaIndex, ChannelNumber,
		mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CMAR>(value);
}

template<uint32_t DmaIndex, uint32_t ChannelNumber>
void set_dma_cndtr(uint32_t value) MCUTL_NOEXCEPT
{
	set_dma_register_bits<DmaIndex, ChannelNumber,
		mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CNDTR>(value);
}

constexpr void calculate_msize(mcutl::dma::detail::data_size::value data_size,
	uint32_t& ccr) noexcept
{
	if (data_size == mcutl::dma::detail::data_size::halfword)
		ccr |= DMA_CCR_MSIZE_0;
	else if (data_size == mcutl::dma::detail::data_size::word)
		ccr |= DMA_CCR_MSIZE_1;
}

constexpr void calculate_psize(const mcutl::dma::detail::data_size::value data_size,
	uint32_t& ccr) noexcept
{
	if (data_size == mcutl::dma::detail::data_size::halfword)
		ccr |= DMA_CCR_PSIZE_0;
	else if (data_size == mcutl::dma::detail::data_size::word)
		ccr |= DMA_CCR_PSIZE_1;
}

struct dma_channel_info
{
	uint32_t ccr = 0;
	uint32_t ccr_mask = 0;
	mcutl::dma::detail::interrupt_info interrupt_info {};
};

template<bool FullConfig, typename OptionsLambda>
constexpr dma_channel_info get_channel_info(OptionsLambda opts_lambda) noexcept
{
	dma_channel_info result {};
	
	constexpr transfer_options opts = opts_lambda();
	
	if constexpr (FullConfig || opts.mode_set_count != 0)
	{
		result.ccr_mask |= DMA_CCR_CIRC_Msk;
	
		if constexpr (opts.mode_value == mcutl::dma::detail::mode::circular)
			result.ccr |= DMA_CCR_CIRC;
	}
	
	if constexpr (FullConfig || opts.priority_level_set_count != 0)
	{
		result.ccr_mask |= DMA_CCR_PL_Msk;
		
		if constexpr (opts.priority_level == mcutl::dma::detail::priority::medium)
			result.ccr |= DMA_CCR_PL_0;
		else if constexpr (opts.priority_level == mcutl::dma::detail::priority::high)
			result.ccr |= DMA_CCR_PL_1;
		else if constexpr (opts.priority_level == mcutl::dma::detail::priority::very_high)
			result.ccr |= DMA_CCR_PL_0 | DMA_CCR_PL_1;
	}
	
	if constexpr (opts.transfer_complete_set_count != 0)
	{
		result.ccr_mask |= DMA_CCR_TCIE_Msk;
		if constexpr (!opts.transfer_complete.disable)
		{
			result.ccr |= DMA_CCR_TCIE;
			result.interrupt_info = opts.transfer_complete;
		}
	}
	
	if constexpr (opts.half_transfer_set_count != 0)
	{
		result.ccr_mask |= DMA_CCR_HTIE_Msk;
		if constexpr (!opts.half_transfer.disable)
		{
			result.ccr |= DMA_CCR_HTIE;
			result.interrupt_info = opts.half_transfer;
		}
	}
	
	if constexpr (opts.transfer_error_set_count != 0)
	{
		result.ccr_mask |= DMA_CCR_TEIE_Msk;
		if constexpr (!opts.transfer_error.disable)
		{
			result.ccr |= DMA_CCR_TEIE;
			result.interrupt_info = opts.transfer_error;
		}
	}
	
	if constexpr (FullConfig || (opts.source_set_count && opts.destination_set_count))
	{
		result.ccr_mask |= DMA_CCR_DIR_Msk | DMA_CCR_MEM2MEM_Msk
			| DMA_CCR_MINC_Msk | DMA_CCR_PINC_Msk
			| DMA_CCR_MSIZE_Msk | DMA_CCR_PSIZE_Msk;
		if constexpr (opts.source.addr_type == mcutl::dma::detail::address_type::memory)
		{
			// mem to periph / mem to mem
			result.ccr |= DMA_CCR_DIR;
		
			if constexpr (opts.destination.addr_type == mcutl::dma::detail::address_type::memory)
				result.ccr |= DMA_CCR_MEM2MEM;
		
			if (opts.source.increment_mode == mcutl::dma::detail::pointer_increment_mode::enabled)
				result.ccr |= DMA_CCR_MINC;
		
			if (opts.destination.increment_mode == mcutl::dma::detail::pointer_increment_mode::enabled)
				result.ccr |= DMA_CCR_PINC;
		
			calculate_msize(opts.source.size, result.ccr);
			calculate_psize(opts.destination.size, result.ccr);
		}
		else
		{
			if (opts.source.increment_mode == mcutl::dma::detail::pointer_increment_mode::enabled)
				result.ccr |= DMA_CCR_PINC;
		
			if (opts.destination.increment_mode == mcutl::dma::detail::pointer_increment_mode::enabled)
				result.ccr |= DMA_CCR_MINC;
		
			calculate_msize(opts.destination.size, result.ccr);
			calculate_psize(opts.source.size, result.ccr);
		}
	}
	
	return result;
}

template<bool FullConfig, typename OptionsLambda>
constexpr auto get_validated_channel_info(OptionsLambda opts_lambda) noexcept
{
	constexpr auto channel_info = get_channel_info<FullConfig>(opts_lambda);
	constexpr auto opts = opts_lambda();
	static_assert(!opts.transfer_complete_set_count
		|| opts.transfer_complete.disable
		|| (opts.transfer_complete.priority == channel_info.interrupt_info.priority
			&& opts.transfer_complete.subpriority == channel_info.interrupt_info.subpriority),
		"Conflicting transfer complete interrupt priority or subpriority");
	static_assert(!opts.half_transfer_set_count
		|| opts.half_transfer.disable
		|| (opts.half_transfer.priority == channel_info.interrupt_info.priority
			&& opts.half_transfer.subpriority == channel_info.interrupt_info.subpriority),
		"Conflicting half transfer interrupt priority or subpriority");
	static_assert(!opts.transfer_error_set_count
		|| opts.transfer_error.disable
		|| (opts.transfer_error.priority == channel_info.interrupt_info.priority
			&& opts.transfer_error.subpriority == channel_info.interrupt_info.subpriority),
		"Conflicting transfer error interrupt priority or subpriority");
	return channel_info;
}

template<typename Channel, typename OptionsLambda>
void configure_dma(OptionsLambda opts_lambda) MCUTL_NOEXCEPT
{
	set_dma_ccr_bits<Channel::dma_index, Channel::channel_number,
		DMA_CCR_EN_Msk, static_cast<uint32_t>(~DMA_CCR_EN)>();
	
	constexpr auto channel_info = get_validated_channel_info<true>(opts_lambda);
	constexpr auto opts = opts_lambda();
	if constexpr (opts.enable_controller_interrupts_set_count != 0)
	{
		if constexpr ((channel_info.ccr
			& (DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE)) != 0)
		{
			mcutl::interrupt::enable<
				mcutl::interrupt::interrupt<
					mcutl::dma::detail::interrupt_helper_t<Channel::dma_index,
						Channel::channel_number
					>,
					channel_info.interrupt_info.priority,
					channel_info.interrupt_info.subpriority
				>,
				opts.priority_count_set_count ? opts.priority_count : mcutl::interrupt::maximum_priorities
			>();
		}
	}
	
	if constexpr (opts.disable_controller_interrupts_set_count != 0)
	{
		if constexpr ((channel_info.ccr
			& (DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE)) == 0)
		{
			mcutl::interrupt::disable_atomic<
				mcutl::dma::detail::interrupt_helper_t<Channel::dma_index,
					Channel::channel_number
				>
			>();
		}
	}
	
	set_dma_ccr_bits<Channel::dma_index, Channel::channel_number,
		mcutl::memory::max_bitmask<uint32_t>, channel_info.ccr>();
}

template<typename Channel, typename OptionsLambda>
void reconfigure_dma(OptionsLambda opts_lambda) MCUTL_NOEXCEPT
{
	uint32_t ccr = get_dma_register_bits<Channel::dma_index,
		Channel::channel_number, &DMA_Channel_TypeDef::CCR>();
	
	ccr &= ~DMA_CCR_EN;
	set_dma_register_bits<Channel::dma_index, Channel::channel_number,
		mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CCR>(ccr);
	
	constexpr auto channel_info = get_validated_channel_info<false>(opts_lambda);
	constexpr auto opts = opts_lambda();
	
	if constexpr (channel_info.ccr_mask != 0)
	{
		ccr &= ~channel_info.ccr_mask;
		ccr |= channel_info.ccr;
	}
	
	if constexpr (opts.enable_controller_interrupts_set_count != 0)
	{
		if ((ccr & (DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE)) != 0)
		{
			mcutl::interrupt::enable<
				mcutl::interrupt::interrupt<
					mcutl::dma::detail::interrupt_helper_t<Channel::dma_index,
						Channel::channel_number
					>,
					channel_info.interrupt_info.priority,
					channel_info.interrupt_info.subpriority
				>,
				opts.priority_count_set_count ? opts.priority_count : mcutl::interrupt::maximum_priorities
			>();
		}
	}
	
	if constexpr (opts.disable_controller_interrupts_set_count != 0)
	{
		if ((ccr & (DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE)) == 0)
		{
			mcutl::interrupt::disable_atomic<
				mcutl::dma::detail::interrupt_helper_t<Channel::dma_index,
					Channel::channel_number
				>
			>();
		}
	}
	
	if constexpr (channel_info.ccr_mask != 0)
	{
		set_dma_register_bits<Channel::dma_index, Channel::channel_number,
			mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CCR>(ccr);
	}
}

template<typename Channel>
void start_transfer(const volatile void* from, volatile void* to,
	size_type size) MCUTL_NOEXCEPT
{
	auto ccr = get_dma_register_bits<Channel::dma_index,
		Channel::channel_number, &DMA_Channel_TypeDef::CCR>();
	set_dma_register_bits<Channel::dma_index, Channel::channel_number,
		mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CCR>(ccr & ~DMA_CCR_EN);
	
	if (ccr & DMA_CCR_DIR) //read from memory
	{
		set_dma_cmar<Channel::dma_index, Channel::channel_number>(
			mcutl::memory::to_address(from));
		set_dma_cpar<Channel::dma_index, Channel::channel_number>(
			mcutl::memory::to_address(to));
	}
	else
	{
		set_dma_cpar<Channel::dma_index, Channel::channel_number>(
			mcutl::memory::to_address(from));
		set_dma_cmar<Channel::dma_index, Channel::channel_number>(
			mcutl::memory::to_address(to));
	}
	
	set_dma_cndtr<Channel::dma_index, Channel::channel_number>(size);
	
	mcutl::instruction::execute<mcutl::device::instruction::type::dmb>();
	set_dma_register_bits<Channel::dma_index, Channel::channel_number,
		mcutl::memory::max_bitmask<uint32_t>, &DMA_Channel_TypeDef::CCR>(ccr);
}

template<typename Channel>
void wait_transfer() MCUTL_NOEXCEPT
{
	if (get_dma_register_bits<Channel::dma_index,
		Channel::channel_number, &DMA_Channel_TypeDef::CCR>() & DMA_CCR_EN)
	{
		while (get_dma_register_bits<Channel::dma_index,
			Channel::channel_number, &DMA_Channel_TypeDef::CNDTR>())
		{
		}
	}
	
	mcutl::instruction::execute<mcutl::device::instruction::type::dmb>();
}

template<uint32_t ChannelNumber>
struct interrupt_flags {};

template<uint32_t TrasferComplete, uint32_t HalfTransfer, uint32_t TransferError,
	uint32_t GlobalFlag>
struct interrupt_flags_base
{
	static constexpr uint32_t transfer_complete = TrasferComplete;
	static constexpr uint32_t half_transfer = HalfTransfer;
	static constexpr uint32_t transfer_error = TransferError;
	static constexpr uint32_t global_flag = GlobalFlag;
};

template<> struct interrupt_flags<1> : interrupt_flags_base<
	DMA_IFCR_CTCIF1, DMA_IFCR_CHTIF1, DMA_IFCR_CTEIF1, DMA_IFCR_CGIF1> {};
template<> struct interrupt_flags<2> : interrupt_flags_base<
	DMA_IFCR_CTCIF2, DMA_IFCR_CHTIF2, DMA_IFCR_CTEIF2, DMA_IFCR_CGIF2> {};
template<> struct interrupt_flags<3> : interrupt_flags_base<
	DMA_IFCR_CTCIF3, DMA_IFCR_CHTIF3, DMA_IFCR_CTEIF3, DMA_IFCR_CGIF3> {};
template<> struct interrupt_flags<4> : interrupt_flags_base<
	DMA_IFCR_CTCIF4, DMA_IFCR_CHTIF4, DMA_IFCR_CTEIF4, DMA_IFCR_CGIF4> {};
template<> struct interrupt_flags<5> : interrupt_flags_base<
	DMA_IFCR_CTCIF5, DMA_IFCR_CHTIF5, DMA_IFCR_CTEIF5, DMA_IFCR_CGIF5> {};
template<> struct interrupt_flags<6> : interrupt_flags_base<
	DMA_IFCR_CTCIF6, DMA_IFCR_CHTIF6, DMA_IFCR_CTEIF6, DMA_IFCR_CGIF6> {};
template<> struct interrupt_flags<7> : interrupt_flags_base<
	DMA_IFCR_CTCIF7, DMA_IFCR_CHTIF7, DMA_IFCR_CTEIF7, DMA_IFCR_CGIF7> {};

template<uint32_t ChannelNumber, typename Interrupt>
struct interrupt_info
{
	static_assert(types::always_false<Interrupt>::value, "Unknown DMA interrupt");
	static constexpr uint32_t pending_flag = 0;
};

template<uint32_t ChannelNumber>
struct interrupt_info<ChannelNumber, mcutl::dma::interrupt::transfer_complete>
{
	static constexpr uint32_t pending_flag
		= interrupt_flags<ChannelNumber>::transfer_complete;
};

template<uint32_t ChannelNumber>
struct interrupt_info<ChannelNumber, mcutl::dma::interrupt::half_transfer>
{
	static constexpr uint32_t pending_flag
		= interrupt_flags<ChannelNumber>::half_transfer;
};

template<uint32_t ChannelNumber>
struct interrupt_info<ChannelNumber, mcutl::dma::interrupt::transfer_error>
{
	static constexpr uint32_t pending_flag
		= interrupt_flags<ChannelNumber>::transfer_error;
};

template<uint32_t ChannelNumber>
struct interrupt_info<ChannelNumber, mcutl::dma::interrupt::global>
{
	static constexpr uint32_t pending_flag
		= interrupt_flags<ChannelNumber>::global_flag;
};

template<uint32_t ChannelNumber, typename Interrupt,
	mcutl::interrupt::priority_t Priority,
	mcutl::interrupt::priority_t Subpriority>
struct interrupt_info<ChannelNumber,
	mcutl::interrupt::interrupt<Interrupt, Priority, Subpriority>>
{
	static constexpr uint32_t pending_flag
		= interrupt_info<ChannelNumber, Interrupt>::pending_flag;
};

template<typename Channel, typename... Interrupts>
void clear_pending_flags() MCUTL_NOEXCEPT
{
	constexpr auto flags = (0u | ... | interrupt_info<
		Channel::channel_number, Interrupts>::pending_flag);
	if constexpr (flags != 0)
	{
		if constexpr (Channel::dma_index == 1)
			mcutl::memory::set_register_value<flags, &DMA_TypeDef::IFCR, DMA1_BASE>();
	
#ifdef DMA2
		if constexpr (Channel::dma_index == 2)
			mcutl::memory::set_register_value<flags, &DMA_TypeDef::IFCR, DMA2_BASE>();
#endif //DMA2
	
		mcutl::instruction::execute<mcutl::device::instruction::type::dmb>();
	}
}

template<typename Channel, typename... Interrupts>
inline void clear_pending_flags_atomic() MCUTL_NOEXCEPT
{
	clear_pending_flags<Channel, Interrupts...>();
}

} //namespace mcutl::device::dma
