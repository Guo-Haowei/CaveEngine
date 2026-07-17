#pragma once

namespace cave::render {

class RGTextureId {
public:
    using Type = uint16_t;

    constexpr RGTextureId() noexcept
        : m_value(0) {}

    constexpr RGTextureId(Type value) noexcept
        : m_value(value) {}

    bool isNull() const { return m_value == 0; }

    constexpr Type underlying() const { return m_value; }

    std::strong_ordering operator<=>(const RGTextureId&) const = default;

    static RGTextureId null() {
        return RGTextureId{};
    }

private:
    Type m_value{};
};

struct RGDependencyId : public RGTextureId {
    explicit RGDependencyId(RGTextureId id) noexcept
        : RGTextureId(id) {
    }

    static RGDependencyId null() {
        return RGDependencyId(RGTextureId{});
    }
};

}  // namespace cave::render

template<>
struct std::hash<cave::render::RGTextureId> {
    std::size_t operator()(const cave::render::RGTextureId& handle) const {
        return std::hash<uint32_t>{}(handle.underlying());
    }
};
