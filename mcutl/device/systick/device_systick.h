#pragma once

#include "mcutl/device/device.h"

#ifdef MCUTL_CORTEX_M3
#	include "mcutl/device/systick/cortex_m3_systick.h"
#else
static_assert(false, "Selected MCU does not support SYSTICK");
#endif
