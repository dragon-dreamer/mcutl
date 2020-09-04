# MCU analog-to-digital converter (ADC) configuration (mcutl/adc/adc.h)
This header provides facilities to configure and use the MCU analog-to-digital converters (if supported by the MCU). All of the definitions are in the `mcutl::adc` namespace, unless otherwise stated.

## Public definitions
There's a generic definition of an ADC channel, which may be used in many configuration functions:
```cpp
using channel_index_type = uint8_t;

template<channel_index_type ChannelIndex>
struct channel;
```

There are three groups of functions to work with the ADC: configuration, calibration and conversion. Each of these groups can support several options, which are placed in the `mcutl::adc::init`, `mcutl::adc::cal` and `mcutl::adc::conv` namespaces, respectively.

### Configuration options (`mcutl::adc::init` namespace)
You may pass the `mcutl::adc::channel` structure to the configuration functions to indicate the channel you are going to perform conversion for. Some MCUs may support another conversion modes (such as group conversions).

---

```cpp
template<bool Enable>
struct enable_peripheral;
```
This structure can be passed to the configuration functions to enable or disable the ADC peripheral. For some MCUs, this may be a no-op.

---

```cpp
template<bool Enable>
struct enable;
```
This structure can be passed to the configuration functions to enable or disable the ADC.

---

```cpp
struct base_configuration_is_currently_present;
```
This structure indicates that you are configuring the ADC for the first time, and the reset ADC configuration is currently present. This may allow the configuration functions skip some checks and actions.

---

```cpp
template<uint64_t SysTickFrequency, typename ClockConfig>
struct wait_finished;
```
This structure can be passed to the configuration functions to indicate that the function must wait until the configuration is completed and the ADC is ready. For some MCUs or some configurations, this may be a no-op. If the systick is available, the waiting is done using the [SYSTICK wait](systick.md) functions. Otherwise, it may be done using other synchronous means (such as a simple loop). If the SYSTICK is available, `SysTickFrequency` must be passed to indicate the frequency of the SYSTICK timer. `ClockConfig` must indicate the current MCU [clock configuration](clock.md) (`mcutl::clock::config`). You may also perform the waiting manually (see the `initialization_time` trait description below).

---

```cpp
template<typename Channel, uint64_t InputImpedanceOhms, typename ClockConfig>
struct input_impedance;
```
This structure can be passed to the configuration functions to set the input impedance of a `Channel` (which must be of `mcutl::adc::channel` type). `InputImpedanceOhms` indicates the input impedance in `Ohms`, and `ClockConfig` must indicate the current MCU [clock configuration](clock.md) (`mcutl::clock::config`). Some MCUs may not support this option, see the traits description below. Input impedance is used to calculate the optimal ADC sampling time to achieve the best accuracy. You may specify one or more `input_impedance` options for different channels in a single configuration call.

---

```cpp
namespace data_alignment
{
struct left;
struct right;
} //namespace data_alignment
```
These two structures indicate the data alignment of the ADC result. By default the result is right-aligned. For example, if the ADC resolution is 12 bits, and the ADC result register is 16 bits, it is possible to left- or right-align the 12-bit value in the result register. Some MCUs may not support this option, see the traits description below.

---

```cpp
namespace interrupt
{
struct conversion_complete;

struct enable_controller_interrupts;
struct disable_controller_interrupts;
} //namespace interrupt
```
This namespace contains a list of interrupts which are supported by the MCU ADC. The `conversion_complete` interrupt is raised when the ADC conversion completes and the result is ready.

To enable an interrupt, specify the related structure in the ADC configuration (see below for examples). To enable an interrupt and set its priority/subpriority, use `mcutl::interrupt::interrupt` [wrapper](interrupt.md). To disable an interrupt, wrap the related interrupt structure into the `mcutl::interrupt::disabled` wrapper structure. To additionally enable or disable the related interrupt controllers interrupts, use the `enable_controller_interrupts` or `disable_controller_interrupts` structs.

The `enable_controller_interrupts` switch will enable all the required for the selected configuration interrupts. The `disable_controller_interrupts` switch will disable the interrupt controller interrupts which are not used in the current ADC configuration, leaving all used interrupts enabled.

Add the `mcutl::interrupt::priority_count` option to explicitly set the interrupt priority count. Use this option if you explicitly changed the interrupt priority count when calling `mcutl::interrupt::initialize_controller`.

