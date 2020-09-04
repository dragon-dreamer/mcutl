#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/adc/stm32f1_adc.h"
#else
static_assert(false, "Selected MCU does not support ADC");
#endif
