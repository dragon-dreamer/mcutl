# MCU external interrupts configuration (mcutl/exti/exti.h)
This header provides facilities to configure external MCU interrupts (EXTI).

### Public definitions
```cpp
template<uint32_t Index>
struct line;
```
This struct represents the external MCU interrupt line with the index `Index`.

```cpp
template<typename Line, typename LineMode, typename LineTrigger,
	typename... Options>
struct line_options;
```
This struct represents the external interrupt line configuration options. `Line` must be the `mcutl::exti::line` type to indicate the target EXTI line, `LineMode` is the line mode, `LineTrigger` is the line trigger, `Options` are additional optional MCU-specific settings for the line.

```cpp
namespace line_mode
{
struct event {};
struct interrupt {};
struct event_and_interrupt {};
} //namespace line_mode
```
These are common available line modes.
* `event` - raise the event when the EXTI triggers
* `interrupt` - trigger the nterrupt when the EXTI triggers
* `event_and_interrupt` - raise the event and trigger the interrupt when the EXTI triggers

Some of these modes may not be supported by the target MCU, and some MCUs may provide additional modes.

```cpp
namespace line_trigger
{
struct software_only {};
struct rising {};
struct falling {};
struct rising_and_falling {};
} //namespace line_trigger
```
These are common available line triggers.
* `software_only` - software-only trigger
* `rising` - rising edge trigger
* `falling` - falling edge trigger
* `rising_and_falling` - both rising and falling edges trigger

Some of these triggers may not be supported by the target MCU, and some MCUs may provide additional triggers.

```cpp
template<typename Line,
	interrupt::priority_t Priority = interrupt::default_priority,
	interrupt::priority_t Subpriority = interrupt::default_priority,
	typename... Options>
struct line_interrupt_options;
```
This struct is used to configure external interrupts through the interrupt controller. `Line` must be the `mcutl::exti::line` type to indicate the target EXTI line, `Priority` and `Subpriority` indicate the priority and the subpriority of the interrupt (see [mcutl/interrupt](interrupt.md) for details), `Options` are additional optional MCU-specific settings for the interrupt.

### Traits
There are several traits available to detect the target MCU capabilities:
```cpp
//True if the MCU supports events
constexpr bool has_events;
//True if the MCU supports rising edge triggers
constexpr bool has_rising_trigger;
//True if the MCU supports falling edge triggers
constexpr bool has_falling_trigger;
//True if the MCU supports rising+falling edge triggers
constexpr bool has_rising_and_falling_trigger;
//True if the MCU supports software triggers
constexpr bool has_software_trigger;
//True if the MCU supports pending EXTI line bits
constexpr bool has_pending_line_bits;
//True if the MCU supports the clear_pending_line_bits_atomic function
constexpr bool has_atomic_clear_pending_line_bits;
//True if the MCU supports the software_trigger_atomic function
constexpr bool has_atomic_software_trigger;
```

### enable_lines, disable_lines
```cpp
template<typename... Lines>
void enable_lines() noexcept;
template<typename... Lines>
void disable_lines() noexcept;
```
Enables or disables EXTI `Lines`, respectively. `Lines` must be a list of `mcutl::exti::line_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type. Here are several examples:
```cpp
//Enable EXTI line #5 to both raise event
//and trigger interrupt on both rising and falling edges.
mcutl::exti::enable_lines<
	mcutl::exti::line_options<
		mcutl::exti::line<5>,
		mcutl::exti::line_mode::event_and_interrupt,
		mcutl::exti::line_trigger::rising_and_falling
	>
>();

//Enable and then disable EXTI lines #1 and #15,
//the first one to generate event on rising edges,
//the second one to trigger interrupt on software trigger only
using config = mcutl::exti::config<
	mcutl::exti::line_options<
		mcutl::exti::line<1>,
		mcutl::exti::line_mode::event,
		mcutl::exti::line_trigger::rising
	>,
	mcutl::exti::line_options<
		mcutl::exti::line<15>,
		mcutl::exti::line_mode::interrupt,
		mcutl::exti::line_trigger::software_only
	>
>;
mcutl::exti::enable_lines<config>();
mcutl::exti::disable_lines<config>();
```

### clear_pending_line_bits, clear_pending_line_bits_atomic
```cpp
template<typename... Lines>
void clear_pending_line_bits() noexcept;
template<typename... Lines>
void clear_pending_line_bits_atomic() noexcept;
```
These functions are used to clear the pending EXTI line bits. Some MCUs support these bits and set them when the external interrupt happens, and it's sometimes necessary to clear them in addition to the interrupt controller pending bits to prevent infinite interrupt or event triggering. For the MCUs which do not support EXTI line pending bits, these functions are no-op. `Lines` must be a list of `mcutl::exti::line_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type.

