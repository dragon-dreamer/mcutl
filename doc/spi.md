# MCU SPI configuration and access (mcutl/spi/spi.h)
This header provides facilities to configure and use the MCU SPI interface (if supported by the MCU). All of the definitions are in the `mcutl::spi` namespace, unless otherwise stated.

## Public definitions
There is a number of options which may be used when configuring SPI.

```cpp
namespace slave_management
{
struct none;
struct software;
struct hardware_with_output;
struct hardware_without_output;
} //namespace slave_management
```
These structs indicate the slave management mode for the master SPI.
* `none` - No management.
* `software` - Software slave management (software sets if the SPI is currently master or slave, and the SS pin value is set by the software).
* `hardware_with_output` - Hardware slave management. The SS pin is automatically pulled down when performing a transfer.
* `hardware_without_output` - Hardware slave management. The SS pin detects another master transfer and triggers the mode error, switching the SPI master to slave mode automatically.

Some MCUs may support another slave management modes, or may not support some of these common modes. The traits (described below) can help to detect the MCU capabilities.

---

```cpp
namespace frame_format
{
struct lsb_first;
struct msb_first;
} //namespace frame_format_type
```
These structs set the bit order for the transfers: LSB or MSB, respectively.

---

```cpp
namespace clock_polarity
{
struct idle_0;
struct idle_1;
} //namespace clock_polarity

using cpol_0 = clock_polarity::idle_0;
using cpol_1 = clock_polarity::idle_1;
```
These structs set the SPI clock polarity: `cpol_0` - clock to `0` when idle; `cpol_1` - clock to `1` when idle.

---

```cpp
namespace clock_phase
{
struct capture_first_clock;
struct capture_second_clock;
} //namespace clock_phase

using cpha_0 = clock_phase::capture_first_clock;
using cpha_1 = clock_phase::capture_second_clock;
```
These structs set the SPI clock phase: `cpha_0` - the first clock transition is the first data capture edge; `cpha_1` - the second clock transition is the first data capture edge.

---

```cpp
namespace interrupt
{
struct transmit_buffer_empty;
struct recieve_buffer_not_empty;
struct error;

struct enable_controller_interrupts;
struct disable_controller_interrupts;
} //namespace interrupt
```
This namespace contains a list of interrupts which are supported by the MCU SPI. The `error` interrupt is raised when the SPI encounters any error (master mode fault or overflow, for example). The `transmit_buffer_empty` interrupt is raised when the SPI transmit buffer has become empty. The `recieve_buffer_not_empty` interrupt is raised when the SPI receive buffer has become not empty. Note that these interrupts may be mapped to a single interrupt controller interrupt. Some MCUs may not support these interrupts or support some other interrupts.

To enable an interrupt, specify the related structure in the SPI configuration (see below for examples). To enable an interrupt and set its priority/subpriority, use `mcutl::interrupt::interrupt` [wrapper](interrupt.md). To disable an interrupt, wrap the related interrupt structure into the `mcutl::interrupt::disabled` wrapper structure. To additionally enable or disable the related interrupt controllers interrupts, use the `enable_controller_interrupts` or `disable_controller_interrupts` structs.

The `enable_controller_interrupts` switch will enable all the required for the selected configuration interrupts. The `disable_controller_interrupts` switch will disable the interrupt controller interrupts which are not used in the current SPI configuration, leaving all used interrupts enabled.

Add the `mcutl::interrupt::priority_count` option to explicitly set the interrupt priority count. Use this option if you explicitly changed the interrupt priority count when calling `mcutl::interrupt::initialize_controller`.

---

```cpp
struct initialize_pins;
```
This struct can be specified in the SPI configuration to automatically enable and configure all the required SPI pins (MISO, MOSI, SCK, SS). This option configures only the pins used in the configuration. For example, if you don't need to receive any data (only to transmit), the MISO pin configuration will be untouched.

---

```cpp
struct clear_error_flags;
```
This struct can be specified in the SPI configuration to clear the SPI error flags (such as mode fault or overflow) prior to configuring.

## SPI traits
There are several traits to determine the characteristics and capabilities of MCU SPI. All traits below require the `Spi` template parameter, which is a device-specific type to define the SPI interface (such as `mcutl::spi::spi1` or `mcutl::spi::spi2`, for example).

---

```cpp
template<typename Spi>
using peripheral_type = ...;
```
This typedef indicates the [peripheral type](periph.md) which must be enabled before the SPI configuration or transfers. This may also be a `mcutl::types::list` list of peripherals, or the `mcutl::periph::no_periph` type.

---

```cpp
template<typename Spi, typename Interrupt>
using interrupt_type = ...;
```
This typedef indicates the [interrupt controller interrupt type](interrupt.md) of the related `Spi` interface `Interrupt`. You may pass `mcutl::spi::interrupt::transmit_buffer_empty` or other SPI interrupt type to the `Interrupt` parameter.

---

```cpp
template<typename Spi>
using default_slave_management_mode = ...;
```
This typedef indicates the default slave management mode for the `Spi` interface.

