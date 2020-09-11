#pragma once

#include <cassert>
#include <limits>
#include <stdint.h>
#include <type_traits>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mcutl/device/memory/memory.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::tests::memory
{

using memory_address_t = uintptr_t;

class memory_interface
{
public:
	virtual ~memory_interface() {}

public:
	virtual void write(memory_address_t address, uint64_t value) = 0;
	virtual uint64_t read(memory_address_t address) = 0;
};

class memory_interface_mock;
class memory_interface_not_mocked : public memory_interface
{
public:
	memory_interface_not_mocked(const memory_interface_not_mocked&) = delete;
	memory_interface_not_mocked& operator=(const memory_interface_not_mocked&) = delete;
	
	explicit memory_interface_not_mocked(memory_interface_mock& mock) noexcept;
	virtual ~memory_interface_not_mocked() override;
	
	virtual void write(memory_address_t address, uint64_t value) override;
	virtual uint64_t read(memory_address_t address) override;
	
private:
	memory_interface_mock& mock_;
};

class memory_interface_mock : public memory_interface
{
public:
	void initialize_default_behavior(bool allow_all_reads = true)
	{
		using namespace ::testing;
		ON_CALL(*this, write).WillByDefault(
			[this] (memory_address_t address, uint64_t value) {
				set(address, value);
			});
		ON_CALL(*this, read).WillByDefault(
			[this] (memory_address_t address) {
				return get(address);
			});

		if (allow_all_reads)
		{
			EXPECT_CALL(*this, read).Times(AtLeast(0));
		}
		
		set_same_address_access_limit(50);
	}
	
	[[nodiscard]] memory_interface_not_mocked with_unmocked_memory() noexcept
	{
		return memory_interface_not_mocked(*this);
	}
	
	void allow_reads(memory_address_t address)
	{
		EXPECT_CALL(*this, read(address)).Times(::testing::AtLeast(0));
	}
	
	[[nodiscard]] uint64_t get(memory_address_t address) const
	{
		check_same_address_accesses(address);
		auto it = memory_.find(address);
		return it == cend(memory_) ? 0 : it->second;
	}
	
	void set(memory_address_t address, uint64_t value)
	{
		check_same_address_accesses(address);
		memory_[address] = value;
	}
	
	[[nodiscard]] uint64_t& get(memory_address_t address)
	{
		check_same_address_accesses(address);
		return memory_[address];
	}
	
	void set_same_address_access_limit(uint32_t max_same_address_accesses)
	{
		max_same_address_accesses_ = max_same_address_accesses;
	}
	
	MOCK_METHOD(void, write, (memory_address_t address, uint64_t value), (override));
	MOCK_METHOD(uint64_t, read, (memory_address_t address), (override));
	
private:
	void check_same_address_accesses(memory_address_t address) const
	{
		if (address != last_access_address_)
		{
			last_access_address_ = address;
			same_address_accesses_ = 0;
			return;
		}
		
		++same_address_accesses_;
		if (max_same_address_accesses_ && same_address_accesses_ > max_same_address_accesses_)
		{
			GTEST_FAIL_AT(__FILE__, __LINE__)
				<< "Possible infinite loop accessing address " << address;
			throw std::runtime_error("Possible infinite loop accessing address "
				+ std::to_string(address));
		}
	}
	
private:
	std::unordered_map<memory_address_t, uint64_t> memory_;
	mutable memory_address_t last_access_address_ = 0;
	uint32_t max_same_address_accesses_ = 0;
	mutable uint32_t same_address_accesses_ = 0;
};

class memory_controller
{
public:
	memory_controller() = delete;
	static void set_interface(memory_interface* instance) noexcept
	{
		interface_ = instance;
	}
	
public:
	template<typename Ptr, typename Value>
	static void write(Ptr* pointer, Value value)
	{
		assert(interface_);
		auto address = reinterpret_cast<memory_address_t>(pointer);
		interface_->write(address, static_cast<uint64_t>(value));
	}
	
	template<typename Value, typename Ptr>
	static Value read(const Ptr* pointer)
	{
		assert(interface_);
		auto address = reinterpret_cast<memory_address_t>(pointer);
		return static_cast<Value>(interface_->read(address));
	}
	
private:
	static inline memory_interface* interface_;
};

inline uint64_t memory_interface_not_mocked::read(memory_address_t address)
{
	return mock_.get(address);
}

inline void memory_interface_not_mocked::write(memory_address_t address, uint64_t value)
{
	return mock_.set(address, value);
}

inline memory_interface_not_mocked::memory_interface_not_mocked(memory_interface_mock& mock) noexcept
	: mock_(mock)
{
	memory_controller::set_interface(this);
}
	
inline memory_interface_not_mocked::~memory_interface_not_mocked()
{
	memory_controller::set_interface(&mock_);
}

template<typename Pointer>
static memory_address_t addr(const Pointer* pointer) noexcept
{
	return reinterpret_cast<memory_address_t>(pointer);
}

class test_fixture_base : virtual public::testing::Test
{
public:
	virtual void SetUp() override
	{
		memory_controller::set_interface(&memory_mock_);
		memory_mock_.initialize_default_behavior();
	}

	virtual void TearDown() override
	{
		memory_controller::set_interface(nullptr);
	}

	[[nodiscard]] memory_interface_mock& memory() noexcept
	{
		return memory_mock_;
	}

	template<typename Pointer>
	[[nodiscard]] static memory_address_t addr(const Pointer* pointer) noexcept
	{
		return mcutl::tests::memory::addr(pointer);
	}

private:
	::testing::StrictMock<memory_interface_mock> memory_mock_;
};

class strict_test_fixture_base : public test_fixture_base
{
public:
	virtual void SetUp() override
	{
		memory_controller::set_interface(&memory());
		memory().initialize_default_behavior(false);
	}
};

} //namespace mcutl::tests::memory

