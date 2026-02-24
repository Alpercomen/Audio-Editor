#pragma once

#include <cstdint>

typedef char char8;
typedef int8_t Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;
typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;

typedef size_t Usize;

typedef float Float32;
typedef double Float64;

typedef bool Bool8;

constexpr Uint64 Uint64_max = 0xFFFFFFFFFFFFFFFF;
constexpr Uint32 Uint32_max = 0xFFFFFFFF;
constexpr Uint16 Uint16_max = 0xFFFF;
constexpr Uint8 Uint8_max = 0xFF;

constexpr Float32 ClipWidth = 200.0;

#include <type_traits>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;