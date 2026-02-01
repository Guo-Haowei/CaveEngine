#pragma once
#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

// @NOTE: maybe don't need it
enum class TextureAspect : uint8_t {
    Color,
    Depth,
    Stencil,
    DepthStencil,
};

enum class LoadOp : uint8_t {
    Load,
    Clear,
    DontCare,
};

enum class StoreOp : uint8_t {
    Store,
    DontCare,
};

struct TextureViewDesc {
    TextureAspect aspect = TextureAspect::Color;
    uint16_t mip_slice = 0;          // D3D: MipSlice
    uint16_t first_array_slice = 0;  // D3D: FirstArraySlice / First2DArrayFace
    uint16_t array_size = 1;         // D3D: ArraySize / NumCubes * 6
};

using GpuTextureId = std::shared_ptr<GpuTexture>;

struct ColorAttachmentDesc {
    GpuTextureId tex;
    TextureViewDesc view;
    LoadOp load = LoadOp::Load;
    StoreOp store = StoreOp::Store;
    float clear_color[4] = { 0, 0, 0, 0 };
};

struct DepthAttachmentDesc {
    GpuTextureId tex;
    TextureViewDesc view;
    LoadOp load = LoadOp::Load;
    StoreOp store = StoreOp::Store;
    float clear_depth = 1.0f;
    uint8_t clear_stencil = 0;
};

struct RenderTargetDesc {
    enum Type : uint8_t {
        Texture = 0,
        Screen,
    };

    Type type = Texture;
    std::vector<ColorAttachmentDesc> colors;
    DepthAttachmentDesc depth;
};

struct RenderTarget {
    RenderTarget(RenderTargetDesc p_desc)
        : desc(std::move(p_desc)) {
    }

    // @TODO: refactor this
    std::tuple<uint32_t, uint32_t> GetBufferSize() const {
        if (desc.depth.tex) {
            return std::make_tuple(desc.depth.tex->desc.width, desc.depth.tex->desc.height);
        }

        DEV_ASSERT(!desc.colors.empty());
        return std::make_tuple(desc.colors[0].tex->desc.width,
                               desc.colors[0].tex->desc.height);
    }

    RenderTargetDesc desc;
};

}  // namespace cave::render
