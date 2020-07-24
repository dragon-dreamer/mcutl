# Microcontroller C++ template library (MCUTL)

## What's this?
This is a modern C++17 header-only template library for microcontrollers. The basic idea is to provide access to microcontroller features and peripherals via a uniform interface and precompute as much as possible during compilation. Bonus: it supports unit-testing!

As this is my hobby project, it currently supports some features of Cortex-M3 STM32F101, STM32F102, STM32F103 and partially STM32F104 and STM32F105 controllers only, but I may add more in the future.

## MCUTL features compared to any other MCU library
**MCUTL** | Any other MCU library
--------- | -----------------------
**Header-only** | Requires compilation
**Compiles and includes to binary *only* the code which is required to do what you ask** | Compiles full-blown bodies of functions with branches for every occasion, which leads to larger and slower code
**Performs compile-time checks so you aren't able to compile ridiculous code (like settings incorrect clock prescalers)** | You can easily pass some invalid arguments to functions, most of them are not checked
**Precomputes everything that's possible, minimizing code size and increasing its speed** | Precomputes a few lines of code using macros, but most of calculations are run-time
**Unit-testable on the host PC, provides mocks for memory reads and writes and MCU-specific instructions. You can test both the library and the code which utilizes it** | Well, no, you have to mock everything yourself
Heavily relies on compiler optimizations, debug builds may be huge | **Doesn't depend on compiler optimizations that much**
Requires C++17 | **Nope**

## Compiler support
Currently supported and tested compilers are **G++ 8.4 and newer** and **Clang 7.0 and newer**.

## Sweet example
Suppose you'd like to configure clocks of your STM32F103xxxx microcontroller. With MCUTL you can do this:
```cpp
// Use external 16 MHz crystal, set core frequency to 72 MHz,
// set some requirements for SPI1 frequency, and also request valid
// USB peripheral frequency (which should be 48 MHz for STM32F103).
using clock_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::core<mcutl::clock::required_frequency<72_MHz>>,
	mcutl::clock::spi1<mcutl::clock::min_frequency<100_KHz>, mcutl::clock::max_frequency<200_KHz>>,
	mcutl::clock::provide_usb_frequency,
	mcutl::clock::base_configuration_is_currently_present
>;

mcutl::clock::configure_clocks<clock_config>();
```

This will:
* Check all the prerequisites and display a readable compile-time error if it's not possible to configure MCU clocks in a way requested. For example, if it's not possible to configure the core to run with 72 MHz frequency with SPI1 from 100 to 200 KHz frequency and USB peripheral enabled at the same time, you'll know this when compiling.
* Compile the code to perform actions exactly required to achieve the configuration requested. No excessive checks, no code that's not needed. You don't need to generate the configuration code using STM32CubeMX, you just get what you see.
* Bonus: if you later wish to change your 16 MHz crystal to a 12 MHz one, you just change a single character in your configuration and recompile the code!