namespace mcutl::device::memory
{

using mcutl::device::memory::common::to_bytes;

template<typename Type, auto Address>
[[nodiscard]] inline volatile Type* volatile_memory() noexcept
{
	return reinterpret_cast<volatile Type*>(Address);
}

template<typename Type, typename Address>
[[nodiscard]] inline volatile Type* volatile_memory(Address address) noexcept
{
	return reinterpret_cast<volatile Type*>(address);
}

template<typename Pointer>
[[nodiscard]] inline uintptr_t to_address(volatile Pointer* pointer) noexcept
{
	return reinterpret_cast<uintptr_t>(pointer);
}

template<typename ValueType>
[[maybe_unused]] constexpr std::enable_if_t<std::is_unsigned_v<ValueType>, ValueType>
	max_bitmask = (std::numeric_limits<ValueType>::max)();

template<auto BitMask, auto BitValues, auto Reg, typename RegStruct>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	[[maybe_unused]] volatile RegStruct* ptr)
{
	using reg_type = std::remove_cv_t<types::type_of_member_pointer_t<Reg>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&(ptr->*Reg), static_cast<reg_type>(BitValues));
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&(ptr->*Reg));
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		tests::memory::memory_controller::write(&(ptr->*Reg), value);
	}
}

template<auto BitMask, auto Reg, typename RegStruct>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<types::type_of_member_pointer_t<Reg>> values)
{
	using reg_type = decltype(values);
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&(ptr->*Reg), values);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&(ptr->*Reg));
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		tests::memory::memory_controller::write(&(ptr->*Reg), value);
	}
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits(
	std::remove_cv_t<types::type_of_member_pointer_t<Reg>> values)
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, Reg>(volatile_memory<class_t, RegStructBase>(), values);
}

template<auto BitMask, auto BitValues, auto Reg, uintptr_t RegStructBase>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_bits()
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, BitValues, Reg>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, typename RegStruct>
[[nodiscard]] auto get_register_bits(const volatile RegStruct* ptr)
{
	using value_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	return tests::memory::memory_controller::read<value_t>(&(ptr->*Reg));
}

template<auto Reg, uintptr_t RegStructBase>
[[nodiscard]] auto get_register_bits()
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	return get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>());
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
[[nodiscard]] auto get_register_bits()
{
	using reg_type = std::remove_cv_t<types::type_of_member_pointer_t<Reg>>;
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
	{
		return static_cast<reg_type>(
			get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>()) & unsigned_bitmask);
	}
	else
	{
		return reg_type {};
	}
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] auto get_register_bits(const volatile RegStruct* ptr)
{
	using reg_type = std::remove_cv_t<types::type_of_member_pointer_t<Reg>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
		return static_cast<reg_type>(get_register_bits<Reg>(ptr) & unsigned_bitmask);
	else
		return reg_type{};
}

template<auto BitMask, auto BitValues, typename RegType, typename RegStruct>
void set_register_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr)
{
	using reg_type = std::remove_cv_t<RegType>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&(ptr->*reg_ptr), BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&(ptr->*reg_ptr));
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		tests::memory::memory_controller::write(&(ptr->*reg_ptr), value);
	}
}

template<auto BitMask, typename RegType, typename RegStruct>
void set_register_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<RegType> values)
{
	using reg_type = decltype(values);
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&(ptr->*reg_ptr), values);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&(ptr->*reg_ptr));
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		tests::memory::memory_controller::write(&(ptr->*reg_ptr), value);
	}
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
void set_register_bits(RegType RegStruct::*reg_ptr, std::remove_cv_t<RegType> values)
{
	set_register_bits<BitMask>(reg_ptr, volatile_memory<RegStruct, RegStructBase>(), values);
}

template<auto BitMask, auto BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
void set_register_bits(RegType RegStruct::*reg_ptr)
{
	set_register_bits<BitMask, BitValues>(reg_ptr, volatile_memory<RegStruct, RegStructBase>());
}

