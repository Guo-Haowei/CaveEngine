#pragma once
#include <ranges>
#include <type_traits>

namespace cave {

template<typename T>
concept IsArithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept IsEnum = std::is_enum_v<T>;

template<typename T>
concept IsRange = std::ranges::range<T>;

template<typename T>
concept IsSizedRange = std::ranges::sized_range<T>;

}  // namespace cave