### Calibration options (`mcutl::adc::cal` namespace)
These options are available only when the `supports_calibration` trait is `true` for the ADC of your choice.

```cpp
using init::wait_finished;
```
This structure is defined the same way as the `mcutl::adc::init::wait_finished` structure. It allows to wait for the calibration to finish. The set of parameters is the same as for the `mcutl::adc::init::wait_finished` structure. You may also perform the waiting manually (see the `calibration_time` trait and the `is_calibration_finished` function descriptions below).

### Conversion options (`mcutl::adc::conv` namespace)
You must pass the `mcutl::adc::channel` structure to the configuration functions to indicate the channel you are going to perform conversion for. Some MCUs may support another conversion modes (such as group conversions), and in this case you may select another conversion mode using the device-specific options. The channel options you've passed to the configuration functions must match the channel options you pass to the conversion functions. Basically, you need to configure the conversion mode, and then to perform conversion for the very same mode (set and order of channels).

## ADC traits
There are several traits to determine the characteristics and capabilities of the ADC or the ADC channel. All traits below require the `Adc` template parameter, which is a device-specific type to define the ADC (such as `mcutl::adc::adc1` or `mcutl::adc::adc2`, for example).

---

```cpp
template<typename Adc>
using peripheral_type = ...;
```
This typedef indicates the [peripheral type](periph.md) which must be enabled before the ADC configuration or conversions. This may also be a `mcutl::types::list` list of peripherals, or the `mcutl::periph::no_periph` type. This peripheral can be enabled or disabled automatically when configuring using the `enable_peripheral` option (see above).

---

```cpp
template<typename Adc, typename Interrupt>
using interrupt_type = ...;
```
This typedef indicates the [interrupt type](interrupt.md) of the related `Adc` `Interrupt`. You may pass `mcutl::adc::init::interrupt::conversion_complete` or other ADC interrupt type to the `Interrupt` parameter.

---

```cpp
template<typename Adc>
using conversion_result_type = ...;
```
This typedef indicates the conversion result type for the `Adc`. This is an unsigned integer type sufficient to keep the ADC conversion result value. For example, for a 12-bit ADC, this type may be `uint16_t` or `uint32_t`.

---

```cpp
template<typename Adc>
constexpr auto resolution_bits = ...;
```
This constant indicates the `Adc` resolution bit count.

---

```cpp
template<typename Adc>
constexpr conversion_result_type<Adc> max_result_value = ...;
```
This constant indicates the maximum possible value for the `Adc` conversion result. For example, for a 12-bit ADC this will be `0xFFFu`.

---

```cpp
template<typename Adc, typename ClockConfig>
using initialization_time = ...;
```
This typedef indicates the initialization time for the `Adc`. `ClockConfig` must indicate the current MCU [clock configuration](clock.md) (`mcutl::clock::config`). This typedef is of the `mcutl::types::duration` type (see the `mcutl/utils/duration.h` header).

---

```cpp
template<typename Adc, typename ClockConfig>
using calibration_time = ...;
```
This typedef indicates the calibration time for the `Adc`. `ClockConfig` must indicate the current MCU [clock configuration](clock.md) (`mcutl::clock::config`). This typedef is of the `mcutl::types::duration` type (see the `mcutl/utils/duration.h` header).

---

```cpp
template<typename Adc, typename Channel>
using map_adc_channel_to_gpio = ...;
template<typename Adc, typename Gpio>
using map_gpio_to_adc_channel = ...;
```
These typedefs map the `Adc` `Channel` to the GPIO pin and back. Each ADC channel is connected to a pin (or some internal reference). These mappings are available for those channels which are connected to pins. The `map_adc_channel_to_gpio` maps the `Channel` (`mcutl::adc::channel`) to the [`mcutl::gpio::pin`](gpio.md) type. The `map_gpio_to_adc_channel` maps the `Gpio` ([`mcutl::gpio::pin`](gpio.md)) to the `mcutl::adc::channel` type.

---

```cpp
using gpio_config = ...;
```
This typedef indicates the required pin configuration type to use it with ADC. For example, this type may be `mcutl::gpio::in::analog`. ADC configuration functions currently does not configure the pins, so you must configure the pins manually before conversion.

