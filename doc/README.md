# General information

## Directories
MCUTL headers and some required external headers are located in the **mcutl** directory.
Directory | Purpose
--------- | -------
[**mcutl/clock**](clock.md) | MCU clock configuration
mcutl/device | Device-specific files. No need to use them directly, necessary files are included automatically
mcutl/external | Required external files (such as CMSIS or STM Device Peripheral Access Layer headers), included automatically
[mcutl/memory](memory.md) | MCU testable memory access layer
[**mcutl/periph**](periph.md) | MCU peripheral configuration
[**mcutl/gpio**](gpio.md) | MCU GPIO configuration
[mcutl/tests](tests.md) | Unit test layer (memory access and MCU-specific instructions mocks). Don't use directly, it's included automatically when required
mcutl/utils | Helper files for different purposes (constexpr math, type helpers etc)

Each of the directories in bold contains **<i>dirname</i>_defs.h** and **<i>dirname</i>.h** files. The first one usually defines some common types for the feature and is included by the second file. The second one contains both types and functions. You always need to include the second file to use the feature. The MCU headers that are automatically included when compiling may provide some additional types and functions (which are specific to the MCU of your choice).

## Usage
To use the library, just include the features you want to utilize in your project. For example, to configure MCU clocks, you need to `#include "mcutl/clock/clock.h"`. Don't forget to add the directory with the MCUTL library to your include paths list. You also need to add a define to your project to identify the MCU you use, so the library knowns which MCU you are compiling to. Currently supported defines are: **STM32F101x6, STM32F101xB, STM32F101xE, STM32F101xG, STM32F102x6, STM32F102xB, STM32F103x6, STM32F103xB, STM32F103xE, STM32F103xG, STM32F105xC, STM32F107xC**. The library will automatically include all additional necessary files (such as CMSIS and STM Device Peripheral Access Layer headers).

## Library defines
The library will compile when the define to identify the MCU it supports (see above) is supplied. However, you may want to further tweak the library behavior. The following configuration defines can be supplied:
Define | Meaning
------ | -------
MCUTL_SMT32F1XX_INCLUDE_PATH | Can be supplied to provide a path to the **stm32f1xx.h** header file. This define is used when compiling for STM32F101, STM32F102, STM32F103, STM32F105 and STM32F107 controllers. By default, the library uses its own bundled external files. You may want to use MCUTL with another library which itself uses CMSIS and STM Device Peripheral Access Layer (such as HAL or SPL), in this case you may need to set **MCUTL_SMT32F1XX_INCLUDE_PATH** to point to the **stm32f1xx.h** file from another common directory.
MCUTL_CORTEX_M3_BITBAND_DISABLE | Can be defined to disable automatic memory bit-banding for Cortex-M3 controllers. See [mcutl/memory](memory.md) for details.
MCUTL_TEST | Can be defined to enable host PC unit-testing. See [mcutl/tests](tests.md) for details.

## CMakeLists.txt
`CMakeLists.txt` and `build-mingw.bat` are provided exclusively to build library tests and to run them. As the library is header-only, the only target to build is `tests`. After building the `tests` target, you can execute `make test` to run all the tests and see the results.
