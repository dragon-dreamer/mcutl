#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/low_power/stm32f1_low_power.h"
#else
static_assert(false, "Selected MCU does not support low power modes");
#endif
