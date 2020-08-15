# MCU direct memory access (DMA) support layer (mcutl/dma/dma.h)
This header provides facilities to configure and use MCU DMA facilities (if supported by the MCU). The DMA facilities greatly vary from one MCU to another, and this layer is currently based on STM32F10x facilities (thus currently lacking DMA linked list blocks support). Keep in mind, that some DMA channels may be connected to the MCU peripherals and receive signals from them, and this may alter your choice of the DMA channel for a particular peripheral data transfer. All of the definitions are in the `mcutl::dma` namespace, unless otherwise stated.

## Public definitions
```cpp
namespace mode
{
struct circular {};
struct normal {};
} //namespace mode
```
These structs set the transfer mode:
* `normal` is a usual transfer from the source to the destination. This is the default mode.
* `circular` requires the DMA channel to continuously serve the DMA requests coming from the peripheral connected to the channel. When the DMA channel receives the request, it will do the same transfer every time.

Some MCUs may support other modes. Some MCUs may not support the `circular` mode. There are several traits (described below) to determine the MCU capabilities.

```cpp
namespace data_size
{
struct byte {};
struct halfword {};
struct word {};
} //namespace data_size
```
These structs set the data size to transfer. It can be set separately for the source and for the destination. `byte` is the hardware byte, `word` is the hadrware word, `halfword` is the hardware word half. Some MCUs may support other sizes. Some MCUs may not support some of these sizes. There are several traits (described below) to determine the MCU capabilities.

```cpp
namespace pointer_increment
{
struct disabled {};
struct enabled {};
} //namespace pointer_increment
```
These struct enable or disable source or destination address pointer increment. When each `byte`, `word`, or `halfword` is transferred, the source and/or the destination pointer can be advanced by the size of the data block (`byte`, `word` or `halfword`, respectively) before transferring the next data block.

```cpp
namespace address
{
struct memory {};
struct peripheral {};
} //namespace address
```
Sets the source and the destination address type, which may be `memory` (RAM) or `peripheral`. Some MCUs may support other address types. Some MCUs may not support some transfer types (for example, memory to memory). There are several traits (described below) to determine the MCU capabilities.

```cpp
namespace priority
{
struct low {};
struct medium {};
struct high {};
struct very_high {};
} //namespace priority
```
These structs indicate the transfer priority, which has meaning when several memory transfers access the same memory bus. These are several relative values which may be mapped differently for the MCU of your choice. Priorities may not be supported by the MCU (or hardwired), and there are several traits (described below) to determine the MCU capabilities.

```cpp
namespace interrupt
{
struct transfer_complete {};
struct half_transfer {};
struct transfer_error {};

struct global {};

struct enable_controller_interrupts {};
struct disable_controller_interrupts {};
} //namespace interrupt
```
These structs are used to configure DMA interrupts.
* `transfer_complete` - transfer complete interrupt.
* `half_transfer` - half-transfer interrupt.
* `transfer_error` - transfer error interrupt.
* `global` - used to clear all DMA interrupt flags (see `clear_pending_flags` below).
* `enable_controller_interrupts` - this options indicates, that the related [interrupt controller](interrupt.md) interrupts must be enabled.
* `disable_controller_interrupts` - this options indicates, that the related [interrupt controller](interrupt.md) interrupts must be disabled.

To enable an interrupt, specify the related structure in the DMA configuration (see below for examples). To enable an interrupt and set its priority/subpriority, use `mcutl::interrupt::interrupt` [wrapper](interrupt.md). To disable an interrupt, wrap the related interrupt structure into the `mcutl::interrupt::disabled` wrapper structure. To additionally enable or disable the related interrupt controllers interrupts, use the `enable_controller_interrupts` or `disable_controller_interrupts` structs. Use interrupt structures (or `global`) with the `clear_pending_flags` function to clear the related pending DMA flags (see below for more information).

Add the `mcutl::interrupt::priority_count` option to explicitly set the interrupt priority count. Use this option if you explicitly changed the interrupt priority count when calling `mcutl::interrupt::initialize_controller`.

Some MCUs may support a different set of interrupts. There are several traits (described below) to determine the MCU capabilities.

```cpp
template<typename DataSize, typename AddressType,
	typename PointerIncrementMode = pointer_increment::enabled>
struct source;
template<typename DataSize, typename AddressType,
	typename PointerIncrementMode = pointer_increment::disabled>
struct destination;
```
These structures describe the source and the destination DMA transfer addresses. `DataSize` is the transfer data size, `AddressType` is the address type, `PointerIncrementMode` indicates if pointer increment must be enabled.

