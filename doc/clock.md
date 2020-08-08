# MCU clock configuration (mcutl/clock/clock.h)
This header provides facilities to configure and reconfigure the MCU clock. It automatically calculates the best suitable clock tree considering your requirements, and then generates the code to configure the clocks as required. All calculations are done during compilation, so the code generated performs only the configuration itself and nothing else. All of the definitions are in the `mcutl::clock` namespace, unless otherwise stated.

## Clock configuration
Clock configuration is represented as a C++ type, for example:
```cpp
using clock_config = mcutl::clock::config<
	mcutl::clock::external_high_speed_crystal<16_MHz>,
	mcutl::clock::core<mcutl::clock::required_frequency<72_MHz>>,
	mcutl::clock::spi1<mcutl::clock::min_frequency<100_KHz>, mcutl::clock::max_frequency<200_KHz>>,
	mcutl::clock::provide_usb_frequency,
	mcutl::clock::base_configuration_is_currently_present
>;
```
This configuration tells the MCUTL library that the external `16 MHz` crystal is used as a source. The MCU core is required to run at a `72 MHz` frequency, `spi1` is required to run at a frequency between `100 KHz` and `200 KHz` inclusive, a valid frequency is required for the USB peripheral. The configuration also tells, that base configuration is present (i.e. the original MCU clock configuration was not yet changed).

`_MHz` and `_KHz` literal operators are defined in the `mcutl::clock::literals` namespace.

If the configuration requested is not valid or impossible with the requirements specified, you will get a readable compile-time error. It's not possible to supply the invalid configuration and get it compiled for your MCU.

## Configuring MCU clocks
When you have the configuration, you need to call `configure_clocks` to apply it:
```cpp
mcutl::clock::configure_clocks<clock_config>();
```

## Reconfiguring MCU clocks
Suppose that you've configured the MCU clocks using `configure_clocks`. Now you want to reconfigure it to another configuration. As the first option, you can write another configuration and call `configure_clocks` again (you need to remove the `base_configuration_is_currently_present` option from the new configuration in this case). However, this may generate unnecessarily large code, as the `configure_clocks` call will have to take care about any possible changes in the configuration. It does not know, which was the previous configuration, so there is no way to optimize the reconfiguring process. Fortunately, there is a way to supply the previous configuration to the MCUTL library:
```cpp
mcutl::clock::reconfigure_clocks<current_clock_config, new_clock_config>();
```
This call will take care exactly of the changes required. If something in the new configuration remains the same, no code will be generated to check this at run time, as the check has been already made at compile time. So, `reconfigure_clocks` will in general produce smaller and faster code, as it knows, which was the previous configuration.

There is also a way to derive the new configuration from the previous one. Suppose you have the configuration `clock_config` defined above. If you want to change the `spi1` frequency only, you can skip copy-pasting the configuration and changing a few digits in a new one. Instead, you can do the following:
```cpp
using spi_config_changes = mcutl::clock::config<
	mcutl::clock::spi1<mcutl::clock::min_frequency<400_KHz>, mcutl::clock::max_frequency<800_KHz>>
>;
using new_clock_config = mcutl::clock::overridden_options_t<clock_config, spi_config_changes>;
mcutl::clock::reconfigure_clocks<clock_config, new_clock_config>();
```

You may also completely remove any requirement from the original configuration:
```cpp
//Remove SPI1 and Core requirements
using config_changes = mcutl::clock::config<
	mcutl::clock::remove_requirement<mcutl::clock::spi1<>>,
	mcutl::clock::remove_requirement<mcutl::clock::core<>>
>;
using new_clock_config = mcutl::clock::overridden_options_t<clock_config, config_changes>;
mcutl::clock::reconfigure_clocks<clock_config, new_clock_config>();
```

## Getting clock tree information at compile time
You may want to retrieve some properties of the clock tree which corresponds to any configuration. This is easily achievable at compile time:
```cpp
constexpr auto spi1_clock = mcutl::clock::get_clock_info<clock_config,
	mcutl::clock::clock_id::spi1>();
static_assert(spi1_clock.get_prescaler() == 256);
static_assert(spi1_clock.get_exact_frequency() == 140625u);
static_assert(spi1_clock.get_prescaler_divider() == 1);
static_assert(spi1_clock.get_unscaled_frequency() == 36_MHz);
```
This requests four properties for any of the clocks in the clock tree. For example, for the configuration `clock_config` defined above, we get `spi1` working at `140.625 KHz` frequency, the selected prescaler is `256`, the prescaler divider is `1`, and the unscaled frequency is `36 MHz`.
* `get_prescaler()` and `get_prescaler_divider()` - prescaler for the clock node. Most clock nodes can either divide or multiply the source frequency. To calculate the prescaler, divide `get_prescaler()` value by `get_prescaler_divider()` value. This is required to represent floating point prescalers (like `1.5`) as integers.
* `get_exact_frequency()` - the exact frequency of the clock node.
* `get_unscaled_frequency()` - the frequency before scaling, i.e. the frequency of the parent clock node output. For example, `spi1` parent may be `apb2` for STM32F103 MCUs, and thus the frequency of the `apb2` is `36 MHz`, which is then divided by 256 for `spi1`.

`get_clock_info` function will not compile, if the clock node is not used. For example, if no requirements were supplied for `spi1` in the `clock_config` configuration, you'd get a compile error.
`mcutl::clock::clock_id` enumeration is MCU-specific and depends on the clock tree of the MCU you are targeting.

---

