# MCU interrupt configuration (mcutl/interrupt/interrupt.h)
This header provides facilities to configure MCU interrupt system (or nested vectored interrupt controller for some MCUs). All of the definitions are in the `mcutl::interrupt` namespace, unless otherwise stated.

### Priority, IRQn
```cpp
using priority_t = int32_t;
using irqn_t = int32_t;
```
These definitions are used to represent the interrupt priority, subpriority, and the interrupt request number (`IRQn`). Some MCUs may not support these features, which can be detected by using traits (described later). Lower `priority_t` value means higher interrupt priority.

### Interrupt wrapper
MCU definitions provide device-specific interrupt structures (which are defined in the `mcutl::interrupt::type` namespace). There is a wrapper struct, which allows to wrap these structures to additionally set interrupt priority and subpriority (if supported by the target MCU):
```cpp
template<typename Type, priority_t Priority, priority_t Subpriority = default_priority>
struct interrupt
{
	using interrupt_type = Type;
	static constexpr auto priority = Priority;
	static constexpr auto subpriority = Subpriority;
};
```
Here `Type` is the device-specific interrupt struct, `Priority` is the desired priority value, `Subpriority` is the desired subpriority value (use `default_priority` to set it to its default value, usually `0`).

### Traits
There are some traits available to determine the capabilities of the interrupt controller of the target MCU:
```cpp
//True if the MCU supports interrupt priorities
constexpr bool has_priorities;
//True if the MCU supports interrupt subpriorities
constexpr bool has_subpriorities;
//True if the MCU supports is_interrupt_context function
constexpr bool has_interrupt_context;
//True if the MCU supports get_active function
constexpr bool has_get_active;
//Maximum priorities count for the target MCU
constexpr auto maximum_priorities;
//True if the MCU supports enable_atomic function
constexpr bool has_atomic_enable;
//True if the MCU supports disable_atomic function
constexpr bool has_atomic_disable;
//True if the MCU supports set_priority_atomic function
constexpr bool has_atomic_set_priority;
//True if the MCU supports clear_pending_atomic function
constexpr bool has_atomic_clear_pending;
```

### initialize_controller
```cpp
template<auto PriorityCount = maximum_priorities, typename... Options>
void initialize_controller() noexcept;
```
Initializes the interrupt controller. Call this function once before calling any other functions from the `mcutl::interrupt` namespace. For some MCUs, this call will do nothing. For controllers with interrupt priority support (such as STM32F1) it will set the maximum priority count for the controller. All other priority bitfield bits will remain to represent the interrupt subpriority.

By default, initializes the controller with maximum available priorities for the target MCU. This behavior may be changed using the `PriorityCount` parameter. `Options` are additional device-specific options for the tagret MCU.

### enable, enable_atomic
```cpp
template<typename Interrupt, auto PriorityCount = maximum_priorities>
void enable() noexcept;
```
Enables the interrupt `Interrupt`. You need to provide `PriorityCount` additionally if you explicitly changed this value when calling `initialize_controller`. `Interrupt` can be either the device-specific interrupt structure, or the `mcutl::interrupts::interrupt` wrapper. If `mcutl::interrupts::interrupt` is passed, then `enable` will additionally set the interrupt priority/subpriority of the interrupt before enabling it.

There's also `enable_atomic` with the same prototype, which is available when `has_atomic_enable` is `true`. `enable_atomic` enables the interrupt atomically, requires no additional locking, but will not set the interrupt priority before enabling.

### disable, disable_atomic
```cpp
template<typename Interrupt>
void disable() noexcept;
```
Disables the interrupt `Interrupt`. `Interrupt` can be either the device-specific interrupt structure, or the `mcutl::interrupts::interrupt` wrapper.

There's also `disable_atomic` with the same prototype, which is available when `has_atomic_disable` is `true`. `disable_atomic` disables the interrupt atomically, requires no additional locking.

### is_enabled
```cpp
template<typename Interrupt>
bool is_enabled() noexcept;
```
Returns `true` if the interrupt `Interrupt` is currently enabled. `Interrupt` can be either the device-specific interrupt structure, or the `mcutl::interrupts::interrupt` wrapper.

### set_priority, set_priority_atomic
```cpp
template<typename Interrupt, auto PriorityCount = maximum_priorities>
void set_priority() noexcept;
```

Sets the interrupt `Interrupt` priority and subpriority. `Interrupt` must be the `mcutl::interrupts::interrupt` wrapper type. You need to provide `PriorityCount` additionally if you explicitly changed this value when calling `initialize_controller`.

There's also `set_priority_atomic` with the same prototype, which is available when `has_atomic_set_priority` is `true`. `set_priority_atomic` sets the interrupt priority atomically and requires no additional locking.

`set_priority` and `set_priority_atomic` functions are available when `has_priorities` is `true`.

