#pragma once

#include <cstddef>
#include <type_traits>

namespace mcutl::types
{

template<typename...>
struct always_false { static constexpr bool value = false; };

template<auto...>
struct value_always_false { static constexpr bool value = false; };

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
	static_assert(always_false<T>::value, "T is not a member pointer");
};

template<typename Class, typename T>
struct class_of_member_pointer<T Class::*>
{
	using type = Class;
};

template<auto PointerToMember>
using class_of_member_pointer_t = typename class_of_member_pointer<decltype(PointerToMember)>::type;

template<typename PointerToMember>
using class_of_member_pointer_type_t = typename class_of_member_pointer<PointerToMember>::type;

template<typename T>
struct member_pointer_type
{
	static_assert(always_false<T>::value, "T is not a member pointer");
};
 
template<typename Class, typename T>
struct member_pointer_type<T Class::*>
{
	using type = T;
};

template<typename T>
using member_pointer_type_t = typename member_pointer_type<T>::type;

template<auto PointerToMember>
using type_of_member_pointer_t = member_pointer_type_t<decltype(PointerToMember)>;

template<typename PointerToMember>
using type_of_member_pointer_type_t = member_pointer_type_t<PointerToMember>;

template<typename... T>
struct list
{
	static constexpr auto length = sizeof...(T);
};

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

template<typename Container, typename Type>
struct push_back_helper {};

template<template <typename...> typename Container, typename... Types, typename Type>
struct push_back_helper<Container<Types...>, Type>
{
	using type = Container<Types..., Type>;
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

template<typename Container, typename Type>
struct push_back
{
	using type = typename detail::push_back_helper<Container, Type>::type;
};

template<typename Container, typename Type>
using push_back_t = typename push_back<Container, Type>::type;

namespace detail
{

template<template <typename, typename> typename IsSame, typename...>
struct duplicate_helper : std::bool_constant<false> {};

template<template <typename, typename> typename IsSame, typename T, typename... Other>
struct duplicate_helper<IsSame, T, Other...> : std::bool_constant<false> {};

template<template <typename, typename> typename IsSame,
	typename T, typename Other, typename... Others>
struct duplicate_helper<IsSame, T, Other, Others...>
{
	static constexpr bool value = IsSame<T, Other>::value
		|| duplicate_helper<IsSame, T, Others...>::value
		|| duplicate_helper<IsSame, Other, Others...>::value;
};

} //namespace detail

template<typename... T>
[[maybe_unused]] constexpr bool has_duplicates_v = detail::duplicate_helper<std::is_same, T...>::value;

template<template <typename, typename> typename IsSame, typename... T>
[[maybe_unused]] constexpr bool has_duplicates_filtered_v = detail::duplicate_helper<IsSame, T...>::value;

namespace detail
{

template<template <typename...> typename Container, typename...>
struct remove_helper
{
};

template<template <typename...> typename Container, typename T, typename... Filtered>
struct remove_helper<Container, T, Container<Filtered...>, Container<>>
{
	using type = Container<Filtered...>;
};

template<template <typename...> typename Container,
	typename T, typename... Filtered, typename ToFilter, typename... OtherToFilter>
struct remove_helper<Container, T, Container<Filtered...>, Container<ToFilter, OtherToFilter...>>
{
	using type = typename std::conditional<std::is_same_v<T, ToFilter>,
		remove_helper<Container, T, Container<Filtered...>, Container<OtherToFilter...>>,
		remove_helper<Container, T, Container<Filtered..., ToFilter>, Container<OtherToFilter...>>
	>::type::type;
};

template<typename Remove, typename Container>
struct remove_container_helper {};

template<typename Remove, template <typename...> typename Container, typename... Types>
struct remove_container_helper<Remove, Container<Types...>>
{
	using type = typename remove_helper<Container, Remove, Container<>, Container<Types...>>::type;
};

} //namespace detail

template<typename Remove, typename... T>
using remove_t = typename detail::remove_helper<list, Remove, list<>, list<T...>>::type;

template<typename Remove, typename Container>
using remove_from_container_t = typename detail::remove_container_helper<Remove, Container>::type;

namespace detail
{

template<typename...>
struct first_type {};

template<typename T, typename... Ts>
struct first_type<T, Ts...>
{
	using type = T;
};

} //namespace detail

template<typename... T>
using first_type_t = typename detail::first_type<T...>::type;

namespace detail
{

template<typename T, typename List>
struct type_index
{
	static_assert(always_false<T>::value, "Invalid type list type");
};

template<typename T, template <typename...> typename List>
struct type_index<T, List<>>
{
	static_assert(always_false<T>::value, "No type T in given type list");
};

template<typename T, template <typename...> typename List, typename... Types>
struct type_index<T, List<T, Types...>>
{
	static constexpr std::size_t value = 0;
};

template<typename T, template <typename...> typename List, typename U, typename... Types>
struct type_index<T, List<U, Types...>>
{
	static constexpr std::size_t value = 1 + type_index<T, List<Types...>>::value;
};

} //namespace detail

template<typename T, typename TypeList>
[[maybe_unused]] constexpr auto type_index_v = detail::type_index<T, TypeList>::value;

namespace detail
{

template<std::size_t Index, typename List>
struct type_by_index
{
	static_assert(always_false<List>::value, "Invalid type list type");
};

template<std::size_t Index, template <typename...> typename List>
struct type_by_index<Index, List<>>
{
	static_assert(value_always_false<Index>::value, "Too large index value");
};

template<std::size_t Index, template <typename...> typename List, typename Type, typename... Types>
struct type_by_index<Index, List<Type, Types...>>
{
	using type = typename std::conditional_t<Index == 0,
		identity<Type>, type_by_index<Index - 1, List<Types...>>>::type;
};

} //namespace detail

template<std::size_t Index, typename List>
using type_by_index_t = typename detail::type_by_index<Index, List>::type;

} //namespace mcutl::types
