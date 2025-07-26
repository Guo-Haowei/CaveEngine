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

template<typename T>
struct ElementType;

template<typename T, typename Alloc>
struct ElementType<std::vector<T, Alloc>> {
    using type = T;
};

template<typename T, std::size_t N>
struct ElementType<std::array<T, N>> {
    using type = T;
};

template<typename T>
using ElementType_t = typename ElementType<T>::type;

template<typename T>
concept ArrayLike =
    requires {
        typename ElementType_t<T>;
    };

template<typename T>
concept ArrayOfTrivial =
    ArrayLike<T> &&
    std::is_trivially_copyable_v<ElementType_t<T>>;

template<typename T>
concept HasResize = requires(T t, size_t n) {
    t.resize(n);
};

static_assert(ArrayLike<std::vector<int>>);
static_assert(ArrayLike<std::array<float, 4>>);
static_assert(!ArrayLike<int>);

static_assert(ArrayOfTrivial<std::vector<int>>);
static_assert(!ArrayOfTrivial<std::vector<std::string>>);

template<typename T>
struct MapTraits;

template<typename K, typename V, typename... Rest>
struct MapTraits<std::map<K, V, Rest...>> {
    using key_type = K;
    using mapped_type = V;
};

template<typename K, typename V, typename... Rest>
struct MapTraits<std::unordered_map<K, V, Rest...>> {
    using key_type = K;
    using mapped_type = V;
};

template<typename T>
concept StringMap = requires {
    typename MapTraits<T>::key_type;
    typename MapTraits<T>::mapped_type;
} && std::is_same_v<typename MapTraits<T>::key_type, std::string>;

static_assert(StringMap<std::map<std::string, int>>);
static_assert(StringMap<std::unordered_map<std::string, float>>);
static_assert(!StringMap<std::map<int, std::string>>);

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
