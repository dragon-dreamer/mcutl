#pragma once

#include <any>
#if defined(_GLIBCXX_RELEASE)
#	if _GLIBCXX_RELEASE == 9
#		include <experimental/any>
#		define MCUTL_USE_EXPERIMENTAL_ANY
#	endif
#endif
#include <cassert>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mcutl/instruction/instruction_defs.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::tests::instruction
{

#ifdef MCUTL_USE_EXPERIMENTAL_ANY
using any_t = std::experimental::any;
#else //MCUTL_USE_EXPERIMENTAL_ANY
using any_t = std::any;
#endif //MCUTL_USE_EXPERIMENTAL_ANY

template<typename T, typename Any>
decltype(auto) any_cast(Any&& value)
{
#ifdef MCUTL_USE_EXPERIMENTAL_ANY
	return std::experimental::any_cast<T>(std::forward<Any>(value));
#else //MCUTL_USE_EXPERIMENTAL_ANY
	return std::any_cast<T>(std::forward<Any>(value));
#endif //MCUTL_USE_EXPERIMENTAL_ANY
}

using instruction_return_type = any_t;
using instruction_args_type = std::vector<any_t>;

class instruction_interface
{
public:
	virtual ~instruction_interface() {}

public:
	virtual instruction_return_type run(
		std::type_index instruction_type,
		const instruction_args_type& args) = 0;
};

class instruction_interface_mock : public instruction_interface
{
public:
	void initialize_default_behavior()
	{
		using namespace ::testing;
		ON_CALL(*this, run).WillByDefault(
			[] (std::type_index, const instruction_args_type&) {
				return instruction_return_type {};
			});
	}
	
	MOCK_METHOD(instruction_return_type, run,
		(std::type_index instruction_type, const instruction_args_type& args),
		(override));
};

class instruction_controller
{
public:
	instruction_controller() = delete;
	static void set_interface(instruction_interface* instance) noexcept
	{
		interface_ = instance;
	}
	
public:
	template<typename Instruction, typename... Args>
	static auto run(Args... args)
	{
		assert(interface_);
		
		auto result = interface_->run(typeid(Instruction), instruction_args_type { args... });
		using return_type = mcutl::instruction::detail::return_type_t<Instruction>;
		EXPECT_EQ(typeid(return_type), result.type());
		if constexpr (!std::is_same_v<return_type, void>)
			return any_cast<return_type>(std::move(result));
	}
	
private:
	static inline instruction_interface* interface_;
};

class test_fixture_base : virtual public ::testing::Test
{
public:
	virtual void SetUp() override
	{
		instruction_controller::set_interface(&instruction_mock_);
		instruction_mock_.initialize_default_behavior();
	}

	virtual void TearDown() override
	{
		instruction_controller::set_interface(nullptr);
	}

	[[nodiscard]] instruction_interface_mock& instruction() noexcept
	{
		return instruction_mock_;
	}
	
	template<typename Instruction>
	[[nodiscard]] static auto instr()
	{
		return std::type_index(typeid(Instruction));
	}

private:
	::testing::StrictMock<instruction_interface_mock> instruction_mock_;
};

template<typename... Args>
class InstructionArgsEqualMatcher
	: public ::testing::MatcherInterface<const mcutl::tests::instruction::instruction_args_type&>
{
public:
	template<typename... T>
	explicit InstructionArgsEqualMatcher(T&&... args)
		: expected_{ std::forward<T>(args)... }
	{
	}
	
	virtual bool MatchAndExplain(const mcutl::tests::instruction::instruction_args_type& values,
		::testing::MatchResultListener* listener) const override
	{
		if (values.size() != expected_.size())
		{
			*listener << "the actual size is " << values.size();
			return false;
		}
		
		if constexpr (sizeof...(Args) != 0)
			return arg_equal<Args...>(values, 0, listener);
		else
			return true;
	}

	void DescribeTo(std::ostream* os) const override
	{
		*os << "instruction args are equal";
	}

	void DescribeNegationTo(std::ostream* os) const override
	{
		*os << "instruction args are not equal";
	}
	
private:
	template<typename T, typename... Ts>
	bool arg_equal(const mcutl::tests::instruction::instruction_args_type& values,
		std::size_t index,
		::testing::MatchResultListener* listener) const
	{
		if (values[index].type() != expected_[index].type())
		{
			*listener << "the actual type of arg# " << index << " is "
				<< testing::PrintToString(values[index].type().name());
			return false;
		}
		
		if (mcutl::tests::instruction::any_cast<T>(values[index])
			!= mcutl::tests::instruction::any_cast<T>(expected_[index]))
		{
			*listener << "the actual value of arg# " << index << " is "
				<< testing::PrintToString(mcutl::tests::instruction::any_cast<T>(expected_[index]));
			return false;
		}
		
		if constexpr (sizeof...(Ts) != 0)
			return arg_equal<Ts...>(values, index + 1, listener);
		else
			return true;
	}
	
private:
	mcutl::tests::instruction::instruction_args_type expected_;
};

template<typename... Args>
auto InstructionArgsEqual(Args&&... args)
{
	return::testing::MakeMatcher(new InstructionArgsEqualMatcher<Args...>(std::forward<Args>(args)...));
}

} //namespace mcutl::tests::instruction

namespace mcutl::instruction::detail
{

template<typename Instruction>
struct executor<instruction_type<Instruction>>
{
	template<typename... Args>
	static auto run(Args... args)
	{
		return tests::instruction::instruction_controller::run<Instruction>(args...);
	}
};

} //namespace mcutl::tests
