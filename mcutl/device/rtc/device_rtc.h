#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/rtc/stm32f1_rtc.h"
#else
static_assert(false, "Selected MCU does not support real-time clock");
#endif