template<typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	return tests::memory::memory_controller::read<std::remove_cv_t<RegType>>(&(ptr->*reg_ptr));
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	return get_register_bits(reg_ptr, volatile_memory<RegStruct, RegStructBase>());
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	using reg_type = std::remove_cv_t<RegType>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
	{
		return static_cast<reg_type>(
			get_register_bits(reg_ptr, volatile_memory<RegStruct, RegStructBase>()) & unsigned_bitmask);
	}
	else
	{
		return reg_type{};
	}
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<RegType>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
		return static_cast<reg_type>(get_register_bits(reg_ptr, ptr) & unsigned_bitmask);
	else
		return reg_type{};
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, typename RegStruct>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	[[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<mcutl::types::type_of_member_pointer_t<Reg>>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&((ptr->*Reg)[RegArrIndex]), BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&((ptr->*Reg)[RegArrIndex]));
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		tests::memory::memory_controller::write(&((ptr->*Reg)[RegArrIndex]), value);
	}
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<std::remove_all_extents_t<
		mcutl::types::type_of_member_pointer_t<Reg>>> values) noexcept
{
	using reg_type = decltype(values);
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&((ptr->*Reg)[RegArrIndex]), values);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&((ptr->*Reg)[RegArrIndex]));
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		tests::memory::memory_controller::write(&((ptr->*Reg)[RegArrIndex]), value);
	}
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits(
	std::remove_cv_t<std::remove_all_extents_t<mcutl::types::type_of_member_pointer_t<Reg>>> value) noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_array_bits<BitMask, Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>(), value);
}

template<auto BitMask, auto BitValues, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
std::enable_if_t<std::is_member_object_pointer_v<decltype(Reg)>> set_register_array_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_array_bits<BitMask, BitValues, Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] auto get_register_array_bits(const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<
		mcutl::types::type_of_member_pointer_t<Reg>>>;
	return tests::memory::memory_controller::read<reg_type>(&((ptr->*Reg)[RegArrIndex]));
}

template<auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] auto get_register_array_bits() noexcept
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	return get_register_array_bits<Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>());
}

template<auto BitMask, auto Reg, size_t RegArrIndex, uintptr_t RegStructBase>
[[nodiscard]] auto get_register_array_bits() noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>>;
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
	{
		return static_cast<reg_type>(
			get_register_array_bits<Reg, RegArrIndex>(volatile_memory<class_t, RegStructBase>()) & unsigned_bitmask);
	}
	else
	{
		return reg_type{};
	}
}

template<auto BitMask, auto Reg, size_t RegArrIndex, typename RegStruct>
[[nodiscard]] auto get_register_array_bits(const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<types::type_of_member_pointer_t<Reg>>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
		return static_cast<reg_type>(get_register_array_bits<Reg, RegArrIndex>(ptr) & unsigned_bitmask);
	else
		return reg_type{};
}

template<auto BitMask, auto BitValues, size_t RegArrIndex, typename RegType, typename RegStruct>
void set_register_array_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<RegType>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&((ptr->*reg_ptr)[RegArrIndex]), BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&((ptr->*reg_ptr)[RegArrIndex]));
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		tests::memory::memory_controller::write(&((ptr->*reg_ptr)[RegArrIndex]), value);
	}
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
void set_register_array_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::remove_cv_t<std::remove_all_extents_t<RegType>> values) noexcept
{
	using reg_type = decltype(values);
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory::memory_controller::write(&((ptr->*reg_ptr)[RegArrIndex]), values);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory::memory_controller::read<reg_type>(&((ptr->*reg_ptr)[RegArrIndex]));
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		tests::memory::memory_controller::write(&((ptr->*reg_ptr)[RegArrIndex]), value);
	}
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
void set_register_array_bits(RegType RegStruct::*reg_ptr,
	std::remove_cv_t<std::remove_all_extents_t<RegType>> value) noexcept
{
	set_register_array_bits<BitMask, RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>(), value);
}

template<auto BitMask, auto BitValues,
	size_t RegArrIndex, uintptr_t RegStructBase, typename RegType, typename RegStruct>
void set_register_array_bits(RegType RegStruct::*reg_ptr) noexcept
{
	set_register_array_bits<BitMask, BitValues, RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>());
}

template<size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	return tests::memory::memory_controller::read<std::remove_cv_t<
		std::remove_all_extents_t<RegType>>>(&((ptr->*reg_ptr)[RegArrIndex]));
}

template<size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_array_bits(RegType RegStruct::*reg_ptr) noexcept
{
	return get_register_array_bits<RegArrIndex>(reg_ptr,
		volatile_memory<RegStruct, RegStructBase>());
}

template<auto BitMask, size_t RegArrIndex, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_array_bits(RegType RegStruct::*reg_ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<RegType>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
	{
		return static_cast<reg_type>(get_register_array_bits<RegArrIndex>(reg_ptr,
			volatile_memory<RegStruct, RegStructBase>()) & unsigned_bitmask);
	}
	else
	{
		return reg_type{};
	}
}

template<auto BitMask, size_t RegArrIndex, typename RegType, typename RegStruct>
[[nodiscard]] auto get_register_array_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	using reg_type = std::remove_cv_t<std::remove_all_extents_t<RegType>>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<reg_type>>(BitMask);
	if constexpr (!!unsigned_bitmask)
		return static_cast<reg_type>(get_register_array_bits<RegArrIndex>(reg_ptr, ptr) & unsigned_bitmask);
	else
		return reg_type{};
}

} //namespace mcutl::device::memory
