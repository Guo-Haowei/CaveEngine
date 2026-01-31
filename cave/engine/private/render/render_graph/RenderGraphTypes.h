#pragma once
// @TODO: refactor
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

namespace cave::render {

using RGImportFunc = std::function<std::shared_ptr<GpuTexture>()>;

// clang-format off
enum class ResourceAccess : uint8_t {
    NONE   = 0,
    SRV    = BIT(0),
    UAV    = BIT(1),
    RTV    = BIT(2),
    DSV    = BIT(3),
    // Present,
    // CopySrc,
    // CopyDst,
    // DepthRead,
    // DepthWrite,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(ResourceAccess);

class RGTextureHandle {
public:
    using Type = uint16_t;

    constexpr RGTextureHandle() noexcept
        : m_value(0) {}

    constexpr RGTextureHandle(Type p_value) noexcept
        : m_value(p_value) {}

    bool IsNull() const { return m_value == 0; }

    static constexpr RGTextureHandle Null() {
        return RGTextureHandle{};
    }

    constexpr Type Underlying() const { return m_value; }

    std::strong_ordering operator<=>(const RGTextureHandle&) const = default;

private:
    Type m_value{};
};

struct RGTextureNode {
    RGTextureHandle handle;
    GpuTextureDesc desc{};
    SamplerDesc sampler{};
    RGImportFunc import_fn;
    ResourceAccess access_mask{ ResourceAccess::NONE };

    std::string debug_name;
};

}  // namespace cave::render

namespace std {

template<>
struct hash<cave::render::RGTextureHandle> {
    std::size_t operator()(const cave::render::RGTextureHandle& p_handle) const {
        return std::hash<uint32_t>{}(p_handle.Underlying());
    }
};

}  // namespace std
