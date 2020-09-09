# MCU timer configuration and access (mcutl/timer/timer.h)
This header provides facilities to configure and use the MCU timers (if supported by the MCU). All of the definitions are in the `mcutl::timer` namespace, unless otherwise stated.

## Public definitions
There is a number of options which may be used when configuring a timer.

```cpp
namespace direction
{
struct up;
struct down;
} //namespace direction
```
These structures define the counting direction of a timer. The default direction may be determined using the `default_count_direction` trait (see below).

---

```cpp
template<bool Stop>
struct stop_on_overflow;
```
This structure enables or disables the one-time shot mode for a timer. To check if this mode is available, use the `supports_stop_on_overflow` trait (see below).

---

```cpp
template<uint64_t Value>
struct prescaler;
```
This structure sets the prescaler of a timer counter. To determine if this feature is available, use the `supports_prescalers` trait. To determine if the prescaler value is supported by the timer, use the `prescaler_supported` trait (see below).

---

```cpp
template<uint64_t Value>
struct reload_value;
```
This structure sets the reload value for a timer. This value determines the boundary value for the timer counter. If the timer upcounts, it will count from `0` to its reload value (inclusive). If the timer downcounts, it will count from the reload value to `0` (inclusive). For other device-specific timer modes, there is no universal meaning. To determine if a timer supports reload values, use the `supports_reload_value` trait. To determine the limits of the reload value, use the `reload_value_limits` trait. To determine the default reload value, use the `default_reload_value` trait (see below).

---

```cpp
template<bool Enable>
struct enable;
```
This structure can be used to enable or disable (start or stop) the timer operation.

---

```cpp
template<bool Enable>
struct enable_peripheral;
```
This structure can be used to enable or disable the timer peripheral when configuring. You can configure the timer peripheral manually. To determine the timer peripheral, use the `peripheral_type` trait (see below).

---

```cpp
template<bool Buffer>
struct buffer_reload_value;
```
This structure can be used to change the timer behavior when changing the reload value. If the buffered reload value is being changed, it will be used by the timer only after the next overflow or underflow. If the unbuffered reload value is being changed, it will be used by the timer instantly. To determine if a timer supports reload values, use the `supports_reload_value` trait. To determine if the reload value can be buffered, use the `supports_reload_value_buffer` trait (see below).

---

```cpp
struct base_configuration_is_currently_present;
```
This structure can be used with the `configure` function to indicate that the timer is being configured for the first time (i.e. the reset configuration is present). This may allow the library to optimize out some excessive checks and actions when configuring the timer.

---

```cpp
template<typename ClockConfig, typename FrequencyHz, typename MaxErrorHz>
struct timer_frequency;
template<typename ClockConfig, typename FrequencyHz>
using exact_timer_frequency = ...;
```
These two types allow to configure the timer counter frequency.
* `ClockConfig` is the [clock configuration](clock.md) of the MCU.
* `FrequencyHz` is the desired counter frequency in `Hertz` (`std::ratio` type).
* `MaxErrorHz` is the maximum possible error for the frequency in `Hertz` (`std::ratio` type).

This structure is used to calculate the best prescaler value for the timer automatically at compile time. To determine if this feature is available, use the `supports_prescalers` trait.

---

```cpp
template<typename ClockConfig, typename FrequencyHz, typename MaxErrorHz, bool ExtraPrecision = false>
struct overflow_frequency;
template<typename ClockConfig, typename FrequencyHz, bool ExtraPrecision = false>
using exact_overflow_frequency = ...;
```
These two types allow to configure the timer overflow or underflow frequency. You can use it to configure the frequency of the timer `overflow` interrupt.
* `ClockConfig` is the [clock configuration](clock.md) of the MCU.
* `FrequencyHz` is the desired overflow/underflow frequency in `Hertz` (`std::ratio` type).
* `MaxErrorHz` is the maximum possible error for the frequency in `Hertz` (`std::ratio` type).
* `ExtraPrecision` indicates if the calculation must be done with extra precision. This option may help calculate the values for some borderline cases, but the calculation will be slower.

This structure is used to calculate the best prescaler and reload values for the timer automatically at compile time. To determine if this feature is available, use both the `supports_prescalers` and the `supports_reload_value` traits.

---

```cpp
namespace interrupt
{
struct overflow;
struct enable_controller_interrupts;
struct disable_controller_interrupts;
} //namespace interrupt
```
This namespace contains a list of interrupts which are supported by the MCU timers. The `overflow` interrupt is raised when the timer counter overflows or underflows.

To enable an interrupt, specify the related structure in the timer configuration (see below for examples). To enable an interrupt and set its priority/subpriority, use `mcutl::interrupt::interrupt` [wrapper](interrupt.md). To disable an interrupt, wrap the related interrupt structure into the `mcutl::interrupt::disabled` wrapper structure. To additionally enable or disable the related interrupt controllers interrupts, use the `enable_controller_interrupts` or `disable_controller_interrupts` structs.

