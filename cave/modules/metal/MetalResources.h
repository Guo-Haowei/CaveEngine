#pragma once
#include "cave/render/rhi/GpuResources.h"

namespace cave::render {

struct MetalGpuBuffer final : GpuResource {
    using GpuResource::GpuResource;

    void* object{};

    ~MetalGpuBuffer() override;

    uint64_t getHandle() const override { return reinterpret_cast<uint64_t>(object); }
};

struct MetalGpuTexture final : GpuResource {
    using GpuResource::GpuResource;

    void* object{};

    ~MetalGpuTexture() override;

    uint64_t getResidentHandle() const final { return 0; }

    uint64_t getHandle() const final { return reinterpret_cast<uint64_t>(object); }

    uint64_t getUavHandle() const final { return getHandle(); }
};

struct MetalGpuMesh final : GpuMesh {
    using GpuMesh::GpuMesh;
};

} // namespace cave::render
