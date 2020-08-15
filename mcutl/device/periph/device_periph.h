#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/periph/stm32_periph.h"
#else
static_assert(false, "Selected MCU does not support peripheral configuration");
#endif