It's also possible to get the entire clock tree for the configuration:
```cpp
constexpr auto clock_tree = mcutl::clock::get_best_clock_tree<clock_config>();
constexpr auto spi1_clock = clock_tree.get_config_by_id(mcutl::clock::clock_id::spi1);
static_assert(spi1_clock.is_used());
static_assert(spi1_clock.get_exact_frequency() == 140625u);
static_assert(spi1_clock.get_prescaler_value() == 256);
static_assert(spi1_clock.get_prescaler_divider() == 1);
static_assert(clock_tree.get_node_parent(mcutl::clock::clock_id::spi1) == mcutl::clock::clock_id::apb2);
```
* `get_config_by_id()` returns the clock node configuration by `clock_id`.
* `get_node_parent()` returns the parent node for the clock node with the `clock_id` id.
* `get_prescaler()` and `get_prescaler_divider()` - prescaler for the clock node. Most clock nodes can either divide or multiply the source frequency. To calculate the prescaler, divide `get_prescaler()` value by `get_prescaler_divider()` value. This is required to represent floating point prescalers (like `1.5`) as integers.
* `get_exact_frequency()` - the exact frequency of the clock node.
* `is_used()` - `true` if the clock node is used in the clock configuration. If this is `false`, other properties are not valid.

## Common clock configuration options
Here is the list of common clock configuration options, which you can pass to the `mcutl::clock::config` template.
* `set_highest_possible_frequencies`, `set_lowest_possible_frequencies`. Select all frequencies as high as possible or as low as possible (considering the requirements). By default, highest possible frequencies are selected.
* `base_configuration_is_currently_present`. Base MCU clock configuration is currently present. This means, `mcutl::clock::configure_clocks` can skip a lot of checks. Supply this option to `mcutl::clock::configure_clocks` if the existing clock configuration is default (for example, at startup).
* `core<frequency requirements>`. Set core frequency requirements. May be absent for some MCUs. `Frequency requirements` are described below.
* `internal_high_speed_crystal`, `external_high_speed_crystal<uint64_t Frequency>`, `external_high_speed_bypass<uint64_t Frequency>`. Set the clock source: internal high speed crystal, external high speed crystal or bypass at specified frequency. This option is required. You may supply several options from this group (e.g. both `internal_high_speed_crystal` and `external_high_speed_crystal`), in this case the best suitable one will be selected. You can check, which one was selected, using the `get_best_clock_tree` and `get_config_by_id` calls (see above).
* `force_use_pll`, `force_skip_pll`. Set the requirements for the PLL usage: force enable and use the PLL or force disable the PLL. This option may be present for MCUs with a single PLL.
* `provide_usb_frequency`, `disable_usb_frequency`. Request a valid USB frequency or disable the USB frequency source. This option may be present for MCUs with a single USB peripheral.

## Frequency requirements
Some clock configuration options may have frequency requirements (such as the `core` option). These are all possible options:
* `required_frequency<uint64_t Frequency>` - set the exact required frequency. Can not be combined with other frequency requirements.
* `min_frequency<uint64_t Frequency>` - set minimum acceptable frequency.
* `max_frequency<uint64_t Frequency>` - set maximum acceptable frequency.

Examples of using the requirements: `mcutl::clock::core<mcutl::clock::required_frequency<72_MHz>>` or `mcutl::clock::spi1<mcutl::clock::min_frequency<100_KHz>, mcutl::clock::max_frequency<200_KHz>>`.

## STM32F101, STM32F102, STM32F103 specific clock configuration options
These options can be passed to the `mcutl::clock::config` template.
* `adc<frequency requirements>` - set ADC frequency requirements.
* `apb1<frequency requirements>` - set APB1 frequency requirements.
* `apb2<frequency requirements>` - set APB2 frequency requirements.
* `ahb<frequency requirements>` - set AHB frequency requirements.
* `spi1<frequency requirements>` - set SPI1 frequency requirements.
* `spi2<frequency requirements>` - set SPI2 frequency requirements.
* `spi3<frequency requirements>` - set SPI3 frequency requirements.
* `timer2_3_4_5_6_7_12_13_14<frequency requirements>` - set frequency requirements for timers 2, 3, 4, 5, 6, 7, 12, 13, 14.
* `timer1_8_9_10_11<frequency requirements>` - set frequency requirements for timers 1, 8, 9, 10, 11.
* `disable_flash_programming_interface`, `enable_flash_programming_interface` - disable or enable flash programming interface configuration (FLITF). This interface is enabled by default.

## STM32F101, STM32F102, STM32F103 specific clock node identifiers (clock_id)
* `hse` - high speed external oscillator or bypass.
* `hsi` - high speed external oscillator.
* `hse_prediv` - optional divider by `2` for the high speed external source (`PLLXTPRE`).
* `hsi_prediv` - optional divider by `2` for the high speed internal crystal (always used with PLL).
* `pll` - PLL.
* `usb` - USB peripheral.
* `sys` - core.
* `ahb` - AHB bridge.
* `apb1` - APB1 bridge.
* `apb2` - APB2 bridge.
* `adc` - ADC.
* `spi1` - SPI1.
* `spi2` - SPI2.
* `spi3` - SPI3.
* `timer2_3_4_5_6_7_12_13_14` - Timers 2, 3, 4, 5, 6, 7, 12, 13, 14.
* `timer1_8_9_10_11` - Timers 1, 8, 9, 10, 11.
* `timer2_3_4_5_6_7_12_13_14_multiplier` - Timers 2, 3, 4, 5, 6, 7, 12, 13, 14 optional `x2` multiplier, which is used when the `APB1` prescaler is greater than `1`.
* `timer1_8_9_10_11_multiplier` - Timers 1, 8, 9, 10, 11 optional `x2` multiplier, which is used when the `APB2` prescaler is greater than `1`.
