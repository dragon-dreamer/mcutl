#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/clock/stm32f1_clock.h"
#else
static_assert(false, "Selected MCU does not support clock configuration");
#endif
