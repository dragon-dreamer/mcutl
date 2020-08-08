#pragma once

#include "mcutl/device/device.h"

#ifdef STM32F1
#	include "mcutl/device/backup/stm32f1_backup.h"
#else
static_assert(false, "Selected MCU does not support backup domain");
#endif
