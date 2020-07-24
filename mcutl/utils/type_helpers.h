#pragma once

#include <type_traits>

namespace mcutl::types
{

template<auto Min, auto Max>
struct limits
{
	static constexpr auto min_value = Min;
	static constexpr auto max_value = Max;
	
	static_assert(min_value <= max_value);
	
	template<auto Value>
	[[nodiscard]] static constexpr bool is_valid() noexcept
	{
		return Value >= Min && Value <= Max;
	}
};

template<typename T>
struct class_of_member_pointer
{
	using type = T;
};

template<typename Class, typename T>
struct class_of_member_pointer<T Class::*>
{
	using type = Class;
};

template<auto PointerToMember>
using class_of_member_pointer_t = typename class_of_member_pointer<decltype(PointerToMember)>::type;

template<typename T>
struct member_pointer_type {};
 
template<typename Class, typename T>
struct member_pointer_type<T Class::*>
{
	using type = T;
};

template<typename T>
using member_pointer_type_t = typename member_pointer_type<T>::type;

template<auto PointerToMember>
using type_of_member_pointer_t = member_pointer_type_t<decltype(PointerToMember)>;

template<typename...>
struct always_false { static constexpr bool value = false; };

template<typename...>
struct list {};

template<typename T>
struct identity
{
	using type = T;
};

namespace detail
{

template<template <typename...> typename Container, typename... Containers>
struct merge_containers {};

template<template <typename...> typename Container,
	template <typename...> typename Container1, typename... Types1>
struct merge_containers<Container, Container1<Types1...>>
{
	using type = Container<Types1...>;
};

template<template <typename...> typename Container,
	template <typename...> typename Container1, typename... Types1,
	template <typename...> typename Container2, typename... Types2,
	typename... OtherContainers>
struct merge_containers<Container, Container1<Types1...>,
	Container2<Types2...>, OtherContainers...>
{
	using type = typename merge_containers<Container,
		Container<Types1..., Types2...>, OtherContainers...>::type;
};

template<template <typename...> typename Container, typename...>
struct pop_back {};

template<template <typename...> typename Container>
struct pop_back<Container, list<>>
{
	using type = Container<>;
};

template<template <typename...> typename Container,
	typename... Types, typename Remaining>
struct pop_back<Container, list<Types...>, Remaining>
{
	using type = Container<Types...>;
};

template<template <typename...> typename Container,
	typename... Types,
	typename Remaining, typename Remaining2, typename... OtherRemaining>
struct pop_back<Container, list<Types...>, Remaining, Remaining2, OtherRemaining...>
{
	using type = typename pop_back<Container,
		list<Types..., Remaining>, Remaining2, OtherRemaining...>::type;
};

template<typename Container>
struct pop_back_helper {};

template<template <typename...> typename Container, typename... Types>
struct pop_back_helper<Container<Types...>>
{
	using type = typename pop_back<Container, list<>, Types...>::type;
};

template<typename Container>
struct pop_front_helper {};

template<template <typename...> typename Container>
struct pop_front_helper<Container<>>
{
	using type = Container<>;
};

template<template <typename...> typename Container, typename Type, typename... Types>
struct pop_front_helper<Container<Type, Types...>>
{
	using type = Container<Types...>;
};

} //namespace detail

template<template <typename...> typename Container, typename... Containers>
struct merge_containers
{
	using type = typename detail::merge_containers<Container, Containers...>::type;
};

template<template <typename...> typename Container, typename... Containers>
using merge_containers_t = typename merge_containers<Container, Containers...>::type;

template<typename Container>
struct pop_back
{
	using type = typename detail::pop_back_helper<Container>::type;
};

template<typename Container>
using pop_back_t = typename pop_back<Container>::type;

template<typename Container>
struct pop_front
{
	using type = typename detail::pop_front_helper<Container>::type;
};

template<typename Container>
using pop_front_t = typename pop_front<Container>::type;

} //namespace mcutl::types
