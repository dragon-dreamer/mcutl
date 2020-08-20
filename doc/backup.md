# MCU backup domain configuration and access (mcutl/backup/backup.h)
This header provides facilities to configure and access MCU backup domain (if supported by the MCU). The backup domain stores data in the special RAM area which is kept intact when the MCU restarts. The data is erased only if the MCU is completely powered off. All of the definitions are in the `mcutl::backup` namespace, unless otherwise stated.

## Public definitions
```cpp
//Minimum available backup register index
constexpr auto min_register_index = ...;
//Maximum available backup register index
constexpr auto max_register_index = ...;
//Backup register index type
using backup_index_type = ...;
//Backup register type
using backup_register_type = ...;
//Backup peripheral type, type list, or mcutl::periph::no_periph
using peripheral_type = ...;
```
To access the backup domain, you need to enable its [peripheral](periph.md) (`peripheral_type`). `min_register_index` and `max_register_index` indicate which backup register indices are accessible (inclusive).

### enable_backup_writes, disable_backup_writes
```cpp
bool enable_backup_writes() noexcept;
bool disable_backup_writes() noexcept;
```
Enables of disables backup domain writes, respectively. If you want to change the contents of the backup register, you need to enable writes to this domain first. This operation may be a no-op for some MCUs. The `enable_backup_writes` function returns `true` if writes were enabled by the call, and `false` if writes had been already enabled when the function was called. The `disable_backup_writes` function returns `true` if writes were disabled. In some cases, write access can not be disabled at all (for example, for STM23F1 MCUs backup domain write access can not be disabled when the RTC uses the HSE clock).

### read_backup
```cpp
template<backup_index_type Index>
backup_register_type read_backup() noexcept;
```
Returns contents of the backup register with the index `Index`. Write access is not required when calling this function.

### write_backup, write_disable_policy
```cpp
enum class write_disable_policy
{
	disable_always,
	disable_only_if_enabled
};

template<backup_index_type Index, bool EnableWrites = true,
	write_disable_policy DisablePolicy = write_disable_policy::disable_only_if_enabled>
void write_backup(backup_register_type value) noexcept;
```
The `write_backup` function writes `value` to the backup register with the index `Index`.
* `EnableWrites` indicates if write access to the backup domain must be enabled before writing.
* `DisablePolicy` indicates how write access to the backup domain should be disabled after writing: `disable_always` indicates write accesses must be always disabled after writing, `disable_only_if_enabled` indicates write accesses must be disabled **only** if they have been enabled **by this** `write_backup` call. The `DisablePolicy` parameter is meaningful only when `EnableWrites` is true.

### backup_write_enabler
```cpp
template<write_disable_policy DisablePolicy = write_disable_policy::disable_only_if_enabled>
struct backup_write_enabler
{
public:
	template<backup_index_type Index>
	backup_register_type read() const noexcept;
	
	template<backup_index_type Index>
	void write(backup_register_type value) const noexcept;
};
```
This is a helper RAII class to enable write access to the backup domain in the constructor, write some values (or read them) using `write` and `read`, and then automatically disable write access in the destructor. Please note you don't need write access if you want to only read backup registers. You can use `mcutl::backup::read_backup` directly. `DisablePolicy` indicates how write access to the backup domain should be disabled when destroying the `backup_write_enabler` object: `disable_always` indicates write accesses must be always disabled, `disable_only_if_enabled` indicates write accesses must be disabled **only** if they have been enabled by the `backup_write_enabler` constructor. The `backup_write_enabler` can not be copied or moved.