---

```cpp
template<typename Spi>
using miso_pin = ...;

template<typename Spi>
using mosi_pin = ...;

template<typename Spi>
using ss_pin = ...;

template<typename Spi>
using sck_pin = ...;
```
These typedefs indicate the [pin types](gpio.md) (of the `mcutl::gpio::pin` type) for the MISO, MOSI, SS and SCK SPI inputs/outputs. If a pin is absent for the SPI interface, the typedef maps to `void`.

---

```cpp
template<typename Spi>
using supported_data_types = ...;
```
This typedef indicates supported data types for SPI transfers. This is a `types::list`, which contains one or more types.

### Capabilities
There are some traits which indicate the capabilities of the SPI interface:

```cpp
//True if the SPI supports the software slave management mode
template<typename Spi>
constexpr bool supports_software_slave_management = ...;

//True if the SPI supports the hardware with output slave management mode
template<typename Spi>
constexpr bool supports_hardware_with_output_slave_management = ...;

//True if the SPI supports the hardware without output slave management mode
template<typename Spi>
constexpr bool supports_hardware_without_output_slave_management = ...;

//True if the SPI supports the frame format options
template<typename Spi>
constexpr bool supports_frame_format = ...;

//True if the SPI supports the error interrupt
template<typename Spi>
constexpr bool supports_error_interrupt = ...;
```

## SPI transmit and receive modes
There are different modes of the SPI transmit and receive transfers, which can be selected when creating the SPI instance. These modes determine how the SPI will transfer data. For example, the transfer may be done either synchronously, or using interrupts or DMA. The following modes are currently available for all devices:

```cpp
struct empty_mode;
```
You can specify this mode as either the receive or the transmit mode argument to disable either the SPI receiving or transmitting side. For example, if your SPI master requires only to transmit data, you may specify the `empty_mode` as the receive mode.

## SPI master
To create the SPI master instance, use the `create_spi_master` method:
```cpp
template<typename Spi, typename DataType = uint8_t, typename ReceiveMode, typename TransmitMode>
decltype(auto) create_spi_master(ReceiveMode&& receive_mode, TransmitMode&& transmit_mode) noexcept;
```

This method creates an instance of the `mcutl::spi::master` class, which is described below. The `Spi` template parameter is a device-specific type to define the SPI interface (such as `mcutl::spi::spi1` or `mcutl::spi::spi2`, for example). The `DataType` indicates the data type of transfers and is `uint8_t` by default, which is always supported. The `ReceiveMode` and `TransmitMode` parameters set the transmit and the receive modes for the SPI interface.

```cpp
template<typename Spi, typename ReceiveMode, typename TransmitMode, typename DataType>
class master
{
public:
	//Alias of the Spi template parameter
	using spi_type = ...;
	//Alias of the DataType template parameter
	using data_type = ...;
	//Alias of the ReceiveMode template parameter
	using receive_mode = ...;
	//Alias of the TransmitMode template parameter
	using transmit_mode = ...;
	//Data length type which is used in transmit() and transmit_receive() methods
	using data_length_type = ...;
	//Maximum possible data length for a single transfer
	//(for a single transmit() or transmit_receive() call)
	static constexpr data_length_type max_data_length = ...;
	
	//The mcutl::gpio::config type, which can be passed to the mcutl::gpio::configure_gpio function
	//to configure the SPI pins as needed for the selected ReceiveMode, TransmitMode
	//and Options. Options is a set of structs described above, which can be optionally
	//wrapped in the mcutl::spi::config type.
	template<typename... Options>
	using gpio_config = ...;
	
public:
	//Class constructors
	master(const master&) = delete;
	master& operator=(const master&) = delete;
	constexpr master(master&&) noexcept = default;
	constexpr master& operator=(master&&) noexcept = default;
	template<typename ReceiveModeType = ReceiveMode, typename TransmitModeType = TransmitMode>
	constexpr master(ReceiveModeType&& receive_mode = {},
		TransmitModeType&& transmit_mode = {}) noexcept;
	
	//Getters for the receive and transmit modes
	ReceiveMode& get_receive_mode() noexcept;
	TransmitMode& get_transmit_mode() noexcept;
	
	//Configures the SPI interface. This method is described below in more detail.
	template<typename... Options>
	void configure() noexcept;
	
	//Initializes the GPIO inputs and outputs as needed
	//for the selected ReceiveMode, TransmitMode
	//and Options. Options is a set of structs described above, which can be optionally
	//wrapped in the mcutl::spi::config type.
	template<typename... Options>
	static void init_pins() noexcept;
	
	//Waits for any ongoing transfer to complete and then disables the SPI
	void disable() noexcept;
	//Enables the SPI
	void enable() noexcept;
	
	//Waits for any ongoing transfer to complete
	void wait() noexcept;
	
	//Transmits data to a slave device. Here length is a number of transfers
	//(with DataType size each), not the number of bytes to transfer.
	//Received data is ignored.
	void transmit(const data_type* data, data_length_type length) noexcept;
	
	//Transmits data to a slave device and receives the response.
	//Here length is a number of transfers.
	//(with DataType size each), not the number of bytes to transfer.
	void transmit_receive(const data_type* data, data_type* receive_buffer,
		data_length_type length) noexcept;
	
	//Clears all SPI error flags, so the SPI is ready for transfer,
	//and the error interrupt will not fire if enabled.
	void clear_error_flags() noexcept;
	
	//Sets the SPI data word to transfer next.
	//May not compile for some transmit modes (for example, for the DMA transfers).
	void set_data(data_type data) noexcept;
	//Returns the next received SPI data word.
	//May not compile for some transmit modes (for example, for the DMA transfers).
	data_type get_data() noexcept;
	
	//Enables or disables the SPI source pointer increment. By default, the increment is
	//enabled. When the increment is disabled, the SPI will perform "length" transfers
	//of the first data word of the "data" pointer. This may be useful when it is required
	//to transfer the same data word more than once to avoid allocating the buffer
	//filled with the same words.
	template<bool Enable>
	void enable_source_pointer_increment() noexcept;
};
```

