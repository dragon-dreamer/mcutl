# MCU peripheral configuration (mcutl/periph/periph.h)
This header provides facilities to configure MCU peripherals. You can enable, disable, reset and reset-clear the peripherals supported by the MCU of your choice. For configuration, `mcutl::periph::configure_peripheral` function is used. It can take any number of template arguments, which define the peripheral configuration to perform. There are four configuration options available: `enable`, `disable`, `reset` and `undo_reset`, which enable, disable, reset a peripheral or clear the reset flag for a peripheral, respectively. All of the definitions are in the `mcutl::periph` namespace, unless otherwise stated.

Here are several self-explanatory examples of peripheral configuration:
```cpp
mcutl::periph::configure_peripheral<
	mcutl::periph::enable<mcutl::periph::adc1>,
	mcutl::periph::enable<mcutl::periph::gpiob>,
	mcutl::periph::enable<mcutl::periph::dma1>,
	mcutl::periph::reset<mcutl::periph::dma1>,
	mcutl::periph::disable<mcutl::periph::dma2>
>();
```
This code enables the `adc1`, `gpiob`, `dma1` peripherals, resets the `dma1` peripheral and disables the `dma2` peripheral. It is beneficial to specify as many peripherals as possible in a single `configure_peripheral` call, as this call combines all the register accesses and generates as short and as fast code as possible.

The second example uses the pre-defined configuration:
```cpp
using periph_config = mcutl::periph::config<
	mcutl::periph::enable<mcutl::periph::gpioa>,
	mcutl::periph::enable<mcutl::periph::gpiob>,
	mcutl::periph::enable<mcutl::periph::gpioc>
>;

mcutl::periph::configure_peripheral<periph_config>();
```

There is a special peripheral `mcutl::periph::no_periph`, which is a no-op and can be passed to `enable`, `disable`, `reset` and `undo_reset` functions.

You can also use the `mcutl::types::list` template (defined in `mcutl/utils/type_helpers.h`) to specify a list of peripherals to configure in a similar fashion, for example:
```cpp
mcutl::periph::configure_peripheral<
	mcutl::periph::enable<mcutl::types::list<
		mcutl::periph::adc1, mcutl::periph::gpiob, mcutl::periph::dma1
	>>,
	mcutl::periph::disable<mcutl::periph::dma2>
>();
```

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 peripherals
Here is the list of the peripherals that may be present on these MCUs:
* ADC: `adc1`, `adc2`, `adc3`
* AFIO: `afio`
* BKP: `bkp`
* CAN: `can1`, `can2`
* CRC: `crc`
* DAC: `dac`
* DMA: `dma1`, `dma2`
* Ethernet: `ethmacrx`, `ethmactx`, `ethmac`
* FLIFT: `flitf`
* FSMC: `fsmc`
* GPIO: `gpioa`, `gpiob`, `gpioc`, `gpiod`, `gpioe`, `gpiof`, `gpiog`
* I2C: `i2c1`, `i2c2`
* PWR: `pwr`
* SDIO: `sdio`
* SRAM: `sram`
* SPI: `spi1`, `spi2`, `spi3`
* TIMER: `timer1`, `timer2`, `timer3`, `timer4`, `timer5`, `timer6`, `timer7`, `timer8`, `timer9`, `timer10`, `timer11`, `timer12`, `timer13`, `timer14`
* UART: `usart1`, `usart2`, `usart3`, `uart4`, `uart5`
* USB: `otgfs`, `usb`
* Window watchdog: `wwdg`