The `clear_pending_line_bits_atomic` function is guaranteed to clear all the bits in a single atomic operation, thus requiring no additional locking. This function is available when `has_atomic_clear_pending_line_bits` is `true`.

### get_pending_line_bits, lines_bit_mask_v
```cpp
template<typename... Lines>
auto get_pending_line_bits() noexcept;
```
This function is used to check for the currently set pending EXTI line bits. Some MCUs support these bits and set them when the external interrupt happens. For the MCUs which do not support EXTI line pending bits, this function returns `0`. `Lines` must be a list of `mcutl::exti::line_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type. To check, which exact lines were triggered, the `lines_bit_mask_v` constexpr variable is used, for example:
```cpp
bool only_lines7_and_9_triggered = mcutl::exti::get_pending_line_bits<line5, line2, line19, line7>()
	== mcutl::exti::lines_bit_mask_v<line7, line19>;
```
`lines_bit_mask_v` is a template constexpr variable which takes a list of `mcutl::exti::line_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type.

### software_trigger, software_trigger_atomic
```cpp
template<typename... Lines>
void software_trigger() noexcept;
template<typename... Lines>
void software_trigger_atomic() noexcept;
```
These functions are used to trigger the external interrupt from software code. They are available when `has_software_trigger` is `true`. `Lines` must be a list of `mcutl::exti::line_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type.

The `software_trigger_atomic` is guaranteed to trigger the lines atomically, thus requiring no additional locking. This function is available when `has_atomic_software_trigger` is `true`.

### enable_interrupts
```cpp
template<typename... LineInterruptOptions>
void enable_interrupts() noexcept;
template<auto PriorityCount, typename... LineInterruptOptions>
void enable_interrupts() noexcept;
```
These functions enable [interrupt controller](interrupt.md) interrupts related to the specified EXTI lines. `LineInterruptOptions` must be a list of `mcutl::exti::line_interrupt_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type. If any priorities or subpriorities are set by the `LineInterruptOptions`, these priorities and subpriorities will be applied to the interrupts before enabling. Note that some EXTI lines may share the same interrupt controller interrupt request line (`IRQn`). Such EXTI lines priorities and subpriorities must be the same, this will be checked during compilation.

These functions may be non-atomic, although they will internally call `mcutl::interrupt::enable_atomic` for each separate interrupt if the target MCU supports it (see [mcutl/interrupt](interrupt.md) for details) and no priorities are set. Othrerwise, `mcutl::interrupt::enable` will be called internally. If you intend to enable **all** of the interrupt controller interrupts atomically, you need to perform additional locking.

You need to provide `PriorityCount` additionally if you explicitly changed this value when calling `mcutl::interrupt::initialize_controller`.

