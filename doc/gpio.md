# MCU GPIO configuration (mcutl/gpio/gpio.h)
This header provides facilities to configure MCU general purpose input/output ports (GPIO). You can configure GPIO, read and write values and check existing configurations.

### Public definitions
```cpp
template<char PortLetter, uint32_t PinNumber>
struct pin;
```
This is the base structure of a MCU GPIO pin. Device-specific definitions use this struct to define available GPIOs (see below).

### Output GPIO modes
There are four available common output modes:
* `out::push_pull` - configure GPIO as a push-pull output
* `out::open_drain` - configure GPIO as an open drain output
* `out::push_pull_alt_func` - configure GPIO as a push-pull, alternative function output
* `out::open_drain_alt_func` - configure GPIO as an open drain, alternative function output

Some of these options may not be supported by the MCU of your choice. Some MCUs may provide additional modes.

### Output GPIO options
Specific MCUs may provide additional configuration options when configuring a GPIO as an output. There is only one available common option: `out::opt::atomic`, which is used to atomically set the GPIO output value. It may or may not be supported by the MCU.

### Input GPIO modes
There are four available common input modes:
* `in::analog` - configure GPIO as an analog input
* `in::floating` - configure GPIO as a floating input
* `in::pull_up` - configure GPIO as a pull-up input
* `in::pull_down` - configure GPIO as a pull-down input

Some of these options may not be supported by the MCU of your choice. Some MCUs may provide additional modes.

### Input GPIO options
Specific MCUs may provide additional configuration options when configuring a GPIO as an input.

### Output GPIO values
There are three structs that indicate the output GPIO values:
* `out::one` - logical one
* `out::zero` - logical zero
* `out::keep_value` - don't change the output level

### GPIO configuration structs
To configure a GPIO, several structures are used:
```cpp
template<typename Pin, typename OutputMode,
	typename OutValue = out::keep_value, typename... OutputOptions>
struct as_output;
```
This structure is used to configure a GPIO as an output. `Pin` indicates the GPIO to configure, `OutputMode` indicates the output mode, `OutValue` sets the logical level of the pin, `OutputOptions` is an optional list of additional output options.

```cpp
template<typename Pin, typename InputMode, typename... InputOptions>
struct as_input;
```
This structure is used to configure a GPIO as an input. `Pin` indicates the GPIO to configure, `InputMode` indicates the input mode, `InputOptions` is an optional list of additional input options.

```cpp
template<typename Pin, typename OutValue, typename... OutputOptions>
struct to_value;
```
This structure is used to set the GPIO output level. `Pin` indicates the GPIO to configure, `OutValue` sets the logical level of the pin, `OutputOptions` is an optional list of additional output options.

## GPIO configuration
Here is an example of the GPIO configuration:
```cpp
mcutl::gpio::configure_gpio<
	mcutl::gpio::as_input<mcutl::gpio::gpioa<2>, mcutl::gpio::in::pull_up>,
	mcutl::gpio::as_output<mcutl::gpio::gpioa<3>, mcutl::gpio::out::push_pull>,
	mcutl::gpio::as_output<mcutl::gpio::gpioa<4>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::one>,
	mcutl::gpio::as_output<mcutl::gpio::gpioc<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::keep_value, mcutl::gpio::out::opt::freq_10mhz>,
	mcutl::gpio::as_input<mcutl::gpio::gpioa<7>, mcutl::gpio::in::analog>,
	mcutl::gpio::to_value<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>,
	mcutl::gpio::enable_peripherals
>();
```
This call is self-explanatory: it configures `gpioa<2>` as a pull-up input, `gpioa<3>` as a push-pull output, `gpioa<4>` as an open-drain output with logical level `1`, `gpioc<11>` as an open-drain output with `10 MHz` frequency, `gpioa<7>` as an analog input, sets the `gpiob<1>` logical level to `1` without altering its configuration, and enables all the required peripherals for all of the listed ports (`enable_peripherals`).

The configuration can be wrapped to `gpio::config` structure:
```cpp
using gpio_config = mcutl::gpio::config<
	mcutl::gpio::as_input<mcutl::gpio::gpioa<2>, mcutl::gpio::in::pull_up>,
	mcutl::gpio::as_output<mcutl::gpio::gpioa<3>, mcutl::gpio::out::push_pull>,
	mcutl::gpio::as_output<mcutl::gpio::gpioa<4>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::one>,
	mcutl::gpio::as_output<mcutl::gpio::gpioc<11>, mcutl::gpio::out::open_drain,
		mcutl::gpio::out::keep_value, mcutl::gpio::out::opt::freq_10mhz>,
	mcutl::gpio::as_input<mcutl::gpio::gpioa<7>, mcutl::gpio::in::analog>,
	mcutl::gpio::to_value<mcutl::gpio::gpiob<1>, mcutl::gpio::out::one>,
	mcutl::gpio::enable_peripherals
>;
mcutl::gpio::configure_gpio<gpio_config>();
```

You may want to combine as much GPIOs as possible in a single `configure_gpio` call, as this call packs and combines MCU register reads and writes. This makes the code smaller and faster.

## set_out_value
The following function sets the output value of a GPIO:
```cpp
template<typename Pin, typename Value, typename... OutputOptions>
void set_out_value() noexcept;
```
This call sets the output value of the output `Pin` to `Value` (which can be `out::one` or `out::zero`) with optional `OutputOptions`.

