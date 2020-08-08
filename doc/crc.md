# MCU hardware CRC configuration (mcutl/crc/crc.h)
This header provides facilities to configure and use MCU hardware CRC block (if supported by the MCU). The MCU can support any CRC algorithm, and this header aims to provide a uniform interface to access the CRC facilities of any MCU. There are some definitions to determine the CRC size and polynomial. All of the definitions are in the `mcutl::crc` namespace, unless otherwise stated.

## Public definitions
```cpp
//CRC input and result type
using crc_input_type = ...;
//Initial CRC value
constexpr crc_input_type crc_initial_value = ...;
//CRC polynomial value
constexpr crc_input_type crc_polynomial = ...;
//CRC peripheral (or mcutl::periph::no_periph, if there's no separate peripheral)
using peripheral_type = ...;
```

### reset
```cpp
void reset() noexcept;
```
Resets the MCU CRC module to its default state.

### add_data
```cpp
void add_data(crc_input_type data) noexcept;
```
Adds the next CRC word to the calculation.

### get_crc
```cpp
crc_input_type get_crc() noexcept;
```
Returns the resulting CRC value.

### calculate_crc
```cpp
crc_input_type calculate_crc(const crc_input_type* data_pointer,
	size_t data_size) noexcept;
crc_input_type calculate_crc(const uint8_t* data_pointer,
	size_t data_size_in_bytes) noexcept;
template<typename Data>
crc_input_type calculate_crc(const Data& data) noexcept;
template<typename T, size_t N>
crc_input_type calculate_crc(T (&arr)[N]) noexcept;
```
These functions are helpers to calculate the CRC checksum of a block of data. Basically, all of them call `reset`, then `add_data` one or more times, and then `get_crc`.
* The first one calculates the CRC of the `crc_input_type` values array `data_pointer` with the `data_size` size (where `data_size` is array element count).
* The second one calculates the CRC of the byte array `data_pointer` with the `data_size_in_bytes` size.
* The third one calculates the CRC of the `data` structure or class. `Data` must be trivially copyable.
* The last one calculates the CRC of an `arr` array of type `T`. `T` must be trivially copyable.