The `enable_controller_interrupts` switch will enable all the required for the selected configuration interrupts. The `disable_controller_interrupts` switch will disable the interrupt controller interrupts which are not used in the current timer configuration, leaving all used interrupts enabled.

Add the `mcutl::interrupt::priority_count` option to explicitly set the interrupt priority count. Use this option if you explicitly changed the interrupt priority count when calling `mcutl::interrupt::initialize_controller`.

## Timer traits
There are several traits to determine the characteristics and capabilities of the timer. All traits below require the `Timer` template parameter, which is a device-specific type to define the timer (such as `mcutl::timer::timer1` or `mcutl::timer::timer2`, for example).

---

```cpp
template<typename Timer>
using peripheral_type = typename detail::timer_traits<Timer>::peripheral;
```
This typedef indicates the [peripheral type](periph.md) which must be enabled before the timer configuration or conversions. This may also be a `mcutl::types::list` list of peripherals, or the `mcutl::periph::no_periph` type. This peripheral can be enabled or disabled automatically when configuring using the `enable_peripheral` option (see above).

---

```cpp
template<typename Timer>
using counter_type = typename detail::timer_traits<Timer>::counter_type;
```
This typedef indicates the counter type of the `Timer`. This is an unsigned integer type sufficient to keep the timer counter value. For example, for a 16-bit timer, this type may be `uint16_t`.

---

```cpp
template<typename Timer>
constexpr auto max_value = detail::timer_traits<Timer>::max_value;
```
This constant indicates the maximum possible timer counter value.

---

```cpp
template<typename Timer, typename Interrupt>
using interrupt_type = typename detail::interrupt_type_helper<Timer, Interrupt>::type;
```
This typedef indicates the [interrupt controller interrupt type](interrupt.md) of the related `Timer` `Interrupt`. You may pass `mcutl::timer::interrupt::overflow` or other timer interrupt type to the `Interrupt` parameter.

---

```cpp
template<typename Timer, uint64_t Prescaler>
constexpr bool prescaler_supported = ...;
```
This constant indicates if the `Prescaler` is supported for the `Timer`. This trait is available only if the `supports_prescalers` is `true`.

---

```cpp
template<typename Timer>
using reload_value_limits = typename detail::timer_traits<Timer>::reload_value_limits;
```
This type indicates the limits of the timer reload value. It is avaliable only when the `supports_reload_value` trait is `true`. It has the type of `mcutl::types::limits<From, To>`.

### Capabilities
There are some traits which indicate the capabilities of the timer:

```cpp
//Indicates if the timer supports the stop_on_overflow option
template<typename Timer>
constexpr bool supports_stop_on_overflow = ...;

//Indicates if the timer supports prescalers
template<typename Timer>
constexpr bool supports_prescalers = ...;

//Indicates if the timer supports reload values
template<typename Timer>
constexpr bool supports_reload_value = ...;

//Indicates if the timer supports the buffer_reload_value option
template<typename Timer>
constexpr bool supports_reload_value_buffer = ...;

//Indicates the default counting direction for the timer:
//It can be either mcutl::timer::count_direction::up,
//or mcutl::timer::count_direction::down,
//or mcutl::timer::count_direction::other.
template<typename Timer>
constexpr count_direction default_count_direction = ...;

//Indicates if the timer supports the clear_pending_flags_atomic function
template<typename Timer>
constexpr bool supports_atomic_clear_pending_flags = ...;

//Default timer reload value. This trait is available if the
//supports_reload_value trait is true.
template<typename Timer>
constexpr auto default_reload_value = ...;
```

## Timer configuration
There are two functions to configure the timer:
```cpp
template<typename Timer, typename... Options>
void configure() noexcept;
template<typename Timer, typename... Options>
void reconfigure() noexcept;
```
The first function configures the timer with the `Options` provided, resetting other timer settings to their default values. The second function keeps the existing configuration and alters only the `Options` provided. The `Timer` template parameter is a device-specific type to define the timer (such as `mcutl::timer::timer1` or `mcutl::timer::timer2`, for example). Here are several examples of the timer configuration:

