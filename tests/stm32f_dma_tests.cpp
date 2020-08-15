#define STM32F107xC
#define STM32F1

#include <stdint.h>
#include <type_traits>

#include "mcutl/dma/dma.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/periph/periph.h"
#include "mcutl/tests/mcu.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class dma_strict_test_fixture
	: public mcutl::tests::mcu::strict_test_fixture_base
{
public:
	void expect_configure(uint32_t channel_base, uint32_t ccr,
		uint32_t enable_interrupt, uint32_t interrupt_priority, uint32_t disable_interrupt,
		uint32_t initial_ccr = 0x12345678u | DMA_CCR_EN)
	{
		auto channel_ptr = mcutl::memory::volatile_memory<DMA_Channel_TypeDef>(channel_base);
		memory().set(addr(&channel_ptr->CCR), initial_ccr);
		memory().allow_reads(addr(&channel_ptr->CCR));
		
		::testing::InSequence s;
		EXPECT_CALL(memory(), write(addr(&channel_ptr->CCR), initial_ccr & ~DMA_CCR_EN));
		
		if (enable_interrupt)
		{
			if (interrupt_priority != mcutl::interrupt::default_priority)
			{
				EXPECT_CALL(memory(), write(addr(&(NVIC->IP[enable_interrupt])),
					interrupt_priority << 4));
				EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
					::testing::IsEmpty()));
			}
	
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
				::testing::IsEmpty()));
			EXPECT_CALL(memory(), write(addr(&(NVIC->ISER[enable_interrupt >> 5u])),
				1u << (enable_interrupt & 0x1Fu)));
		}
		
		if (disable_interrupt)
		{
			EXPECT_CALL(memory(), write(addr(&(NVIC->ICER[disable_interrupt >> 5u])),
				1u << (disable_interrupt & 0x1Fu)));
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dsb>(),
				::testing::IsEmpty()));
			EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::isb>(),
				::testing::IsEmpty()));
		}
		
		if (ccr != initial_ccr && ccr != (initial_ccr & ~DMA_CCR_EN))
			EXPECT_CALL(memory(), write(addr(&channel_ptr->CCR), ccr));
	}
	
	void expect_start_transfer(uint32_t channel_base, uint32_t direction)
	{
		::testing::Sequence s1, s2, s3;
		
		auto channel_ptr = mcutl::memory::volatile_memory<DMA_Channel_TypeDef>(channel_base);
		
		uint32_t initial_ccr = DMA_CCR_EN | direction;
		memory().set(addr(&channel_ptr->CCR), initial_ccr);
		memory().allow_reads(addr(&channel_ptr->CCR));
		EXPECT_CALL(memory(), write(addr(&channel_ptr->CCR), initial_ccr & ~DMA_CCR_EN))
			.InSequence(s1, s2, s3);
		
		if (direction == DMA_CCR_DIR) //mem to periph
		{
			EXPECT_CALL(memory(), write(addr(&channel_ptr->CMAR),
				mcutl::memory::to_address(from_address)))
				.InSequence(s1);
			EXPECT_CALL(memory(), write(addr(&channel_ptr->CPAR),
				mcutl::memory::to_address(to_address)))
				.InSequence(s2);
		}
		else
		{
			EXPECT_CALL(memory(), write(addr(&channel_ptr->CMAR),
				mcutl::memory::to_address(to_address)))
				.InSequence(s1);
			EXPECT_CALL(memory(), write(addr(&channel_ptr->CPAR),
				mcutl::memory::to_address(from_address)))
				.InSequence(s2);
		}
		
		EXPECT_CALL(memory(), write(addr(&channel_ptr->CNDTR), transfer_size))
			.InSequence(s3);
		EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
			::testing::IsEmpty()))
			.InSequence(s1, s2, s3);
		EXPECT_CALL(memory(), write(addr(&channel_ptr->CCR), initial_ccr))
			.InSequence(s1, s2, s3);
	}
	
public:
	const void* from_address = reinterpret_cast<const void*>(0x12345678);
	void* to_address = reinterpret_cast<void*>(0x89abcdef);
	static constexpr uint16_t transfer_size = 0x1234;
};