### get_priority, get_subpriority
```cpp
template<typename Interrupt, auto PriorityCount = maximum_priorities>
priority_t get_priority() noexcept;

template<typename Interrupt, auto PriorityCount = maximum_priorities>
priority_t get_subpriority() noexcept;
```
Return priority and subpriority of the interrupt `Interrupt`. You need to provide `PriorityCount` additionally if you explicitly changed this value when calling `initialize_controller`. `Interrupt` can be either the device-specific interrupt structure, or the `mcutl::interrupts::interrupt` wrapper.

These functions are available when `has_priorities` and `has_subpriorities` are `true`, respectively.

### clear_pending, clear_pending_atomic
```cpp
template<typename Interrupt>
void clear_pending() noexcept;
```
Clears pending interrupt `Interrupt`. `Interrupt` can be either the device-specific interrupt structure, or the `mcutl::interrupts::interrupt` wrapper.

There's also `clear_pending_atomic` with the same prototype, which is available when `has_atomic_clear_pending` is `true`. `clear_pending_atomic` clears pending interrupt atomically and requires no additional locking.

### is_pending
```cpp
template<typename Interrupt>
bool is_pending() noexcept;
```
Returns `true` if the interrupt `Interrupt` is currently pending. `Interrupt` can be either the device-specific interrupt structure, or the `mcutl::interrupts::interrupt` wrapper.

### is_interrupt_context
```cpp
template<typename T = void>
bool is_interrupt_context() noexcept;
```
Returns `true` if the code executing is currently in interrupt context. `T` parameter may be omitted, it's required only to make template-dependent calls to make the code compile in case the target MCU does not support `is_interrupt_context`.

This function is available when `has_interrupt_context` is `true`.

### enable_all, disable_all
```cpp
void enable_all() noexcept;
void disable_all() noexcept;
```
Enables or disables all interrupts without changing the individual interrupt settings.

### get_active
```cpp
template<typename T = void>
irqn_t get_active() noexcept;
```
Returns the interrupt request number (`IRQn`) of the currently executing interrupt. If there's no interrupt executing, returns the `unused_irqn` value. `T` parameter may be omitted, it's required only to make template-dependent calls to make the code compile in case the target MCU does not support `get_active`.

This function is available when `has_get_active` is `true`.

### disabled
```cpp
template<typename Interrupt>
struct disabled {};
```
This is a helper structure to disable peripheral-specific interrupts. Please refer to peripherals documentation, which sometimes make use of this structure and provide examples of how to use it.

## Processing interrupts
To process an interrupt, you still have to define an interrupt handler for the MCU of your choice yourself. The MCUTL library does not define any handlers and does not change handler addresses dynamically.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific interrupts
These interrupt definitions are defined in the `mcutl::interrupt::types` namespace. They can be used as is or with the `mcutl::interrupt::interrupt` wrapper type.

The following interrupts are available for all devices: `wwdg`, `pvd`, `tamper`, `rtc`, `rtc_alarm`, `flash`, `rcc`, `exti0`, `exti1`, `exti2`, `exti3`, `exti4`, `exti9_5`, `exti15_10`, `dma1_ch1`, `dma1_ch2`, `dma1_ch3`, `dma1_ch4`, `dma1_ch5`, `dma1_ch6`, `dma1_ch7`, `dma2_ch1`, `dma2_ch2`, `dma2_ch3`, `adc1_2`, `can1_rx1`, `can1_sce`, `tim1_cc`, `tim2`, `tim3`, `tim4`, `tim5`, `tim6`, `tim7`, `i2c1_ev`, `i2c1_er`, `i2c2_ev`, `i2c2_er`, `spi1`, `spi2`, `spi3`, `usart1`, `usart2`, `usart3`, `uart4`, `uart5`.

The following additional interrupts are available for connectivity line devices: `can1_tx`, `can1_rx0`, `dma2_ch5`, `eth`, `eth_wkup`, `can2_tx`, `can2_rx0`, `can2_rx1`, `can2_sce`, `otg_fs`, `tim1_brk`, `tim1_up`, `tim1_trg_com`, `otg_fs_wkup`, `dma2_ch4`.

The following additional interrupts are available for XL-density devices: `usb_hp_can1_tx`, `usb_lp_can1_rx0`, `tim1_brk_tim9`, `tim1_up_tim10`, `tim1_trg_com_tim11`, `usb_wakeup`, `tim8_brk_tim12`, `tim8_up_tim13`, `tim8_trg_com_tim14`, `tim8_cc`, `adc3`, `fsmc`, `sdio`, `dma2_ch4_5`.

The following additional interrupts are available for all other devices (non-XL-density and non-Connectivity): `usb_hp_can1_tx`, `usb_lp_can1_rx0`, `tim1_brk`, `tim1_up`, `tim1_trg_com`, `usb_wakeup`, `tim8_brk`, `tim8_up`, `tim8_trg_com`, `tim8_cc`, `adc3`, `fsmc`, `sdio`, `dma2_ch4_5`.

The following interrupts are available and currently support only setting priorities via the `mcutl::interrupt` functions: `svcall`, `pendsv`, `systick`.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific options for the interrupt controller
These MCUs do not provide any additional options for the `initialize_controller` function.
