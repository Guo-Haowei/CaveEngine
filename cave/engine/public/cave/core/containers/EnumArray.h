#pragma once
#include <cstddef>
#include <type_traits>

namespace cave {

template<typename Enum, typename T, size_t N>
struct EnumArray {
    static_assert(std::is_enum_v<Enum>, "EnumArray requires Enum to be an enum type");

    T data[N]{};

    constexpr T& operator[](Enum e) noexcept {
        return data[static_cast<size_t>(e)];
    }

    constexpr const T& operator[](Enum e) const noexcept {
        return data[static_cast<size_t>(e)];
    }

    constexpr T* begin() noexcept { return data; }
    constexpr T* end() noexcept { return data + N; }
    constexpr const T* begin() const noexcept { return data; }
    constexpr const T* end() const noexcept { return data + N; }

    static constexpr size_t size() noexcept { return N; }
};

} // namespace cave