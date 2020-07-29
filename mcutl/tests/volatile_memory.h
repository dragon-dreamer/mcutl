#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::tests
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
	
	memory_interface_not_mocked with_unmocked_memory() noexcept
	{
		return memory_interface_not_mocked(*this);
	}
	
	void allow_reads(memory_address_t address)
	{
		EXPECT_CALL(*this, read(address)).Times(::testing::AtLeast(0));
	}
	
	uint64_t get(memory_address_t address) const
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
	
	uint64_t& get(memory_address_t address)
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
		auto address = reinterpret_cast<memory_address_t>(pointer);
		interface_->write(address, static_cast<uint64_t>(value));
	}
	
	template<typename Value, typename Ptr>
	static Value read(const Ptr* pointer)
	{
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

class test_fixture_base : public ::testing::Test
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

	memory_interface_mock& memory() noexcept
	{
		return memory_mock_;
	}

	template<typename Pointer>
	static memory_address_t addr(const Pointer* pointer) noexcept
	{
		return mcutl::tests::addr(pointer);
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

} //namespace mcutl::tests

namespace mcutl::device::memory
{

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

template<auto BitMask, std::make_unsigned_t<decltype(BitMask)> BitValues, auto Reg, typename RegStruct>
void set_register_bits([[maybe_unused]] volatile RegStruct* ptr)
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory_controller::write(&(ptr->*Reg), BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		using value_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
		auto value = tests::memory_controller::read<value_t>(&(ptr->*Reg));
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		tests::memory_controller::write(&(ptr->*Reg), value);
	}
}

template<auto BitMask, auto Reg, typename RegStruct>
void set_register_bits([[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] std::make_unsigned_t<decltype(BitMask)> values)
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory_controller::write(&(ptr->*Reg), values);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		using value_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
		auto value = tests::memory_controller::read<value_t>(&(ptr->*Reg));
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		tests::memory_controller::write(&(ptr->*Reg), value);
	}
}

template<auto BitMask, auto Reg, uintptr_t RegStructBase>
void set_register_bits(decltype(BitMask) value)
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, Reg>(volatile_memory<class_t, RegStructBase>(), value);
}

template<auto BitMask, decltype(BitMask) BitValues, auto Reg, uintptr_t RegStructBase>
void set_register_bits()
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	set_register_bits<BitMask, BitValues, Reg>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr)
{
	using value_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	return tests::memory_controller::read<value_t>(&(ptr->*Reg));
}

template<auto Reg, uintptr_t RegStructBase>
[[nodiscard]] inline auto get_register_bits()
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	return get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>());
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline auto get_register_bits()
{
	using class_t = mcutl::types::class_of_member_pointer_t<Reg>;
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	return static_cast<types::type_of_member_pointer_t<Reg>>(
		get_register_bits<Reg>(volatile_memory<class_t, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(const volatile RegStruct* ptr)
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	return static_cast<types::type_of_member_pointer_t<Reg>>(get_register_bits<Reg>(ptr) & unsigned_bitmask);
}

template<auto Reg, uintptr_t RegStructBase, auto BitMask>
[[nodiscard]] inline bool get_register_flag()
{
	return static_cast<bool>(get_register_bits<Reg, RegStructBase, BitMask>());
}

template<auto BitMask, auto Reg, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(const volatile RegStruct* ptr)
{
	return static_cast<bool>(get_register_bits<BitMask, Reg>(ptr));
}

template<auto Value, auto Reg, typename RegStruct>
inline void set_register_value(volatile RegStruct* ptr)
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, static_cast<bitmask_t>(Value), Reg>(ptr);
}

template<auto Reg, typename RegStruct, typename Value>
inline void set_register_value(volatile RegStruct* ptr, Value value)
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, Reg>(ptr, value);
}

template<auto Reg, uintptr_t RegStructBase, typename Value>
inline void set_register_value(Value value)
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>, Reg, RegStructBase>(value);
}

template<auto Value, auto Reg, uintptr_t RegStructBase>
inline void set_register_value()
{
	using bitmask_t = std::remove_cv_t<mcutl::types::type_of_member_pointer_t<Reg>>;
	set_register_bits<max_bitmask<std::make_unsigned_t<bitmask_t>>,
		static_cast<bitmask_t>(Value), Reg, RegStructBase>();
}

