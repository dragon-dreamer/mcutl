# MCUTL MCU-specific instructions access layer (mcutl/instruction)
MCUTL routes all MCU-specific instruction calls to a separate instruction layer. This is a zero-cost abstraction, which allows to unit-test your firmware on a host PC. If you plan to unit-test your firmware on host PC, you need to use the functions described below when executing the MCU-specific instructions. The MCUTL library uses these functions and never executes MCU-specific instructions directly, which makes it testable. All of the definitions are in the `mcutl::instruction` namespace, unless otherwise stated.

## instruction.h header
Contains functions to execute MCU-specific instructions.

#### execute
```cpp
template<typename Instruction, typename... Args>
MCUTL_STATIC_FORCEINLINE auto execute(Args... args) noexcept;
```

Executes MCU-specific `Instruction` with arguments `Args` and returns optional execution result. All MCU-specific instructions are located in `mcutl::device::instruction::type` namespace.

## STM32F101, STM32F102, STM32F103, STM32F105, STM32F107 specific instructions.
The following MCU-specific instructions are available for these MCUs: `dmb`, `dsb`, `isb`, `cpsid_i`, `cpsie_i`, `wfi`, `wfe`. All of these instructions do not have any parameters and do not return value. `wfi` and `wfe` instructions are mapped to a special function calls to mitigate the STM32F1 hardware low-power mode bug.
