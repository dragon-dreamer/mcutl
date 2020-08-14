#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/dma/stm32f1_dma.h"
#else
static_assert(false, "Selected MCU does not support DMA");
#endif
