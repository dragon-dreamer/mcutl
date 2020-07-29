#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/instruction/stm32_instruction_types.h"
#else
static_assert(false, "Selected MCU does not support MCU-specific instructions");
#endif
