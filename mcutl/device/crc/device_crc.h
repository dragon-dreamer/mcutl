#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/crc/stm32f1_crc.h"
#else
static_assert(false, "Selected MCU does not support hardware CRC calculation");
#endif