### Capabilities
There are some traits which indicate the capabilities of the ADC:
```cpp
//Indicates if the Adc supports calibration
template<typename Adc>
constexpr bool supports_calibration = ...;
//Indicates if the Adc supports the input_impedance option
template<typename Adc>
constexpr bool supports_input_impedance_option = ...;
//Indicates if the Adc supports the clear_pending_flags_atomic function
template<typename Adc>
constexpr bool supports_atomic_clear_pending_flags = ...;
//Indicates if the Adc supports the data_alignment options
template<typename Adc>
constexpr bool supports_data_alignment = ...;
```

## ADC configuration
There are two functions to configure the ADC:
```cpp
template<typename Adc, typename... Options>
void configure() noexcept;
template<typename Adc, typename... Options>
void reconfigure() noexcept;
```
The first function configures the ADC with the `Options` provided, resetting other ADC settings to their default values. The second function keeps the existing configuration and alters only the `Options` provided. `Options` may be any options from the `mcutl::adc::init` namespace and also conversion mode options (such as the `mcutl::adc::channel`). The `Adc` template parameter is a device-specific type to define the ADC (such as `mcutl::adc::adc1` or `mcutl::adc::adc2`, for example). Here are several examples of the ADC configuration:

```cpp
//Enable the mcutl::adc::adc1 ADC, enable its peripheral, prepare conversion for channel 5,
//set data alignment to left, set the channel 5 input impedance to 25 KOhm,
//enable the conversion_complete interrupt and set its priority to 3,
//enable the related interrupt controller interrupt.
//Wait for the initialization to finish.
//Finally, indicate that the configuration is performed for the first time
//(base_configuration_is_currently_present) to optimize out the excessive actions.
mcutl::adc::configure<mcutl::adc::adc1,
	mcutl::adc::init::enable_peripheral<true>,
	mcutl::adc::channel<5>,
	mcutl::adc::init::input_impedance<mcutl::adc::channel<5>, 25'000, clock_config>,
	mcutl::adc::init::enable<true>,
	mcutl::adc::init::data_alignment::left,
	mcutl::adc::init::wait_finished<72'000'000, clock_config>,
	mcutl::interrupt::interrupt<mcutl::adc::init::interrupt::conversion_complete, 3>,
	mcutl::adc::init::interrupt::enable_controller_interrupts,
	mcutl::adc::init::base_configuration_is_currently_present
>();
```

```cpp
//Disable the mcutl::adc::adc1 ADC. Other ADC configuration options
//are not preserved.
mcutl::adc::configure<mcutl::adc::adc1,
	mcutl::adc::init::enable_peripheral<false>,
	mcutl::adc::init::enable<false>
>();
```

```cpp
//Enable the mcutl::adc::adc1 ADC, keep all other options intact.
mcutl::adc::reconfigure<mcutl::adc::adc1, mcutl::adc::init::enable<true>>();
```

```cpp
//Prepare the mcutl::adc::adc1 ADC for the channel 2 conversion.
//Keep all other options intact.
mcutl::adc::reconfigure<mcutl::adc::adc1, mcutl::adc::channel<2>>();
```

```cpp
//Disable all unneeded interrupt controller mcutl::adc::adc1 interrupts,
//keep all other options intact.
//If the mcutl::adc::adc1 had the conversion_complete interrupt enabled,
//the related interrupt controller interrupt will not be disabled.
mcutl::adc::reconfigure<mcutl::adc::adc1,
	mcutl::adc::init::interrupt::disable_controller_interrupts>();
```

When using the `enable_controller_interrupts` or `disable_controller_interrupts` options with the `configure` call, only the interrupt controller interrupts, specified in the same call, will be enabled or disabled. When using the `enable_controller_interrupts` or `disable_controller_interrupts` options with the `reconfigure` call, only truly used or unused controller interrupts will be enabled or disabled despite other options passed to the call.

## ADC calibration
It may be beneficial to calibrate the ADC before conversion to increase its accuracy. There is a function to perform the calibration, which is available when the `supports_calibration` trait is `true`:
```cpp
template<typename Adc, typename... Options>
void calibrate() noexcept;
```
`Options` may be any options from the `mcutl::adc::cal` namespace. The `Adc` template parameter is a device-specific type to define the ADC (such as `mcutl::adc::adc1` or `mcutl::adc::adc2`, for example).