```cpp
template<uint32_t DmaIndex, uint32_t ChannelNumber>
struct channel;
```
This structure is the base structure for any DMA channel. MCUs provide device-specific definitions (such as `dma1`) for DMA peripherals.

```cpp
template<typename Interrupt, typename Channel>
using interrupt_type = ...;
```
This typedef indicates a corresponding [interrupt controller](interrupt.md) interrupt type for the specified DMA interrupt and channel.

```cpp
template<typename Channel>
using peripheral_type = ...;
```
This typedef indicates a corresponding [peripheral](peripheral.md) type (or `mcutl::periph::no_periph`) for the specified DMA channel. This peripheral must be enabled before accessing a channel.

```cpp
using size_type = ...;
```
This typedef indicates the transfer size type which the DMA controller(s) can handle. This can be, for example, `uint8_t` or `uint16_t`. This indicates how much data can be transfered by a single DMA transfer.

## Traits
There are several traits available to determine the target MCU capabilities:
```cpp
//True if the target MCU supports priority levels
[[maybe_unused]] constexpr bool supports_priority_levels = ...;
//True if the target MCU supports byte-sized transfers
[[maybe_unused]] constexpr bool supports_byte_transfer = ...;
//True if the target MCU supports halfword-sized transfers
[[maybe_unused]] constexpr bool supports_halfword_transfer = ...;
//True if the target MCU supports word-sized transfers
[[maybe_unused]] constexpr bool supports_word_transfer = ...;
//True if the target MCU supports circular transfer mode
[[maybe_unused]] constexpr bool supports_circular_mode = ...;
//True if the target MCU supports memory-to-memory transfers
[[maybe_unused]] constexpr bool supports_memory_to_memory_transfer = ...;
//True if the target MCU supports memory-to-peripheral transfers
[[maybe_unused]] constexpr bool supports_memory_to_periph_transfer = ...;
//True if the target MCU supports peripheral-to-memory transfers
[[maybe_unused]] constexpr bool supports_periph_to_memory_transfer = ...;
//True if the target MCU supports peripheral-to-peripheral transfers
[[maybe_unused]] constexpr bool supports_periph_to_periph_transfer = ...;
//True if the target MCU supports transfer complete interrupt
[[maybe_unused]] constexpr bool supports_transfer_complete_interrupt = ...;
//True if the target MCU supports half transfer interrupt
[[maybe_unused]] constexpr bool supports_half_transfer_interrupt = ...;
//True if the target MCU supports transfer error interrupt
[[maybe_unused]] constexpr bool supports_transfer_error_interrupt = ...;
//True if the target MCU supports clear_pending_flags_atomic function
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags = ...;
```

## configure_channel
```cpp
template<typename Channel, typename... Options>
void configure_channel() noexcept;
```
This function is used to configure a DMA `Channel`. `Options` is a set of options describing the configuration. Here are several examples:

```cpp
mcutl::dma::configure_channel<mcutl::dma::dma1<2>,
	mcutl::dma::priority::high,
	mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_complete, 3>,
	mcutl::dma::interrupt::enable_controller_interrupts,
	mcutl::dma::source<mcutl::dma::data_size::byte,
		mcutl::dma::address::memory, mcutl::dma::pointer_increment::enabled>,
	mcutl::dma::destination<mcutl::dma::data_size::byte,
		mcutl::dma::address::peripheral, mcutl::dma::pointer_increment::disabled>
>();
```
This call configures the `dma1<2>` channel. If the transfer is currently in progress, it will be halted. It sets the transfer priority to `high`, enables the `transfer_complete` interrupt and sets its priority to `3`, enables the related interrupt controller interrupts. It declares the transfer source as `memory` with the `byte` transfer size, pointer increment enabled. It declares the transfer destination as `peripheral` with the `byte` transfer size, pointer increment disabled.

```cpp
mcutl::dma::configure_channel<mcutl::dma::dma1<1>,
	mcutl::dma::interrupt::disable_controller_interrupts,
	mcutl::dma::source<mcutl::dma::data_size::byte, mcutl::dma::address::memory,
		mcutl::dma::pointer_increment::disabled>,
	mcutl::dma::destination<mcutl::dma::data_size::halfword, mcutl::dma::address::memory,
		mcutl::dma::pointer_increment::enabled>
>();
```
This call configures the `dma1<1>` channel. It disables all the related interrupt controller interrupts. It declares the transfer source as `memory` with the `byte` transfer size, pointer increment disabled. It declares the transfer destination as `memory` with the `halfword` transfer size, pointer increment enabled. Note that the rules of extending or shrinking data blocka are device-specific. They apply when you specify different transfer sizes for the source and the destination.

