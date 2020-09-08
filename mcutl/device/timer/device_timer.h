#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/timer/stm32f1_timer.h"
#else
static_assert(false, "Selected MCU does not support timers");
#endif