There's also a function to check if the calibration has finished:
```cpp
template<typename Adc>
bool is_calibration_finished() noexcept;
```
You may use the `mcutl::adc::cal::wait_finished` option to wait for the calibration to finish or use the `is_calibration_finished` function in a loop to wait for it to complete. There is also the `calibration_time` trait, which indicates the time required to calibrate the ADC.

## ADC conversion
After you have configured and enabled the ADC, you can perform the conversion using the following function:
```cpp
template<typename Adc, typename... Options>
decltype(auto) prepare_conversion() noexcept;
```
`Options` may be any options from the `mcutl::adc::conv` namespace or the conversion mode options (such as the `mcutl::adc::channel`). You must pass the very same conversion mode options to this function, as you passed to the `configure` or `reconfigure` function. For example, if you called `configure` with the `mcutl::adc::channel<3>` option, you must pass the same `mcutl::adc::channel<3>` option to the `prepare_conversion` function.

The `Adc` template parameter is a device-specific type to define the ADC (such as `mcutl::adc::adc1` or `mcutl::adc::adc2`, for example).

This function returns an instance of a class, which is suitable for the required conversion mode. For example, if you've passed the `mcutl::adc::channel<...>` option to the `prepare_conversion` function, it will return an instance of a class for a single conversion. This class will provide the following methods:

```cpp
class _unnamed_
{
	//Starts the prepared conversion
	void start() noexcept;
	//Returns the conversion result
	mcutl::adc::conversion_result_type<Adc> get_conversion_result() noexcept;
	//Returns true if the conversion has finished
	bool is_finished() noexcept;
};
```

The MCU of your choice may provide other conversion mode options (for example, scanning of several channels in a row), in this case the `prepare_conversion` function may return another object with another set of methods.

Here is an example of configuring the ADC and converting:
```cpp
//Configure the mcutl::adc::adc1 ADC to convert channel 10 with input impedance of 10 KOhm,
//enable the ADC and its peripheral and wait for the initialization to complete.
mcutl::adc::configure<mcutl::adc::adc1,
	mcutl::adc::init::enable_peripheral<true>,
	mcutl::adc::channel<10>,
	mcutl::adc::init::input_impedance<mcutl::adc::channel<10>, 10'000, clock_config>,
	mcutl::adc::init::enable<true>,
	mcutl::adc::init::wait_finished<72'000'000, clock_config>
>();

//Prepare conversion for the channel 10 (note that we configured the ADC for this channel).
auto conv = mcutl::adc::prepare_conversion<mcutl::adc::adc1, mcutl::adc::channel<10>>();

//Start the conversion
conv.start();

//Synchronously wait for the conversion to finish
while (!conv.is_finished()) {}

//Get the conversion result
auto result = conv.get_conversion_result();
```

## ADC interrupt pending flags
There are several functions to check which ADC interrupt pending flags are set, and to clear them.

```cpp
template<typename Adc, typename... Interrupts>
auto get_pending_flags() noexcept;
template<typename Adc, typename... Interrupts>
constexpr auto pending_flags_v = ...;
```
The `get_pending_flags` function can be used to get the currently pending `Adc` interrupt flags. To determine which interrupts are currently pending, use the `pending_flags_v` constant, for example:

```cpp
auto pending_flags = mcutl::adc::get_pending_flags<
	mcutl::adc::init::interrupt::conversion_complete
>()),

bool alarm_pending = (pending_flags
	& mcutl::adc::pending_flags_v<mcutl::adc::init::interrupt::conversion_complete>) != 0;
```

```cpp
template<typename Adc, typename... Interrupts>
void clear_pending_flags() noexcept;
template<typename Adc, typename... Interrupts>
void clear_pending_flags_atomic() noexcept;
```
These functions can be used to reset the pending `Adc` `Interrupts` flags, so that the interrupt will not fire again after being handled. The `clear_pending_flags_atomic` is available only when the `supports_atomic_clear_pending_flags` trait is `true`.

The `Adc` template parameter is a device-specific type to define the ADC (such as `mcutl::adc::adc1` or `mcutl::adc::adc2`, for example).

## STM32F1** specific options and functions
These MCUs provide the `mcutl::adc::adc1`, `mcutl::adc::adc2` and `mcutl::adc::adc3` analog-to-digital converters. Some of them may be absent for some MCU models.