## set_out_value_atomic
The following function sets the output value of a GPIO:
```cpp
template<typename Pin, typename Value, typename... OutputOptions>
void set_out_value_atomic() noexcept;
```
This call sets the output value of the output `Pin` to `Value` (which can be `out::one` or `out::zero`) with optional `OutputOptions`. This call is guaranteed to run atomically. No locking is required when changing the levels of shared GPIOs using this call.

## set_one, set_zero
```cpp
template<typename Pin>
void set_one() noexcept;
template<typename Pin>
void set_zero() noexcept;
```
These calls set the output `Pin` value to `1` or `0`, respectively.

## set_one_atomic, set_zero_atomic
```cpp
template<typename Pin>
void set_one_atomic() noexcept;
template<typename Pin>
void set_zero_atomic() noexcept;
```
These calls set the output `Pin` value to `1` or `0`, respectively. This call is guaranteed to run atomically. No locking is required when changing the levels of shared GPIOs using this call.

## get_input_values_mask, get_output_values_mask, pin_bit_mask_v
```cpp
template<bool NegateBits, typename... Pins>
[[nodiscard]] auto get_input_values_mask() noexcept;
template<bool NegateBits, typename... Pins>
[[nodiscard]] auto get_output_values_mask() noexcept;
```
These calls may be used to get the input or output bitmask for the `Pins` GPIOs. All `Pins` must be from the same GPIO port. For example, this is a way to check which bits are set for input pins `1`, `2` and `5` of the `gpiob` port:
```cpp
auto set_pins = mcutl::gpio::get_input_values_mask<false,
	mcutl::gpio::gpiob<1>,
	mcutl::gpio::gpiob<2>,
	mcutl::gpio::gpiob<5>
>();

//You may later want to apply a bitmask to see which pins are set separately:
if (set_pins & mcutl::gpio::pin_bit_mask_v<mcutl::gpio::gpiob<1>>)
{
	//If gpiob<1> is set to 1...
}
if (set_pins & mcutl::gpio::pin_bit_mask_v<
	mcutl::gpio::gpiob<2>, mcutl::gpio::gpiob<5>>)
{
	//If gpiob<2> ot gpiob<5> is set to 1...
}
```
The `NegateBits` parameter indicates if the port value must be negated after reading.

## get_input_bit, get_output_bit
```cpp
template<typename Pin>
[[nodiscard]] bool get_input_bit() noexcept;
template<typename Pin>
[[nodiscard]] bool get_output_bit() noexcept;
```
These calls may be used to get the input or output logical level for the `Pin`. For example, to check if there is a logical level `1` at the input pin `gpiod<15>`, the following code may be used:
```cpp
bool is_one = mcutl::gpio::get_input_bit<mcutl::gpio::gpiod<15>>;
```

## is_output
```cpp
template<typename Pin>
bool is_output() noexcept;
```
Returns `true` if the `Pin` is configured as an output, and `false` otherwise.

## is
```cpp
template<typename PinConfig>
[[nodiscard]] bool is() noexcept;
```
Returns `true` if the `PinConfig` exactly matches the real configuration of the pin. For example, the following call checks if the pin `gpiob<11>` is an open drain output with `50 MHz` frequency and its logical level set to `1`:
```cpp
bool config_matches = mcutl::gpio::is<mcutl::gpio::as_output<mcutl::gpio::gpiob<11>,
	mcutl::gpio::out::open_drain, mcutl::gpio::out::one, mcutl::gpio::out::opt::freq_50mhz>>();
```

## has
```cpp
template<typename Pin, typename... Options>
[[nodiscard]] bool has() noexcept;
```
Checks if the `Pin` has all of the traits listed in the `Options` list. All `Options` must be either input or output. Here are several examples:
```cpp
//Checks if the gpiob<1> pin is configured as an output
bool is_output = mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::keep_value>();

//Checks if the gpiob<1> pin is configured as an output and has the logical level `1`
bool is_one_output = mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::zero>();

//Checks if the gpiob<1> pin is configured as a push-pull alternative function output with
//50 MHz frequency and has the logical level `1`
bool has_config = mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::out::push_pull_alt_func,
	mcutl::gpio::out::opt::freq_50mhz, mcutl::gpio::out::one>();

//Checks if the gpiob<1> pin is configured as an analog input
bool is_analog_input = mcutl::gpio::has<mcutl::gpio::gpiob<1>, mcutl::gpio::in::analog>();
```

## MCU-specific configuration

### STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific GPIO output options
There are several specific GPIO output options for these MCUs to set the GPIO pin frequency for output GPIOs:
* `out::opt::freq_2mhz` - set the frequency to 2 MHz
* `out::opt::freq_10mhz` - set the frequency to 10 MHz
* `out::opt::freq_50mhz` - set the frequency to 50 MHz. This is the default frequency for the output GPIOs if the option is not present

### STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific GPIO pins
The following GPIO pins may be present for these MCUs: `gpioa`, `gpiob`, `gpioc`, `gpiod`, `gpioe`, `gpiof`, `gpiog`. These aliases take a pin number (which can be `0` to `15` inclusive), for example: `gpioc<7>`.