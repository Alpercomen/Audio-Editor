#pragma once
#include <type_traits>

#include <Application/Core/Definitions/Definitions.h>


template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, T> 
operator|(T lhs, T rhs)
{
	using uType = std::underlying_type_t<T>;
	return static_cast<T>(
		static_cast<uType>(lhs) | 
		static_cast<uType>(rhs)
	);
}

template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, T&>
operator|=(T& lhs, T rhs)
{
	using uType = std::underlying_type_t<T>;
	lhs = static_cast<T>(
		static_cast<uType>(lhs) |
		static_cast<uType>(rhs)
	);
	return lhs;
}

template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, T>
operator&(T lhs, T rhs)
{
	using uType = std::underlying_type_t<T>;
	return static_cast<T>(
		static_cast<uType>(lhs) &
		static_cast<uType>(rhs)
	);
}

template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, T&>
operator&=(T& lhs, T rhs)
{
	using uType = std::underlying_type_t<T>;
	lhs = static_cast<T>(
		static_cast<uType>(lhs) &
		static_cast<uType>(rhs)
	);
	return lhs;
}

template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, T>
operator~(T value)
{
	using uType = std::underlying_type_t<T>;
	return static_cast<T>(~static_cast<uType>(value));
}

template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, Bool8>
HasFlag(T value, T flag)
{
	using uType = std::underlying_type_t<T>;
	return (static_cast<uType>(value) & static_cast<uType>(flag)) != 0;
}

template <typename T>
constexpr static std::enable_if_t<std::is_enum_v<T>, Bool8>
IsSameFlag(T a, T b)
{
	using uType = std::underlying_type_t<T>;
	return static_cast<uType>(a) == static_cast<uType>(b);
}