### STM32F1** ADC configuration options
In addition to the above mentioned options, these MCUs provide the following options for the `configure` and `reconfigure` functions:

```cpp
template<typename Channel, typename SampleTimeCycles>
struct sample_time;
```
This structure indicates the exact sample time for the `Channel`. `SampleTimeCycles` may be one of the following types:
```cpp
namespace mcutl::adc::init::sample_time_cycles
{
struct cycles_1_5; //1.5 cycles
struct cycles_7_5; //7.5 cycles
struct cycles_13_5; //13.5 cycles
struct cycles_28_5; //28.5 cycles
struct cycles_41_5; //41.5 cycles
struct cycles_55_5; //55.5 cycles
struct cycles_71_5; //71.5 cycles
struct cycles_239_5; //239.5 cycles
} //namespace mcutl::adc::init::sample_time_cycles
```
You may instead use the easier option `input_impedance`, which calculates the best sampling time automatically.

### STM32F1** - scanning mode
There is an option, which allows to convert one or more (up to `16`) channels in a row and place the results to an in-memory array via the [DMA](dma.md). This option may be specified instead of the `mcutl::adc::channel` option and must be present for both the `configure` (or `reconfigure`) call and the `prepare_conversion` call:

```cpp
template<typename DmaChannelConfig, typename Channel, typename... Channels>
struct scan_channels;
```
Here `Channel` and `Channels` is the list of channels to convert in an order of appearance. The `DmaChannelConfig` parameter must be of the `mcutl::adc::dma_channel_config` type:
```cpp
template<typename DmaChannel, typename... DmaTraits>
struct dma_channel_config;
```
Here `DmaChannel` is the channel to use, and `DmaTraits` are additional optional [DMA options](dma.md). For `STM32F1**` controllers, there are fixed DMA channels attached to the ADC peripherals. For the `mcutl::adc::adc1` it is the DMA1 channel 1 (`mcutl::dma::dma1<1>`), for the `mcutl::adc::adc3` it is the DMA2 channel 5 (`mcutl::dma::dma2<5>`), and the `mcutl::adc::adc2` does not have the direct DMA connection. To determine which DMA peripheral and channel to use, there is the following device-specific trait:
```cpp
template<typename Adc>
using dma_channel = ...;
```

When using the `scan_channels` mode, the `prepare_conversion` call will return an object of a class with the following public methods:
```cpp
class _unnamed_
{
	//Start the conversion; result_pointer must point to an array to hold the
	//result data (at least of size to hold all the scanned channels data).
	//ValueType must be a 2-byte integer type.
	template<typename ValueType, size_t N>
	void start(volatile ValueType (&result_pointer)[N]) noexcept;
	template<typename ValueType>
	void start(volatile ValueType* result_pointer) noexcept;
	
	//Returns true when the conversion is completed.
	bool is_finished() noexcept;
};
```

Here is an example of configuration and conversion using the `scan_channels` option:
```cpp
//Use the DMA channel suitable for the adc1 ADC.
//Additionally, set the DMA priority to high.
using dma_config = mcutl::adc::dma_channel_config<
	mcutl::adc::dma_channel<mcutl::adc::adc1>,
	mcutl::dma::priority::high
>;

//Convert the ADC channels 5, 7, 11 in a row.
using scanned_channels = mcutl::adc::init::scan_channels<
	dma_config,
	mcutl::adc::channel<5>,
	mcutl::adc::channel<7>,
	mcutl::adc::channel<11>
>;

//Prepare the adc1 ADC: enable it, enable its peripheral,
//configure the conversion sequence.
mcutl::adc::configure<mcutl::adc::adc1,
	mcutl::adc::init::enable<true>,
	mcutl::adc::init::enable_peripheral<true>,
	scanned_channels
>();

//Prepare the conversion
auto conv = mcutl::adc::prepare_conversion<mcutl::adc::adc1,
	scanned_channels>();

//Start the conversion, put the results to the appropriate array
uint16_t result[3] {};
conv.start(result);

//Wait until the conversion is completed
while (!conv.is_finished()) {}

//Now result[0] contains the ADC channel 5 value,
//result[1] contains the ADC channel 7 value,
//result[2] contains the ADC channel 11 value.
```
