#pragma once
#include "OpenGlPrerequisites.h"

#include "cave/core/hash/Hash.h"

namespace cave::render {

struct RenderTargetDesc;

constexpr size_t kMaxColorAttachments = 8;

enum class AttachKind : uint8_t {
    None,
    Tex2D,
    CubeFace,
    Layer,
    Layered,
};

struct FboAttachmentKey {
    enum : uint8_t {
        DepthStencil = 254,
        Depth = 255,
    };

    uint32_t tex = 0;
    AttachKind kind = AttachKind::None;
    uint16_t mip = 0;
    uint16_t first_slice = 0;      // array slice OR cube face (0..5)
    uint16_t slice_count = 1;      // 1 for normal RT usage
    uint8_t attachment_point = 0;  // 0..kMaxColorAttachments-1 for color, 255 for depth, 254 for depth stencil

    bool operator==(const FboAttachmentKey&) const = default;

    size_t Hash(size_t& inout) const noexcept {
        cave::Hash::add(inout, tex);
        cave::Hash::add(inout, kind);
        cave::Hash::add(inout, mip);
        cave::Hash::add(inout, first_slice);
        cave::Hash::add(inout, slice_count);
        cave::Hash::add(inout, attachment_point);
        return inout;
    }
};

struct FboKey {
    uint8_t numColors = 0;
    uint8_t hasDepthStencil = 0;

    FboAttachmentKey colors[kMaxColorAttachments]{};
    FboAttachmentKey depthStencil{};

    bool operator==(const FboKey&) const = default;
};

struct FboKeyHash {
    size_t operator()(const FboKey& key) const noexcept {
        size_t hash = 0;
        cave::Hash::add(hash, key.numColors);
        cave::Hash::add(hash, key.hasDepthStencil);
        for (size_t i = 0; i < cave::render::kMaxColorAttachments; ++i) {
            key.colors[i].Hash(hash);
        }
        key.depthStencil.Hash(hash);
        return hash;
    }
};

class OpenGlFramebufferCache {
public:
    struct Stats {
        uint32_t fbo_count = 0;
        uint32_t fbo_hits = 0;
        uint32_t fbo_misses = 0;
    };

    explicit OpenGlFramebufferCache() noexcept;
    ~OpenGlFramebufferCache();

    void clear();

    GLuint getOrCreateFbo(const RenderTargetDesc& desc);

    Stats getStats() const;
    void resetStats() { m_stats = {}; }

private:
    GLuint createFbo(const FboKey& key);

    HashMap<FboKey, GLuint, FboKeyHash> m_fbos;

    mutable Stats m_stats{};
};

}  // namespace cave::render