### disable_interrupts
```cpp
template<typename... LineInterruptOptions>
void disable_interrupts() noexcept;
```
This function disables [interrupt controller](interrupt.md) interrupts related to the specified EXTI lines. `LineInterruptOptions` must be a list of `mcutl::exti::line_interrupt_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type.

This function may be non-atomic, although it will internally call `mcutl::interrupt::disable_atomic` for each separate interrupt if the target MCU supports it (see [mcutl/interrupt](interrupt.md) for details). Othrerwise, `mcutl::interrupt::disable` will be called internally. If you intend to disable **all** of the interrupt controller interrupts atomically, you need to perform additional locking.

### set_interrupt_prioritites
```cpp
template<typename... LineInterruptOptions>
void set_interrupt_prioritites() noexcept;
template<auto PriorityCount, typename... LineInterruptOptions>
void set_interrupt_prioritites() noexcept;
```

These functions set the priorities of the [interrupt controller](interrupt.md) interrupts related to the specified EXTI lines. `LineInterruptOptions` must be a list of `mcutl::exti::line_interrupt_options` types, optionally wrapped in the `mcutl::exti::config` wrapper type. Note that some EXTI lines may share the same interrupt controller interrupt request line (`IRQn`). Such EXTI lines priorities and subpriorities must be the same, this will be checked during compilation.

You need to provide `PriorityCount` additionally if you explicitly changed this value when calling `mcutl::interrupt::initialize_controller`.

## Connecting GPIO pins to EXTI lines
If you use a hardware trigger on the EXTI line (`falling` or `rising`, for instance), you need to connect the GPIO ports to the lines you have configured and enabled. Please refer to the [main GPIO documentation page](gpio.md) to learn how to configure the GPIO pins. There are two additional GPIO configuration structs that allow the GPIO pins to connect to and disconnect from the EXTI lines:
```cpp
//Defined in the mcutl::gpio namespace
template<typename Pin, uint32_t ExtiLine = default_exti_line, typename... Options>
struct connect_to_exti_line : detail::exti_line_options<Pin, ExtiLine, Options...> {};
template<typename Pin, uint32_t ExtiLine = default_exti_line, typename... Options>
struct disconnect_from_exti_line : detail::exti_line_options<Pin, ExtiLine, Options...> {};
```
The first one is used to connect the GPIO pin to the EXTI line, the second one - to disconnect them.
* `Pin` is the GPIO pin type (`mcutl::pin` type)
* `ExtiLine` is the target EXTI line index or the `default_exti_line` value. If the `default_exti_line` value is supplied, the default EXTI line index for the target pin will be selected. If the pin can be connected to several lines, a compile-time error is generated.
* `Options` are optional device-specific additional settings.

This is an example of how to connect a GPIO pin to its default EXTI line:
```cpp
using gpio_config = mcutl::gpio::config<
	mcutl::gpio::as_input<mcutl::gpio::gpioa<2>, mcutl::gpio::in::pull_up>,
	mcutl::gpio::connect_to_exti_line<mcutl::gpio::gpioa<2>>,
	mcutl::gpio::enable_peripherals
>;
mcutl::gpio::configure_gpio<gpio_config>();
```
The target GPIO pin must be configured as an input in general, but some MCUs may allow output pins to be connected to EXTI lines (to allow software external interrupt triggering). The `enable_peripherals` option will enable not only the required pin peripheral for the `gpioa<2>` pin, but also the EXTI controller if this is required for the MCU (for example, the `afio` peripheral in case of STM32F1 MCUs). For some MCUs, `connect_to_exti_line` may be a no-op, as the pin is connected to the EXTI line by default, and all that remains is to configure and enable the EXTI line itself.

`disconnect_from_exti_line` may be a no-op for the MCU of your choice, but the `enable_peripherals` will still enable the EXTI peripheral of the MCU.

The target MCU may have several different GPIO pins connectable to the same EXTI line by default. In this case, if you specify several of these pins in a single `configure_gpio` call with the `connect_to_exti_line` or the `disconnect_from_exti_line` option, you will get a compile-time conflict error, because several pins can not be connected to or disconnected from the same line.

## default_exti_line_type
```cpp
//Defined in the mcutl::gpio namespace
template<typename Pin>
using default_exti_line_type = ...;
```
This is a typedef of the `mcutl::line` type for a default EXTI line for the GPIO pin `Pin`. This typedef is not valid if the `Pin` can be connected to several different EXTI lines.

## exti_lines_bit_mask_v
```cpp
template<typename... PinConfig>
constexpr auto exti_lines_bit_mask_v = ...;
```
This constexpr variable represents a bitmask containing all the EXTI lines for the pins from the `PinConfig`. `PinConfig` can be either a set of `mcutl::pin` types, or a set of GPIO configuration options optionally wrapped in the `mcutl::gpio::config` wrapper type. This is an example of using this constant:
```cpp
constexpr bool lines_match = mcutl::gpio::exti_lines_bit_mask_v<
		mcutl::gpio::gpiof<15>, mcutl::gpio::gpioa<3>>
	== mcutl::exti::lines_bit_mask_v<mcutl::exti::line<3>, mcutl::exti::line<15>>;
```

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific EXTI lines
All of these MCUs provide lines with indices `0` to `18` inclusive. Connectivity line devices (STM32F105, STM32F107) also provide line `19`. These controllers introduce additional definitions to the `mcutl::exti` namespace:
* `pvd_output_line` - programmable Voltage Detector external interrupt (line `16`)
* `rtc_alarm_line_index` - real-time clock alarm external interrupt (line `17`)
* `usb_wakeup_line_index` - USB wakeup line external interrupt (line `18`)
* `ethernet_wakeup_line` - Ethernet wakeup line external interrupt (line `19`, connectivity line devices only)

STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 MCUs do not provide any device-specific line modes or triggers. They do not provide any device-specific options for `line_options` or `line_interrupt_options`.
