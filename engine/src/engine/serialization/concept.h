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

class ISerializer;
class IDeserializer;

template<typename T>
concept IsSerializable = requires(ISerializer& s, const T& obj) {
    { WriteObject(s, obj) } -> std::same_as<ISerializer&>;
};

template<typename T>
concept IsDeserializable = requires(IDeserializer& d, T& obj) {
    { ReadObject(d, obj) } -> std::same_as<bool>;
};

template<typename T>
concept Serializable = IsSerializable<T> && IsDeserializable<T>;

}  // namespace cave
