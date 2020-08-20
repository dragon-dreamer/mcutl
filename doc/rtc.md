# MCU real-time clock (RTC) configuration (mcutl/rtc/rtc.h)
This header provides facilities to configure and use the MCU real-time clock (if supported by the MCU). All of the definitions are in the `mcutl::rtc` namespace, unless otherwise stated.

## Public definitions
```cpp
template<bool Enable>
struct enable {};
```
This struct can be used to disable or enable the RTC when configuring.

```cpp
struct base_configuration_is_currently_present {};
```
This structure can be used when configuring the RTC (using the `mcutl::rtc::configure` function) to indicate that the RTC is being configured the for the first time after the MCU power-on, and the related configuration is at reset state. This switch will allow the `mcutl::rtc::configure` function to skip a bunch of excessive checks.

```cpp
namespace interrupt
{
struct alarm {};
struct second {};
struct overflow {};

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};
} //namespace interrupt
```
These structs are used to configure the common RTC interrupts:
* `alarm` - alarm interrupt;
* `second` - second interrupt, which is triggered every second;
* `overflow` - overflow interrupt, which is triggered when the RTC counter overflows.

Some MCUs may support other interrupt types. Some MCUs may not support all of these interrupts. There are several traits which can be used to determine the MCU RTC capabilities (described below).

You can specify the `enable_controller_interrupts` and/or the `disable_controller_interrupts` struct when configuring or reconfiguring the RTC to enable or disable the [interrupt controller](interrupt.md) interrupts related to the RTC interrupts. The `enable_controller_interrupts` switch will enable all the required for the selected configuration interrupts. The `disable_controller_interrupts` switch will disable the interrupt controller interrupts which are not used in the current RTC configuration, leaving all used interrupts enabled.

To enable an interrupt, specify the related structure in the RTC configuration (see below for examples). To enable an interrupt and set its priority/subpriority, use `mcutl::interrupt::interrupt` [wrapper](interrupt.md). To disable an interrupt, wrap the related interrupt structure into the `mcutl::interrupt::disabled` wrapper structure.

Add the `mcutl::interrupt::priority_count` option to explicitly set the interrupt priority count. Use this option if you explicitly changed the interrupt priority count when calling `mcutl::interrupt::initialize_controller`.

```cpp
namespace clock
{
struct internal {};
struct external_crystal {};

template<uint32_t PrescalerValue>
struct prescaler {};

struct one_second_prescaler {};
} //namespace clock
```
These structs are used to configure the RTC clock source and the prescaler:
* `internal` - use the internal RTC clock source;
* `external_crystal` - use the external RTC clock source (usually the `32768 Hz` crystal);
* `prescaler` - set the RTC clock prescaler value;
* `one_second_prescaler` - set the RTC clock prescaler so that the RTC counter frequency will be exactly `1 Hz`.

Some MCUs may support other clock source types. Some MCUs may not support all of these common clock sources. There are several traits which can be used to determine the MCU RTC capabilities (described below).

```cpp
template<typename Interrupt>
using interrupt_type = ...;
```
This typedef indicates a corresponding [interrupt controller](interrupt.md) interrupt type for the specified RTC interrupt. Some RTC interrupts may map to a single interrupt controller interrupt.

```cpp
using peripheral_type = ...;
```
This typedef indicates a corresponding [peripheral](peripheral.md) type or type list (or `mcutl::periph::no_periph`) for the RTC. This peripheral must be enabled before accessing the RTC.

## Traits
There are several traits that can be used to determine the MCU RTC capabilities:
```cpp
//True if the target MCU supports RTC clock prescalers
constexpr bool supports_prescalers = ...;
//True if the target MCU supports RTC alarms (including the alarm interrupt)
constexpr bool supports_alarm = ...;
//True if the target MCU supports the RTC second interrupt
constexpr bool supports_second_interrupt = ...;
//True if the target MCU supports the RTC overflow interrupt
constexpr bool supports_overflow_interrupt = ...;
//True if the target MCU supports RTC internal clock source
constexpr bool supports_internal_clock_source = ...;
//True if the target MCU supports the clear_pending_flags_atomic function
constexpr bool supports_atomic_clear_pending_flags = ...;
//True if the target MCU supports the RTC clock source reconfiguration using
//the mcutl::rtc::reconfigure function
constexpr bool supports_clock_source_reconfiguration = ...;

//This constant indicates if the specified RTC clock prescaler supported
template<uint32_t PrescalerValue>
constexpr bool prescaler_supported = ...;

//These constants indicate the minimum and the maximum valid RTC prescaler values
constexpr auto min_prescaler = ...;
constexpr auto max_prescaler = ...;
```

