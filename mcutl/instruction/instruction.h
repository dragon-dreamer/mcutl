#pragma once

#include "mcutl/device/device.h"
#include "mcutl/device/instruction/instruction_types.h"
#ifdef MCUTL_TEST
#	include "mcutl/tests/instruction.h"
#else //MCUTL_TEST
#	include "mcutl/device/instruction/instruction.h"
#endif //MCUTL_TEST
#include "mcutl/instruction/instruction_defs.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::instruction
{

template<typename Instruction, typename... Args>
MCUTL_STATIC_FORCEINLINE auto execute(Args... args) MCUTL_NOEXCEPT
{
	return detail::executor<detail::instruction_type<Instruction>>::run(args...);
}

} //namespace mcutl::instruction