TEST_F(dma_strict_test_fixture, TraitsTest)
{
	EXPECT_TRUE(mcutl::dma::supports_priority_levels);
	EXPECT_TRUE(mcutl::dma::supports_byte_transfer);
	EXPECT_TRUE(mcutl::dma::supports_halfword_transfer);
	EXPECT_TRUE(mcutl::dma::supports_word_transfer);
	EXPECT_TRUE(mcutl::dma::supports_circular_mode);
	EXPECT_TRUE(mcutl::dma::supports_memory_to_memory_transfer);
	EXPECT_TRUE(mcutl::dma::supports_memory_to_periph_transfer);
	EXPECT_TRUE(mcutl::dma::supports_periph_to_memory_transfer);
	EXPECT_TRUE(mcutl::dma::supports_periph_to_periph_transfer);
	EXPECT_TRUE(mcutl::dma::supports_transfer_complete_interrupt);
	EXPECT_TRUE(mcutl::dma::supports_half_transfer_interrupt);
	EXPECT_TRUE(mcutl::dma::supports_transfer_error_interrupt);
	EXPECT_TRUE(mcutl::dma::supports_atomic_clear_pending_flags);
	
	EXPECT_TRUE((std::is_same_v<mcutl::dma::peripheral_type<mcutl::dma::dma1<3>>,
		mcutl::periph::dma1>));
	EXPECT_TRUE((std::is_same_v<mcutl::dma::peripheral_type<mcutl::dma::dma2<5>>,
		mcutl::periph::dma2>));
	
	EXPECT_TRUE((std::is_same_v<mcutl::dma::size_type, uint16_t>));
	
	EXPECT_TRUE((std::is_same_v<mcutl::dma::interrupt_type<mcutl::dma::interrupt::half_transfer,
		mcutl::dma::dma1<3>>, mcutl::interrupt::type::dma1_ch3>));
	EXPECT_TRUE((std::is_same_v<mcutl::dma::interrupt_type<mcutl::dma::interrupt::transfer_complete,
		mcutl::dma::dma1<7>>, mcutl::interrupt::type::dma1_ch7>));
	EXPECT_TRUE((std::is_same_v<mcutl::dma::interrupt_type<mcutl::dma::interrupt::transfer_error,
		mcutl::dma::dma1<1>>, mcutl::interrupt::type::dma1_ch1>));
	EXPECT_TRUE((std::is_same_v<mcutl::dma::interrupt_type<mcutl::dma::interrupt::half_transfer,
		mcutl::dma::dma2<2>>, mcutl::interrupt::type::dma2_ch2>));
	EXPECT_TRUE((std::is_same_v<mcutl::dma::interrupt_type<mcutl::dma::interrupt::transfer_complete,
		mcutl::dma::dma2<5>>, mcutl::interrupt::type::dma2_ch5>));
	EXPECT_TRUE((std::is_same_v<mcutl::dma::interrupt_type<mcutl::dma::interrupt::transfer_error,
		mcutl::dma::dma2<4>>, mcutl::interrupt::type::dma2_ch4>));
}

TEST_F(dma_strict_test_fixture, StartTransferPeriphToMemTest)
{
	expect_start_transfer(DMA1_Channel5_BASE, 0);
	mcutl::dma::start_transfer<mcutl::dma::dma1<5>>(from_address, to_address, transfer_size);
}

TEST_F(dma_strict_test_fixture, StartTransferMemToPeriphTest)
{
	expect_start_transfer(DMA2_Channel3_BASE, DMA_CCR_DIR);
	mcutl::dma::start_transfer<mcutl::dma::dma2<3>>(from_address, to_address, transfer_size);
}

TEST_F(dma_strict_test_fixture, WaitTransferEnabledTest)
{
	uint32_t initial_ccr = DMA_CCR_EN | 0x12345678u;
	memory().set(addr(&DMA1_Channel7->CCR), initial_ccr);
	memory().allow_reads(addr(&DMA1_Channel7->CCR));
	memory().set(addr(&DMA1_Channel7->CNDTR), 10u);
	
	::testing::InSequence s;
	EXPECT_CALL(memory(), read(addr(&DMA1_Channel7->CNDTR)))
		.Times(10)
		.WillRepeatedly([this] (auto) {
			return --memory().get(addr(&DMA1_Channel7->CNDTR));
		});
	
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	
	mcutl::dma::wait_transfer<mcutl::dma::dma1<7>>();
}

TEST_F(dma_strict_test_fixture, WaitTransferDisabledTest)
{
	uint32_t initial_ccr = 0x12345678u & ~DMA_CCR_EN;
	memory().set(addr(&DMA1_Channel7->CCR), initial_ccr);
	
	::testing::InSequence s;
	memory().allow_reads(addr(&DMA1_Channel7->CCR));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	
	mcutl::dma::wait_transfer<mcutl::dma::dma1<7>>();
}

TEST_F(dma_strict_test_fixture, ClearPendingEmptyFlagsTest)
{
	mcutl::dma::clear_pending_flags<mcutl::dma::dma2<3>>();
}

TEST_F(dma_strict_test_fixture, ClearPendingFlagsMask1Test)
{
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&DMA2->IFCR), DMA_IFCR_CTCIF3 | DMA_IFCR_CHTIF3));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	
	mcutl::dma::clear_pending_flags<mcutl::dma::dma2<3>,
		mcutl::dma::interrupt::transfer_complete,
		mcutl::dma::interrupt::half_transfer>();
}

