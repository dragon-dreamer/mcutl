#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/interrupt/stm32f1_interrupt.h"
#else
static_assert(false, "Selected MCU does not support interrupt configuration");
#endif