### SPI master configuration
To configure the SPI master, the `configure` method is used:

```cpp
template<typename... Options>
void configure() noexcept;
```
`Options` is a set of structs described above, which can be optionally wrapped in the `mcutl::spi::config` type. This method resets all of the SPI settings to their default values (expect the SPI clock prescaler, which can be configured with the [clock](clock.md) functions), and then sets the SPI options according to the `Options` provided. Here is an example of the SPI configuration:

```cpp
//Device-specific DMA transmit and receive modes are used to
//create the SPI instance.
auto spi = mcutl::spi::create_spi_master<mcutl::spi::spi1>(
	mcutl::spi::dma_receive_mode<>(),
	mcutl::spi::dma_transmit_mode<>());

//Configure the SPI interface: LSB first, clock to 1 when idle,
//the second clock transition is the first data capture edge,
//software slave management. The initialize_pins option will
//initialize all the required GPIO pins according to the
//SPI receive and transmit modes and options provided.
//The error interrupt is then enabled with the priority 2,
//and the related interrupt controller interrupt is enabled, too.
spi.configure<mcutl::spi::frame_format::lsb_first,
	mcutl::spi::cpol_1, mcutl::spi::cpha_1,
	mcutl::spi::slave_management::software,
	mcutl::spi::initialize_pins,
	mcutl::interrupt::interrupt<mcutl::spi::interrupt::error, 2>,
	mcutl::spi::interrupt::enable_controller_interrupts>();
```

## SPI interrupt pending flags
There are several functions to check which SPI interrupt pending flags are set.

```cpp
template<typename Spi, typename... Interrupts>
auto get_pending_flags() noexcept;
template<typename Spi, typename... Interrupts>
constexpr auto pending_flags_v = ...;
```
The `get_pending_flags` function can be used to get the currently pending `Spi` interrupt flags. To determine which interrupts are currently pending, use the `pending_flags_v` constant, for example:

```cpp
auto pending_flags = mcutl::spi::get_pending_flags<
	mcutl::spi::interrupt::error
>()),

bool error_pending = (pending_flags
	& mcutl::spi::pending_flags_v<mcutl::spi::interrupt::error>) != 0;
```

The `Spi` template parameter is a device-specific type to define the SPI (such as `mcutl::spi::spi1` or `mcutl::spi::spi2`, for example).

## STM32F1** specific SPI modes and functions.
The `STM32F1**` MCUs provide the following SPI interfaces: `mcutl::spi::spi1`, `mcutl::spi::spi2`, `mcutl::spi::spi3`. Some of these interfaces may not be available for all of the MCU families.

The following device-specific SPI modes are also available:
```cpp
template<typename... DmaOptions>
struct dma_transmit_mode;
template<typename... DmaOptions>
struct dma_receive_mode;
```
These modes may be selected as a transmit and receive mode for the SPI master to enable the transmission via DMA. In this case, after calling the `transmit` or the `transmit_receive` methods, the `data` and the `receive_buffer` arrays must not be deleted until the transmission completes, which may be detected by processing the DMA channel transfer complete interrupt. `DmaOptions` allow to specify additional [DMA options](dma.md), including the required DMA interrupts configuration or the DMA transfer priority.

---

```cpp
template<typename Spi, typename CurrentClockConfig, typename FrequencyOption>
void change_prescaler() noexcept;
```
The `change_prescaler` device-specific function allows to change the SPI clock prescaler without altering any other clock configurations or SPI options. The `Spi` is the device-specific class to indicate the SPI interface, such as the `mcutl::spi::spi1`, `mcutl::spi::spi2` or `mcutl::spi::spi3`. The `CurrentClockConfig` is the [clock configuration](clock.md) indicating the current MCU clock tree state. `FrequencyOption` may be one of the `mcutl::clock::min_frequency`, `mcutl::clock::max_frequency` or `mcutl::clock::required_frequency` option. You will get a compile-time error, if it's not possible to configure the SPI prescaler in a desired way.
