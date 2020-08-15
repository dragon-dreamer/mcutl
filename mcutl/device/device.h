#pragma once

#ifndef MCUTL_SMT32F1XX_INCLUDE_PATH
#	define MCUTL_SMT32F1XX_INCLUDE_PATH "mcutl/external/cmsis/device/st/stm32f1xx/stm32f1xx.h"
#endif //MCUTL_SMT32F1XX_INCLUDE_PATH

#if defined(STM32F101x6) || defined(STM32F101xB) || defined(STM32F101xE) || defined(STM32F101xG) \
	|| defined(STM32F102x6) || defined(STM32F102xB) || defined(STM32F103x6) || defined(STM32F103xB) \
	|| defined(STM32F103xE) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC)
#	include MCUTL_SMT32F1XX_INCLUDE_PATH
#else
static_assert(false, "Please set the supported MCU type");
#endif

#ifdef __CORTEX_M
#	if __CORTEX_M == 3
#		define MCUTL_CORTEX_M3 1
#	endif //__CORTEX_M == 3
#endif //__CORTEX_M
