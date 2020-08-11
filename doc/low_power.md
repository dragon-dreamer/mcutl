# MCU low-power modes support (mcutl/low_power/low_power.h)
This header provides facilities to use low-power MCU modes. All of the definitions are in the `mcutl::low_power` namespace, unless otherwise stated. Low-power modes can be very different for different controllers, and this header aims to more or less unify the interface for entering and exiting these modes.

## Public definitions
```cpp
namespace wakeup_mode
{
struct wait_for_event {};
struct wait_for_interrupt {};
} //namespace wait_mode
```
These are the available wakeup modes: `wait_for_event` indicates that the MCU should sleep until the event is raised, `wait_for_interrupt` indicates the MCU should sleep until the interrupt occurs. Some MCUs may not support these modes, and there are several traits (described later) that allow to determine the capabilities of the target MCU. Some MCUs may provide additional device-specific modes. Which event or interrupt wakes up the MCU is entirely specific to that MCU.

```cpp
namespace sleep_mode
{
struct core_stop {};
struct adc_noise_reduction {};
struct stop_all_clocks {};
struct power_off {};
} //namespace mode
```
These are several common sleep modes. Some MCUs may not support these modes, and there are several traits (described later) that allow to determine the capabilities of the target MCU. Some MCUs may provide additional device-specific modes.
* `core_stop` - stop the core and flash memory clocks only. The fastest mode to exit, but saves least power.
* `adc_noise_reduction` - sleep for the time of the ADC conversion to reduce the ADC noise.
* `stop_all_clocks` - stop all MCU clocks, possibly leaving some critical on-board devices active.
* `power_off` - the deepest sleep mode: turn off everything, possibly leaving some critical on-board devices active. RAM contents is reset on wakeup. [Backup domain registers](backup.md) may keep the data after wakeup.

### Traits
```cpp
//True if the target MCU supports 'wait_for_event' wakeup mode
constexpr bool supports_wait_for_event = ...;
//True if the target MCU supports 'wait_for_interrupt' wakeup mode
constexpr bool supports_wait_for_interrupt = ...;

//True if the target MCU supports 'core_stop' sleep mode
constexpr bool supports_core_stop = ...;
//True if the target MCU supports 'adc_noise_reduction' sleep mode
constexpr bool supports_adc_noise_reduction = ...;
//True if the target MCU supports 'stop_all_clocks' sleep mode
constexpr bool supports_stop_all_clocks = ...;
//True if the target MCU supports 'power_off' sleep mode
constexpr bool supports_power_off = ...;

//Peripheral type to enable before using low-power modes, or
//mcutl::periph::no_periph.
using peripheral_type = ...;
```

### sleep
```cpp
template<typename SleepMode, typename WakeUpMode, typename... Options>
void sleep() noexcept;
```
This function puts the MCU to the selected low-power mode. `SleepMode` is the low-power mode, `WakeUpMode` is the wakeup mode, `Options` are optional device-specific low-power settings. All such options are always defined in the `mcutl::low_power::options` namespace. The MCU will transition to the selected low-power mode if all the conditions met (for example, there are no pending interrupts for STM32F1), otherwise, this function may return immediately.

### reset_sleep_flags
```cpp
template<typename SleepMode, typename WakeUpMode, typename... Options>
void reset_sleep_flags() noexcept;
```
This function resets all sleep flags that were set when calling the `sleep` function. You don't need to call this function is most cases, as all the flags are automatically reset by the `sleep` function itself. However, there are scenarios when the `sleep` function leaves some low-power flags configured. For example, some devices support the sleep-on-exit option, which transitions the MCU to the low-power mode when returning from the interrupt service routine (ISR). In this case, the `sleep` call will return immediately and leave some flags configured to put the device to sleep when returning from the ISR. When the device wakes up, sometimes you need these flags to be cleared (otherwise the MCU may sleep again when returning from the ISR), and they can be reset by calling the `reset_sleep_flags` function. For other scenarios, when the device transitions to the low-power mode **inside** the `sleep` function, you don't need to call `reset_sleep_flags` afterwards, as this is done for you.

The `SleepMode`, `WakeUpMode` and `Options` arguments must match the corresponding `sleep` function call.

## Cortex-M3 MCUs specific low-power options
```cpp
namespace mcutl::low_power::options
{
struct sleep_on_exit {};
struct event_on_pending_interrupt {};
} //namespace mcutl::low_power::options
```
These are the device-specific options for all Cortex-M3 MCUs.
* `sleep_on_exit` - Enter the low-power mode when returning from the interrupt service routine (ISR). This mode allows the MCU to only process interrupts and never execute non-interrupt context code. This option is supported by only the `core_stop` sleep mode.
* `event_on_pending_interrupt` - Trigger the event, when the interrupt is pending. The interrupt may be disabled or unsufficient priority, and this still will trigger the event. This option is compatible with the `wait_for_event` wakeup mode only.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific low-power options
```cpp
namespace mcutl::low_power::options
{
struct voltage_regulator_low_power_stop {};
struct debug_support {};
} //namespace mcutl::low_power::options
```
* `voltage_regulator_low_power_stop` - Put the embedded MCU voltage regulator to low-power mode when sleeping. This extends wakeup time, but saves energy. This option is compatible with `stop_all_clocks` and `power_off` sleep modes.
* `debug_support` - Set this option for debugging purposes to be able to still debug the MCU when it is sleeping.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 sleep mode aliases
There are some device-specific aliases for the sleep modes for these MCUs:
```cpp
namespace mcutl::low_power::sleep_mode
{
using sleep = core_stop;
using stop = stop_all_clocks;
using standby = power_off;
} //namespace mcutl::low_power::sleep_mode
```
These aliases associate the common low-power sleep mode definitions with the modes specific to these MCUs. The `supports_adc_noise_reduction` low-power mode is not supported by these MCUs.
