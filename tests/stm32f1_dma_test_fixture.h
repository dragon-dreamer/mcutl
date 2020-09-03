#pragma once

#include <stdint.h>

#include "mcutl/dma/dma.h"
#include "mcutl/tests/mcu.h"
#include "mcutl/memory/volatile_memory.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class dma_strict_test_fixture
	: virtual public mcutl::tests::mcu::strict_test_fixture_base
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
		expect_start_transfer(channel_base, direction,
			from_address, to_address, transfer_size);
	}
	
	void expect_start_transfer(uint32_t channel_base, uint32_t direction,
		const volatile void* from_address, volatile void* to_address, uint16_t transfer_size,
		const ::testing::Sequence& s1 = {},
		const ::testing::Sequence& s2 = {},
		const ::testing::Sequence& s3 = {})
	{
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
