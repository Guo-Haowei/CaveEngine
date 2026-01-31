#pragma once

namespace cave::render {

class RGTextureId {
public:
    using Type = uint16_t;

    constexpr RGTextureId() noexcept
        : m_value(0) {}

    constexpr RGTextureId(Type p_value) noexcept
        : m_value(p_value) {}

    bool IsNull() const { return m_value == 0; }

    static constexpr RGTextureId Null() {
        return RGTextureId{};
    }

    constexpr Type Underlying() const { return m_value; }

    std::strong_ordering operator<=>(const RGTextureId&) const = default;

private:
    Type m_value{};
};

}  // namespace cave::render

namespace std {

template<>
struct hash<cave::render::RGTextureId> {
    std::size_t operator()(const cave::render::RGTextureId& p_handle) const {
        return std::hash<uint32_t>{}(p_handle.Underlying());
    }
};

}  // namespace std