## configure
```cpp
template<typename... Options>
void configure() noexcept;
```
This function is used to configure the RTC without keeping its existing configuration. It has several required options: the RTC prescaler (if supported by the MCU) and the RTC clock source type. Here are several examples of RTC configuration:

```cpp
//Enable the RTC, set its clock source to internal and
//the prescaler to one-second prescaler. Disables all
//RTC interrupts (as no interrupts specified).
mcutl::rtc::configure<
	mcutl::rtc::clock::internal,
	mcutl::rtc::clock::one_second_prescaler,
	mcutl::rtc::enable<true>
>();
```

```cpp
//Configure the RTC (but do not enable it):
//- Set its clock source to external
//- Set its prescaler to the 12345 value
//- Indicate that the RTC reset configuration is currently present, which
//  will allow the configure function to skip some checks and actions
//- Enable the alarm interrupt and set its priority to 3
//- Enable the second interrupt
//- Disable all other RTC interrupts, which are not listed in this call
//- Enable and/or disable the related interrupt controller interrupts
mcutl::rtc::configure<
	mcutl::rtc::clock::external_crystal,
	mcutl::rtc::clock::prescaler<12345>,
	mcutl::rtc::base_configuration_is_currently_present,
	mcutl::interrupt::interrupt<mcutl::rtc::interrupt::alarm, 3>,
	mcutl::rtc::interrupt::second,
	mcutl::rtc::interrupt::enable_controller_interrupts,
	mcutl::rtc::interrupt::disable_controller_interrupts
>();
```

## reconfigure
```cpp
template<typename... Options>
void reconfigure() noexcept;
```
This function is used to reconfigure some of the RTC options while keeping other options as is. The `base_configuration_is_currently_present` option can not be present with this function. Here are several examples of RTC reconfiguration:

```cpp
//Disable the RTC
mcutl::rtc::reconfigure<
	mcutl::rtc::enable<false>
>();
```

```cpp
//Change the RTC clock prescaler to 123
mcutl::rtc::reconfigure<
	mcutl::rtc::clock::prescaler<123>
>();
```

```cpp
//Disable the alarm interrupt and also disable all unneeded related
//interrupt controller interrupts
mcutl::rtc::reconfigure<
	mcutl::rtc::interrupt::disable_controller_interrupts,
	mcutl::interrupt::disabled<mcutl::rtc::interrupt::alarm>
>();
```

```cpp
//Disable the second interrupt, enable the alarm interrupt and set its
//priority to 2. Also enable all required RTC-related interrupt controller
//interrupts and disable all unneeded RTC-related interrupt controller interrupts
mcutl::rtc::reconfigure<
	mcutl::rtc::interrupt::disable_controller_interrupts,
	mcutl::rtc::interrupt::enable_controller_interrupts,
	mcutl::interrupt::disabled<mcutl::rtc::interrupt::second>,
	mcutl::interrupt::interrupt<mcutl::rtc::interrupt::alarm, 2>
>();
```

Note that some MCUs may not support the RTC clock source reconfiguring (see the `supports_clock_source_reconfiguration` trait).

## clear_pending_flags, clear_pending_flags_atomic
```cpp
template<typename... Interrupts>
void clear_pending_flags() noexcept;
template<typename... Interrupts>
void clear_pending_flags_atomic() noexcept;
```
These functions can be used to reset the pending RTC `Interrupts` flags, so that the interrupt will not fire again after being handled. The `clear_pending_flags_atomic` is available only when the `supports_atomic_clear_pending_flags` is `true`.

## is_enabled
```cpp
bool is_enabled() noexcept;
```
Returns `true` if the RTC is enabled and working.

## has_alarmed
```cpp
template<typename T = void>
bool has_alarmed() noexcept;
```
Returns `true` if there is currently a pending RTC alarm flag. Do not explicitly specify the `T` template parameter, as it is only used to make the function compile on demand. This function is available when the `supports_alarm` trait is `true`.

