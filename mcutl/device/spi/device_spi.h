#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/spi/stm32f1_spi.h"
#else
static_assert(false, "Selected MCU does not support SPI");
#endif
