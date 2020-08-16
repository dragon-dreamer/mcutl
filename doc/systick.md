# MCU SYSTICK timer configuration and access (mcutl/systick/systick.h)
This header provides facilities to configure and use the MCU SYSTICK timer facilities (if supported by the MCU). All of the definitions are in the `mcutl::systick` namespace, unless otherwise stated.

## Public definitions
```cpp
template<bool Enable>
struct enable {};
```
This struct is used to enable or disable the SYSTICK timer.

```cpp
namespace clock_source
{
struct processor {};
struct external {};
} //namespace clock_source
```
These structs may be used to set the clock source for the SYSTICK timer: either `processor` (MCU core clock, which is the default) or `external`. Some MCUs may not support all of these options or support more clock sources.

```cpp
namespace interrupt
{
struct tick {};
} //namespace interrupt
```
This is a common interrupt which fires when the SYSTICK timer overflows (or underflows, depending on count direction). The `tick` struct is used to enable or disable the interrupt when configuring the SYSTICK timer. To enable an interrupt and set its priority/subpriority, use `mcutl::interrupt::interrupt` [wrapper](interrupt.md). To disable an interrupt, wrap the related interrupt structure into the `mcutl::interrupt::disabled` wrapper structure.

Add the `mcutl::interrupt::priority_count` option to explicitly set the interrupt priority count. Use this option if you explicitly changed the interrupt priority count when calling `mcutl::interrupt::initialize_controller`.

Some MCUs may support more SYSTICK interrupts.

```cpp
template<typename Interrupt>
using interrupt_type = ...;
```
This typedef indicates a corresponding [interrupt controller](interrupt.md) interrupt type for the specified SYSTICK interrupt.

```cpp
template<typename Channel>
using peripheral_type = ...;
```
This typedef indicates a corresponding [peripheral](peripheral.md) type (or `mcutl::periph::no_periph`) for the SYSTICK hardware. This peripheral must be enabled before accessing SYSTICK.

```cpp
using value_type = device::systick::value_type;
```
This is the type wide enough to hold the SYSTICK value.

## Traits
There are several traits to determine the capabilities of the SYSTICK for the MCU of your choice:
```cpp
//SYSTICK counting direction
enum class direction_type
{
	top_down,
	bottom_up
};

//mcutl::systick::direction_type, indicates SYSTICK counting direction
constexpr auto count_direction = ...;
//Minimum possible SYSTICK value
constexpr auto min_value = ...;
//Maximum possible SYSTICK value
constexpr auto max_value = ...;
//True if the MCU SYSTICK supports overflow detection
constexpr auto supports_overflow_detection = ...;
//True if the MCU SYSTICK clears overflow flag after reading it
constexpr auto overflow_flag_cleared_on_read = ...;
//True if the MCU SYSTICK supports value requests
constexpr auto supports_value_request = ...;
//True if the MCU SYSTICK supports reading the reload value
constexpr auto supports_reload_value = ...;
//True if the MCU SYSTICK supports changing the reload value
constexpr auto supports_reload_value_change = ...;
//True if the MCU SYSTICK supports value reset
constexpr auto supports_reset_value = ...;
//True if the MCU supports clear_pending_flags_atomic function
constexpr auto supports_atomic_clear_pending_flags = ...;
```

## configure
```cpp
template<typename... Options>
void configure() noexcept;
```
This function is used to configure the SYSTICK. It configures the SYSTICK, resetting all of its options to their default values and setting the explicitly specified option values. Here are several examples:

Enable the systick and set its clock source to `external`:
```cpp
mcutl::systick::configure<
	mcutl::systick::clock_source::external,
	mcutl::systick::enable<true>
>();
```

Disable the systick (as no `enable` option specified), set its clock source to `processor` (as the `clock_source` option is absent) and enable its `tick` interrupt with priority `3` and subpriority `5`:
```cpp
mcutl::systick::configure<
	mcutl::interrupt::interrupt<mcutl::systick::interrupt::tick, 3, 5>
>();
```

## reconfigure
```cpp
template<typename... Options>
void reconfigure() noexcept;
```
This function is used to reconfigure the SYSTICK. It reconfigures the SYSTICK, keeping all of its options and changing the explicitly specified option values. Here are several examples:

Keep all the SYSTICK options as is and disable its `tick` interrupt:
```cpp
mcutl::systick::configure<
	mcutl::interrupt::disable<mcutl::systick::interrupt::tick>
>();
```

Keep all the SYSTICK options as is and disable it:
```cpp
mcutl::systick::reconfigure<
	mcutl::systick::enable<false>
>();
```

## clear_pending_flags, clear_pending_flags_atomic
```cpp
template<typename... Interrupts>
void clear_pending_flags() noexcept;
template<typename... Interrupts>
void clear_pending_flags_atomic() noexcept;
```
These functions are used to clear the pending `Interrupts` flags to prevent the interrupt from firing again in an infinite loop. The `clear_pending_flags_atomic` function is atomic and thus requires no locking. It is available when the `supports_atomic_clear_pending_flags` trait is `true`.

## get_value
```cpp
template<typename T = void>
auto get_value() noexcept;
```
Returns current SYSTICK value. Available when the `supports_value_request` trait is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## reset_value
```cpp
template<typename T = void>
void reset_value() noexcept;
```
Resets the SYSTICK value to its default. Available when the `supports_reset_value` trait is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## has_overflown
```cpp
template<typename T = void>
bool has_overflown() noexcept;
```
Returns `true` if the SYSTICK value has overflown. Available when the `supports_overflow_detection` is `true`. Note that when the `overflow_flag_cleared_on_read` is `true`, this call will clear the overflow flag. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## clear_overflow_flag
```cpp
template<typename T = void>
void clear_overflow_flag() noexcept;
```
Clears the SYSTICK overflow flag. Available when the `supports_overflow_detection` is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## get_reload_value
```cpp
template<typename T = void>
auto get_reload_value() noexcept;
```
Returns the SYSTICK reload value. This value is loaded to SYSTICK every time when it underflows (or the value the SYSTICK count up to). Available when the `supports_reload_value` trait is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## get_default_reload_value
```cpp
template<typename T = void>
constexpr auto get_default_reload_value() noexcept;
```
Returns the SYSTICK default reload value. This value is loaded to SYSTICK every time when it overflows (or the value the SYSTICK count up to). Available when the `supports_reload_value` trait is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## set_reload_value
```cpp
template<typename T = void>
void set_reload_value(value_type value) noexcept;
```
Sets the SYSTICK reload value. This value is loaded to SYSTICK every time when it overflows (or the value the SYSTICK count up to). Available when the `supports_reload_value_change` trait is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.

## reset_reload_value
```cpp
template<typename T = void>
void reset_reload_value() noexcept;
```
Resets the SYSTICK reload value to its default. This value is loaded to SYSTICK every time when it overflows (or the value the SYSTICK count up to). Available when the `supports_reload_value_change` trait is `true`. Do not specity the `T` argument, it's used to make the function compile only when explicitly used.