TEST_F(dma_strict_test_fixture, ClearPendingFlagsMask2Test)
{
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&DMA1->IFCR), DMA_IFCR_CGIF4));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	
	mcutl::dma::clear_pending_flags<mcutl::dma::dma1<4>,
		mcutl::dma::interrupt::global>();
}

TEST_F(dma_strict_test_fixture, ClearPendingFlagsMask3Test)
{
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&DMA2->IFCR), DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	
	mcutl::dma::clear_pending_flags<mcutl::dma::dma2<1>,
		mcutl::dma::interrupt::half_transfer,
		mcutl::dma::interrupt::transfer_error>();
}

TEST_F(dma_strict_test_fixture, ClearPendingFlagsMaskAtomicTest)
{
	::testing::InSequence s;
	EXPECT_CALL(memory(), write(addr(&DMA1->IFCR),
		DMA_IFCR_CHTIF6 | DMA_IFCR_CTEIF6 | DMA_IFCR_CTCIF6));
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::dmb>(),
		::testing::IsEmpty()));
	
	mcutl::dma::clear_pending_flags_atomic<mcutl::dma::dma1<6>,
		mcutl::dma::interrupt::half_transfer,
		mcutl::dma::interrupt::transfer_error,
		mcutl::dma::interrupt::transfer_complete>();
}

TEST_F(dma_strict_test_fixture, ComplexConfigureEnableInterruptTest)
{
	expect_configure(DMA2_Channel3_BASE, DMA_CCR_DIR | DMA_CCR_PL_1 | DMA_CCR_TCIE
		| DMA_CCR_TEIE | DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_1,
		DMA2_Channel3_IRQn, 3u, 0);
	mcutl::dma::configure_channel<mcutl::dma::dma2<3>,
		mcutl::dma::priority::high,
		mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_complete, 3>,
		mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_error, 3>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::interrupt::disabled<mcutl::dma::interrupt::half_transfer>,
		mcutl::dma::interrupt::disable_controller_interrupts,
		mcutl::dma::source<mcutl::dma::data_size::halfword, mcutl::dma::address::memory,
			mcutl::dma::pointer_increment::enabled>,
		mcutl::dma::destination<mcutl::dma::data_size::word, mcutl::dma::address::peripheral,
			mcutl::dma::pointer_increment::disabled>
	>();
}

TEST_F(dma_strict_test_fixture, ComplexConfigureDisableInterruptTest)
{
	expect_configure(DMA1_Channel1_BASE, DMA_CCR_DIR | DMA_CCR_PL_0 | DMA_CCR_PL_1
		| DMA_CCR_PINC | DMA_CCR_PSIZE_0 | DMA_CCR_MEM2MEM,
		0, 0, DMA1_Channel1_IRQn);
	mcutl::dma::configure_channel<mcutl::dma::dma1<1>,
		mcutl::dma::priority::very_high,
		mcutl::dma::interrupt::disable_controller_interrupts,
		mcutl::dma::source<mcutl::dma::data_size::byte, mcutl::dma::address::memory,
			mcutl::dma::pointer_increment::disabled>,
		mcutl::dma::destination<mcutl::dma::data_size::halfword, mcutl::dma::address::memory,
			mcutl::dma::pointer_increment::enabled>
	>();
}

TEST_F(dma_strict_test_fixture, ComplexConfigureTest)
{
	expect_configure(DMA1_Channel2_BASE, DMA_CCR_CIRC | DMA_CCR_HTIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1,
		0, 0, 0);
	mcutl::dma::configure_channel<mcutl::dma::dma1<2>,
		mcutl::dma::interrupt::half_transfer,
		mcutl::dma::mode::circular,
		mcutl::dma::source<mcutl::dma::data_size::word, mcutl::dma::address::peripheral,
			mcutl::dma::pointer_increment::disabled>,
		mcutl::dma::destination<mcutl::dma::data_size::word, mcutl::dma::address::memory,
			mcutl::dma::pointer_increment::enabled>
	>();
}