## get_pending_flags, pending_flags_v
```cpp
template<typename... Interrupts>
auto get_pending_flags() noexcept;
template<typename... Interrupts>
constexpr auto pending_flags_v = ...;
```
The `get_pending_flags` function can be used to get the currently pending RTC interrupt flags. To determine which interrupts are currently pending, use the `pending_flags_v` constant, for example:

```cpp
auto pending_flags = mcutl::rtc::get_pending_flags<
	mcutl::rtc::interrupt::alarm,
	mcutl::rtc::interrupt::overflow,
	mcutl::rtc::interrupt::second
>()),

bool alarm_pending = (pending_flags
	& mcutl::rtc::pending_flags_v<mcutl::rtc::interrupt::alarm>) != 0;
```

## disable_alarm
```cpp
template<typename T = void>
void disable_alarm() noexcept;
```
This function disables the RTC alarm and clears the pending RTC alarm interrupt flag. It does not clear the related interrupt controller interrupt, if it is pending. Do not explicitly specify the `T` template parameter, as it is only used to make the function compile on demand. This function is available when the `supports_alarm` trait is `true`.

## set_date_time
```cpp
template<typename DateTimeOrTimeStamp>
void set_date_time(const DateTimeOrTimeStamp& value) noexcept;
```
This function sets the RTC internal counter to the specified date and time or timestamp value. `DateTimeOrTimeStamp` may be the following:
* An integral type. In this case it is treated as the 32-bit timestamp value ([see](datetime.md) the `mcutl::datetime::timestamp_t` type defined in `utils/datetime.h`).
* A `mcutl::datetime::date_time` type ([defined](datetime.md) in `utils/datetime.h`).
* Any structure or class containing the following integral public fields: `second`, `minute`, `hour`, `day`, `month`, `year`. The valid value ranges for these fields are described [here](datetime.md).

## set_alarm
```cpp
template<typename DateTimeOrTimeStamp>
void set_alarm(const DateTimeOrTimeStamp& value) noexcept;
```
This function can be used to set the RTC alarm to the specific date and time or timestamp. For the `DateTimeOrTimeStamp` type requirements, see the `set_date_time` description above. This function is available when the `supports_alarm` trait is `true`.

## get_date_time
```cpp
template<typename DateTime>
void get_date_time(DateTime& value) noexcept;
```
Returns the date, time and/or the timestamp of the RTC. `DateTime` may be the following:
* A `mcutl::datetime::date_time` type ([defined](datetime.md) in `utils/datetime.h`).
* Any structure or class containing at least one of the following integral public fields: `second`, `minute`, `hour`, `day`, `month`, `year`, `timestamp`. The valid value ranges for these fields are described [here](datetime.md). There are no requirements to the exact set of fields, you can make your own structure containing the fields you are going to request. For example, the following structure will do:

```cpp
struct year_with_timestamp
{
	uint16_t year;
	uint32_t timestamp;
	char something;
};
```
The `get_date_time` function will correctly fill the `year` and the `timestamp` fields while leaving the `something` fiels as is.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific description
There are several additional RTC clock sources supported by these MCUs:
```cpp
namespace mcutl::rtc::clock
{
using lse_crystal = external_crystal;
using lsi = internal;
struct lse_bypass {};
struct hse_div_128 {};
} //namespace mcutl::rtc::clock
```
The first two are the aliases to the common clock source types. The last two are device-specific.

There is also the device-specific `mcutl::rtc::interrupt::external_alarm` interrupt, which can be used to enable the external RTC alarm interrupt via the related [EXTI](exti.md) line. The `mcutl::rtc::configure` or the `mcutl::rtc::reconfigure` function will configure that line for you when you specify this interrupt type. This interrupt allows the RTC to wake the MCU from sleep modes. It shares the RTC interrupt flag and enable bit with the `alarm` interrupt.

Note that the `supports_clock_source_reconfiguration` is `false` for these controllers. This means, you can not reconfigure the RTC clock source using the `mcutl::rtc::reconfigure` function. However, you are allowed to configure the RTC clock source using the `mcutl::rtc::configure` function any number of times.

**Important note:** once the RTC clock source is selected and you change it to another clock source type by calling the `mcutl::rtc::configure` function, **this will reset the whole [backup](backup.md) domain, including the backup registers contents**! If you keep the clock source unchanged after it has been selected, this will not happen.