template<auto BitMask, decltype(BitMask) BitValues, typename RegType, typename RegStruct>
inline void set_register_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr)
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory_controller::write(&(ptr->*reg_ptr), BitValues);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory_controller::read<RegType>(&(ptr->*reg_ptr));
		value &= ~unsigned_bitmask;
		value |= (BitValues & unsigned_bitmask);
		tests::memory_controller::write(&(ptr->*reg_ptr), value);
	}
}

template<auto BitMask, typename RegType, typename RegStruct>
inline void set_register_bits([[maybe_unused]] RegType RegStruct::*reg_ptr,
	[[maybe_unused]] volatile RegStruct* ptr,
	[[maybe_unused]] decltype(BitMask) values)
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	if constexpr (unsigned_bitmask == max_bitmask<decltype(unsigned_bitmask)>)
	{
		tests::memory_controller::write(&(ptr->*reg_ptr), values);
	}
	else if constexpr (!!unsigned_bitmask)
	{
		auto value = tests::memory_controller::read<RegType>(&(ptr->*reg_ptr));
		value &= ~unsigned_bitmask;
		value |= (unsigned_bitmask & values);
		tests::memory_controller::write(&(ptr->*reg_ptr), value);
	}
}

template<auto BitMask, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr, decltype(BitMask) value)
{
	set_register_bits<BitMask>(reg_ptr, volatile_memory<RegStruct, RegStructBase>(), value);
}

template<auto BitMask, decltype(BitMask) BitValues, uintptr_t RegStructBase,
	typename RegType, typename RegStruct>
inline void set_register_bits(RegType RegStruct::*reg_ptr)
{
	set_register_bits<BitMask, BitValues>(reg_ptr, volatile_memory<RegStruct, RegStructBase>());
}

template<auto Value, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr) noexcept
{
	set_register_bits<max_bitmask<std::make_unsigned_t<RegType>>, static_cast<RegType>(Value)>(reg_ptr, ptr);
}

template<typename RegType, typename RegStruct, typename Value>
inline void set_register_value(RegType RegStruct::*reg_ptr, volatile RegStruct* ptr, Value value) noexcept
{
	set_register_bits<max_bitmask<std::make_unsigned_t<RegType>>>(reg_ptr, ptr, value);
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct, typename Value>
inline void set_register_value(RegType RegStruct::*reg_ptr, Value value) noexcept
{
	set_register_bits<max_bitmask<std::make_unsigned_t<RegType>>, RegStructBase>(reg_ptr, value);
}

template<auto Value, uintptr_t RegStructBase, typename RegType, typename RegStruct>
inline void set_register_value(RegType RegStruct::*reg_ptr) noexcept
{
	set_register_bits<max_bitmask<std::make_unsigned_t<RegType>>,
		static_cast<RegType>(Value), RegStructBase>(reg_ptr);
}

template<typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	return tests::memory_controller::read<RegType>(&(ptr->*reg_ptr));
}

template<uintptr_t RegStructBase, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	return get_register_bits(reg_ptr, volatile_memory<RegStruct, RegStructBase>());
}

template<uintptr_t RegStructBase, auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr) noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	return static_cast<RegType>(
		get_register_bits(reg_ptr, volatile_memory<RegStruct, RegStructBase>()) & unsigned_bitmask);
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline auto get_register_bits(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	constexpr auto unsigned_bitmask = static_cast<std::make_unsigned_t<decltype(BitMask)>>(BitMask);
	return static_cast<RegType>(get_register_bits(reg_ptr, ptr) & unsigned_bitmask);
}

template<uintptr_t RegStructBase, auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(RegType RegStruct::*reg_ptr) noexcept
{
	return static_cast<bool>(get_register_bits<RegStructBase, BitMask>(reg_ptr));
}

template<auto BitMask, typename RegType, typename RegStruct>
[[nodiscard]] inline bool get_register_flag(RegType RegStruct::*reg_ptr,
	const volatile RegStruct* ptr) noexcept
{
	return static_cast<bool>(get_register_bits<BitMask>(reg_ptr, ptr));
}

} //namespace mcutl::tests
