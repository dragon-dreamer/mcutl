#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/exti/stm32f1_exti.h"
#else
static_assert(false, "Selected MCU does not support external interrupts");
#endif
