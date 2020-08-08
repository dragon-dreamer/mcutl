#pragma once

#include "mcutl/device/backup/device_backup.h"
#include "mcutl/utils/class_limitations.h"
#include "mcutl/utils/definitions.h"

namespace mcutl::backup
{

constexpr auto min_register_index = device::backup::min_index;
constexpr auto max_register_index = device::backup::max_index;
using backup_index_type = decltype(min_register_index);
using backup_register_type = device::backup::backup_register_type;
using peripheral_type = device::backup::peripheral_type;

inline bool enable_backup_writes() MCUTL_NOEXCEPT
{
	return device::backup::enable_backup_writes();
}

inline bool disable_backup_writes() MCUTL_NOEXCEPT
{
	return device::backup::disable_backup_writes();
}

enum class write_disable_policy
{
	disable_always,
	disable_only_if_enabled
};

namespace detail
{

template<backup_index_type Index>
constexpr void check_backup() noexcept
{
	static_assert(Index <= max_register_index, "Invalid backup register index");
	static_assert(Index >= min_register_index, "Invalid backup register index");
}

template<write_disable_policy DisablePolicy>
struct backup_write_enabler_base : types::noncopymovable
{
	inline backup_write_enabler_base() MCUTL_NOEXCEPT
	{
		enable_backup_writes();
	}
	
	inline ~backup_write_enabler_base() MCUTL_NOEXCEPT
	{
		disable_backup_writes();
	}
};

template<>
struct backup_write_enabler_base<write_disable_policy::disable_only_if_enabled>
	: types::noncopymovable
{
public:
	inline backup_write_enabler_base() MCUTL_NOEXCEPT
		: was_enabled_(enable_backup_writes())
	{
	}
	
	inline ~backup_write_enabler_base() MCUTL_NOEXCEPT
	{
		if (was_enabled_)
			disable_backup_writes();
	}
	
private:
	bool was_enabled_ = false;
};

} //namespace detail

template<backup_index_type Index>
[[nodiscard]] inline backup_register_type read_backup() MCUTL_NOEXCEPT
{
	detail::check_backup<Index>();
	return device::backup::read_backup<Index>();
}

template<backup_index_type Index, bool EnableWrites = true,
	write_disable_policy DisablePolicy = write_disable_policy::disable_only_if_enabled>
inline void write_backup(backup_register_type value) MCUTL_NOEXCEPT
{
	detail::check_backup<Index>();
	
	if constexpr (EnableWrites)
	{
		[[maybe_unused]] bool enabled = enable_backup_writes();
		device::backup::write_backup<Index>(value);
		
		if constexpr (DisablePolicy == write_disable_policy::disable_always)
			disable_backup_writes();
		else if (enabled)
			disable_backup_writes();
	}
	else
	{
		device::backup::write_backup<Index>(value);
	}
}

template<write_disable_policy DisablePolicy = write_disable_policy::disable_only_if_enabled>
struct backup_write_enabler : detail::backup_write_enabler_base<DisablePolicy>
{
public:
	template<backup_index_type Index>
	inline backup_register_type read() const MCUTL_NOEXCEPT
	{
		return read_backup<Index>();
	}
	
	template<backup_index_type Index>
	inline void write(backup_register_type value) const MCUTL_NOEXCEPT
	{
		write_backup<Index, false>(value);
	}
};

} //namespace mcutl::backup