TEST_F(dma_strict_test_fixture, ComplexReConfigureTest)
{
	uint32_t initial_ccr = DMA_CCR_CIRC | DMA_CCR_TCIE | DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1 | DMA_CCR_EN;
	uint32_t new_ccr = DMA_CCR_TEIE | DMA_CCR_MINC | DMA_CCR_MSIZE_0
		| DMA_CCR_HTIE;
	
	expect_configure(DMA1_Channel5_BASE, new_ccr, 0, 0, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma1<5>,
		mcutl::dma::interrupt::half_transfer,
		mcutl::interrupt::disabled<mcutl::dma::interrupt::transfer_complete>,
		mcutl::dma::mode::normal,
		mcutl::dma::source<mcutl::dma::data_size::byte, mcutl::dma::address::peripheral,
			mcutl::dma::pointer_increment::disabled>,
		mcutl::dma::destination<mcutl::dma::data_size::halfword, mcutl::dma::address::memory,
			mcutl::dma::pointer_increment::enabled>
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureModePriorityOnlyTest)
{
	uint32_t initial_ccr = DMA_CCR_TCIE | DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1 | DMA_CCR_EN;
	uint32_t new_ccr = (initial_ccr | DMA_CCR_CIRC
		| DMA_CCR_PL_0 | DMA_CCR_PL_1) & ~DMA_CCR_EN;
	
	expect_configure(DMA2_Channel2_BASE, new_ccr, 0, 0, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma2<2>,
		mcutl::dma::mode::circular,
		mcutl::dma::priority::very_high
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureInterruptEnableDisableTest1)
{
	uint32_t initial_ccr = DMA_CCR_TCIE | DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
	uint32_t new_ccr = initial_ccr;
	
	expect_configure(DMA2_Channel3_BASE, new_ccr, DMA2_Channel3_IRQn,
		mcutl::interrupt::default_priority, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma2<3>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::dma::interrupt::disable_controller_interrupts
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureInterruptEnableDisableTest2)
{
	uint32_t initial_ccr = DMA_CCR_TCIE | DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
	uint32_t new_ccr = initial_ccr & ~DMA_CCR_TCIE;
	
	expect_configure(DMA2_Channel4_BASE, new_ccr, DMA2_Channel4_IRQn,
		mcutl::interrupt::default_priority, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma2<4>,
		mcutl::interrupt::disabled<mcutl::dma::interrupt::transfer_complete>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::dma::interrupt::disable_controller_interrupts
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureInterruptEnableDisableTest3)
{
	uint32_t initial_ccr = DMA_CCR_TCIE | DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
	uint32_t new_ccr = initial_ccr & ~(DMA_CCR_TCIE | DMA_CCR_TEIE);
	
	expect_configure(DMA2_Channel5_BASE, new_ccr, 0, 0, DMA2_Channel5_IRQn, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma2<5>,
		mcutl::interrupt::disabled<mcutl::dma::interrupt::transfer_complete>,
		mcutl::interrupt::disabled<mcutl::dma::interrupt::transfer_error>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::dma::interrupt::disable_controller_interrupts
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureInterruptEnableDisableTest4)
{
	uint32_t initial_ccr = DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
	uint32_t new_ccr = initial_ccr;
	
	expect_configure(DMA1_Channel6_BASE, new_ccr, 0, 0, DMA1_Channel6_IRQn, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma1<6>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::dma::interrupt::disable_controller_interrupts
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureInterruptEnableDisableTest5)
{
	uint32_t initial_ccr = DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
	uint32_t new_ccr = initial_ccr | DMA_CCR_TCIE;
	
	expect_configure(DMA1_Channel7_BASE, new_ccr, DMA1_Channel7_IRQn, 5, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma1<7>,
		mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_complete, 5>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::dma::interrupt::disable_controller_interrupts
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureSourceIncrementOnlyTest)
{
	uint32_t initial_ccr = DMA_CCR_DIR | DMA_CCR_TCIE | DMA_CCR_TEIE;
	uint32_t new_ccr = initial_ccr | DMA_CCR_MINC;
	
	expect_configure(DMA1_Channel4_BASE, new_ccr, 0, 0, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma1<4>,
		mcutl::dma::source<mcutl::dma::data_size::byte, mcutl::dma::address::memory,
			mcutl::dma::pointer_increment::enabled>,
		mcutl::dma::destination<mcutl::dma::data_size::byte, mcutl::dma::address::peripheral,
			mcutl::dma::pointer_increment::disabled>
	>();
}

TEST_F(dma_strict_test_fixture, ReConfigureInterruptEnableWithPrioritiesTest)
{
	uint32_t initial_ccr = DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
	uint32_t new_ccr = initial_ccr | DMA_CCR_TCIE;
	
	expect_configure(DMA2_Channel5_BASE, new_ccr, DMA2_Channel5_IRQn,
		3 << 1, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma2<5>,
		mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_complete, 3>,
		mcutl::dma::interrupt::enable_controller_interrupts,
		mcutl::interrupt::priority_count<8>
	>();
}

TEST_F(dma_strict_test_fixture, DisableTransferTest)
{
	uint32_t initial_ccr = DMA_CCR_TEIE
		| DMA_CCR_MINC | DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1 | DMA_CCR_EN;
	uint32_t new_ccr = initial_ccr  & ~DMA_CCR_EN;
	
	expect_configure(DMA2_Channel5_BASE, new_ccr, 0, 0, 0, initial_ccr);
	mcutl::dma::reconfigure_channel<mcutl::dma::dma2<5>>();
}
