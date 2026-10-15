#pragma once

#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

struct MetalBuffer final : GpuBuffer {
    using GpuBuffer::GpuBuffer;

    void* object{};

    ~MetalBuffer() override;

    uint64_t GetHandle() const final { return reinterpret_cast<uint64_t>(object); }
};

struct MetalTexture final : GpuTexture {
    using GpuTexture::GpuTexture;

    void* object{};

    ~MetalTexture() override;

    uint64_t GetResidentHandle() const final { return 0; }

    uint64_t GetHandle() const final { return reinterpret_cast<uint64_t>(object); }

    uint64_t GetUavHandle() const final { return GetHandle(); }
};

struct MetalConstantBuffer final : GpuConstantBuffer {
    using GpuConstantBuffer::GpuConstantBuffer;

    void* object{};

    ~MetalConstantBuffer() override;
};

struct MetalGpuMesh final : GpuMesh {
    using GpuMesh::GpuMesh;
};

} // namespace cave::render