```cpp
//Sample clock config with timers enables
using timer_clock_config = mcutl::clock::config<mcutl::clock::external_high_speed_crystal<8'000'000>,
	mcutl::clock::timer2_3_4_5_6_7_12_13_14<mcutl::clock::required_frequency<72'000'000>>>;

mcutl::timer::configure<mcutl::timer::timer2,
	mcutl::timer::direction::down,
	mcutl::timer::stop_on_overflow<true>,
	mcutl::timer::overflow_frequency<timer_clock_config, std::ratio<20>, std::ratio<1, 5>>,
	mcutl::timer::enable<true>,
	mcutl::timer::enable_peripheral<true>,
	mcutl::interrupt::interrupt<mcutl::timer::interrupt::overflow, 5>,
	mcutl::timer::interrupt::enable_controller_interrupts,
	mcutl::timer::base_configuration_is_currently_present>();
```
This call enables the `mcutl::timer::timer2` peripheral, sets the timer dorection to `down`, sets timer to stop on overflow, sets the timer overflow frequency to `20 Hz` with max error of `1/5 Hz`. It also enables the `overflow` interrupt and sets its priority to `5`, and enables the related [interrupt controller](interrupt.md) interrupt. The timer is then started. The `base_configuration_is_currently_present` option indicates that the timer is being configured for the first time after reset to optimize the configuration procedure. If this option was absent, all the unaltered timer options would have been reset to their default values.

```cpp
mcutl::timer::reconfigure<mcutl::timer::timer2,
	mcutl::timer::enable<false>>();
```
This call stops the `mcutl::timer::timer2` timer, keeping the existing timer configuration intact.

## Timer interrupt pending flags
There are several functions to check which timer interrupt pending flags are set, and to clear them.

```cpp
template<typename Timer, typename... Interrupts>
auto get_pending_flags() noexcept;
template<typename Timer, typename... Interrupts>
constexpr auto pending_flags_v = ...;
```
The `get_pending_flags` function can be used to get the currently pending `Timer` interrupt flags. To determine which interrupts are currently pending, use the `pending_flags_v` constant, for example:

```cpp
auto pending_flags = mcutl::timer::get_pending_flags<
	mcutl::timer::interrupt::overflow
>()),

bool alarm_pending = (pending_flags
	& mcutl::timer::pending_flags_v<mcutl::timer::interrupt::overflow>) != 0;
```

```cpp
template<typename Timer, typename... Interrupts>
void clear_pending_flags() noexcept;
template<typename Timer, typename... Interrupts>
void clear_pending_flags_atomic() noexcept;
```
These functions can be used to reset the pending `Timer` `Interrupts` flags, so that the interrupt will not fire again after being handled. The `clear_pending_flags_atomic` is available only when the `supports_atomic_clear_pending_flags` trait is `true`.

The `Timer` template parameter is a device-specific type to define the timer (such as `mcutl::timer::timer1` or `mcutl::timer::timer2`, for example).

## Timer counter access
There are several functions to access and modify the timer counter.

```cpp
template<typename Timer>
auto get_timer_count() noexcept;

template<typename Timer, typename Value>
void set_timer_count(Value value) noexcept;
```
The `get_timer_count` function returns the current `Timer` counter value. The `set_timer_count` sets the `Timer` counter `value`.

## STM32F1** specific timer options and definitions
There are the following device-specific timer types available for these MCUs:
* `mcutl::timer::timer1`, `mcutl::timer::timer2`, `mcutl::timer::timer3`, `mcutl::timer::timer4`, `mcutl::timer::timer5`
* `mcutl::timer::timer8` (high- and XL-density devices only)
* `mcutl::timer::timer9`, `mcutl::timer::timer10`, `mcutl::timer::timer11`, `mcutl::timer::timer12`, `mcutl::timer::timer13`, `mcutl::timer::timer14` (XL-density devices only)
* `mcutl::timer::timer6`, `mcutl::timer::timer7` (high- and XL-density devices and connectivity line devices only)

Currently, only the `mcutl::timer::timer2`, `mcutl::timer::timer3`, `mcutl::timer::timer4`, `mcutl::timer::timer5` timers are partially supported (overflow/underflow mode only).

---

There are several device-specific options for the timers:
```cpp
namespace update_request_source
{
struct overflow_ug_bit_slave_controller;
struct overflow;
} //namespace update_request_source
```
These structs can be used to set the timer update request source. The `overflow_ug_bit_slave_controller` options indicates that the timer update event must be generated on overflow/underflow, as well as setting the `UG` bit (see `trigger_registers_update` below) and the slave controller notification. By default, the `overflow_ug_bit_slave_controller` option is active.

---

```cpp
template<bool Disable>
struct disable_update;
```
This struct can be used to disable the timer update. No update events are generated, when the timer update is disabled.

---

```cpp
struct trigger_registers_update;
```
This struct can be used to instantly trigger the timer registers update. The prescaler and the reload values are updated instantly and not on the next update event. This struct will also cause the update event to be generated if the `overflow_ug_bit_slave_controller` option is active (which is by default).

---

```cpp
namespace master_mode
{
struct none;
struct output_enable;
struct output_update;
} //namespace master_mode
```
These structs can be used to configure the master mode of the timer. By default, the master mode is not active (`none`). The `output_enable` option sets the counter enable signal (which is generated when the timer is being started) to be used as trigger output. The `output_update` option sets the update event to be used as trigger output.

---

```cpp
namespace interrupt
{
using update = overflow;
} //namespace interrupt
```
This is a device-specific alias for the `overflow` interrupt.
