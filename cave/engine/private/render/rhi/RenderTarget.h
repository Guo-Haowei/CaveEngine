#pragma once
#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

enum class LoadOp : uint8_t {
    Load = 0,
    Clear,
    DontCare,
};

enum class StoreOp : uint8_t {
    Store = 0,
};

struct TextureViewDesc {
    uint16_t mip_slice = 0;          // D3D: MipSlice
    uint16_t first_array_slice = 0;  // D3D: FirstArraySlice / First2DArrayFace
    uint16_t array_size = 1;         // D3D: ArraySize / NumCubes * 6
};

// @TODO: use actual id
using GpuTextureId = std::shared_ptr<GpuTexture>;

struct ColorAttachmentDesc {
    GpuTextureId tex{};
    TextureViewDesc view{};
    LoadOp load{ LoadOp::Load };
    float clear_color[4]{ 0, 0, 0, 0 };
};

struct DepthAttachmentDesc {
    GpuTextureId tex{};
    TextureViewDesc view{};
    LoadOp depth_load{ LoadOp::Load };
    float clear_depth{ 1.0f };
    LoadOp stencil_load{ LoadOp::Load };
    uint8_t clear_stencil{ 0 };
};

struct RenderTargetDesc {
    std::span<const ColorAttachmentDesc> colors;
    std::optional<DepthAttachmentDesc> depth;
};

}  // namespace cave::render
