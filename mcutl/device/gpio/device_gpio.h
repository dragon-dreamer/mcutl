#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/gpio/stm32f1_gpio.h"
#else
static_assert(false, "Selected MCU does not support GPIO configuration");
#endif
