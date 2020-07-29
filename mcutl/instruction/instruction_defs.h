#pragma once

#include "mcutl/utils/type_helpers.h"

namespace mcutl::instruction::detail
{

template<typename Instruction>
struct executor
{
	template<typename... Args>
	static constexpr void run(Args...) noexcept
	{
		static_assert(types::always_false<Instruction>::value,
			"Unknown MCU-specific instruction");
	}
};

template<typename Instruction>
struct instruction_type {};

template<typename Instruction>
struct return_type { using type = void; };

template<typename Instruction>
using return_type_t = typename return_type<Instruction>::type;

} //namespace mcutl::instruction::detail
