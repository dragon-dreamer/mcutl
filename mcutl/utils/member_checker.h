#pragma once

#include <type_traits>

#define MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(memberName) \
	template<typename T, typename = std::true_type, typename = std::false_type> \
	struct has_ ## memberName : std::false_type {}; \
	template<typename T> \
    struct has_ ## memberName<T, \
		typename std::is_integral<decltype(std::declval<T>().memberName)>::type, \
		typename std::is_const<decltype(std::declval<T>().memberName)>::type> \
		: std::true_type {}; \
	template<typename T> \
	[[maybe_unused]] static constexpr bool has_ ## memberName ## _v = has_ ## memberName<T>::value;