```cpp
mcutl::dma::configure_channel<mcutl::dma::dma2<3>,
	mcutl::dma::mode::circular,
	mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_complete, 5>,
	mcutl::interrupt::interrupt<mcutl::dma::interrupt::transfer_error, 5>,
	mcutl::dma::interrupt::enable_controller_interrupts,
	mcutl::dma::interrupt::disabled<mcutl::dma::interrupt::half_transfer>,
	mcutl::dma::interrupt::disable_controller_interrupts,
	mcutl::dma::source<mcutl::dma::data_size::word, mcutl::dma::address::peripheral,
		mcutl::dma::pointer_increment::disabled>,
	mcutl::dma::destination<mcutl::dma::data_size::word, mcutl::dma::address::memory,
		mcutl::dma::pointer_increment::enabled>
>();
```
This call configures the `dma2<3>` channel. It enables the `circular` transfer mode. It also enables the `transfer_complete` and the `transfer_error` interrupts (and sets their priorities to `5`) and disables the `half_transfer` interrupt. It declares the transfer source as `peripheral` with the `word` transfer size, pointer increment disabled. It declares the transfer destination as `memory` with the `word` transfer size, pointer increment enabled.

Note that some DMA interrupts may map to the same interrupt controller interrupt (for example, a single interrupt for a DMA channel). In this case, when enabling interrupts and setting their priorities, all the priorities of these interrupts must match. This also means, that if you enable and disable the related interrupt controller interrupt in the same `configure_channel` call, the interrupt will be enabled, but not disabled.

The only required options for the `configure_channel` function are `source` and `destination`.

## reconfigure_channel
```cpp
template<typename Channel, typename... Options>
void reconfigure_channel() noexcept;
```
This function reconfigures the already configured DMA `Channel`. If the transfer is currently in progress, it will be halted. You can pass the same options to this function, as to the `configure_channel` function. You may skip `source` and `destination` options, and in that case the source and the destination configurations will remain the same. If you pass the `enable_controller_interrupts` to this function, it will enable the related interrupt controller interrupts for all the DMA channel interrupts already enabled and being enabled by this call. If you pass the `disable_controller_interrupts` to this function, it will disable the related interrupt controller interrupts for all the DMA channel interrupts already disabled and being disabled by this call.

You may disable the ongoing transfer using the following call:
```cpp
//Disables ongoing transfer for the dma2<5> channel
mcutl::dma::reconfigure_channel<mcutl::dma::dma2<5>>();
```

## start_transfer
```cpp
template<typename Channel>
void start_transfer(const volatile void* from,
	volatile void* to, size_type size) noexcept;
```
Starts the DMA transfer for the previously configured `Channel` from the `from` address to the `to` address of the size `size`.

## wait_transfer
```cpp
template<typename Channel>
void wait_transfer() noexcept;
```
Synchronuously waits for the `Channel` transfer to complete. Returns immediately if there is no ongoing transfer.

## clear_pending_flags, clear_pending_flags_atomic
```cpp
template<typename Channel, typename... Interrupts>
void clear_pending_flags() noexcept;
template<typename Channel, typename... Interrupts>
void clear_pending_flags_atomic() noexcept;
```
These functions clear the pending DMA channel interrupt flags for `Interrupts`. You can pass either interrupt types or `mcutl::dma::interrupt::global` to clear all flags at once. You may need to clear these flags each time an interrupt is handled to prevent an interrupt from being raised again in an infinite loop. For some MCUs, these functions may be a no-op. The `clear_pending_flags_atomic` function clears the flags atomically, thus requiring no locking. It is available when the `supports_atomic_clear_pending_flags` is `true`.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific options and defines
These MCUs declare no device-specific DMA channel options. They provide these device-specific channel aliases:
```cpp
template<uint32_t ChannelNumber>
using dma1 = channel<1u, ChannelNumber>;

//Only when DMA2 controller exists on the device
template<uint32_t ChannelNumber>
using dma2 = channel<2u, ChannelNumber>;
```
