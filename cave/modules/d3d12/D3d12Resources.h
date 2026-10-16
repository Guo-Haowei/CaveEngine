#pragma once
#include "D3d12Core.h"

namespace cave::render {

struct D3d12MeshBuffers : GpuMesh {
    using GpuMesh::GpuMesh;

    D3D12_VERTEX_BUFFER_VIEW vbvs[kMeshMaxVertexBufferCount];
    D3D12_INDEX_BUFFER_VIEW ibv;
};

struct D3d12Buffer final : public GpuBuffer {
    using GpuBuffer::GpuBuffer;

    ~D3d12Buffer() {
        if (buffer && mappedData) {
            buffer->Unmap(0, nullptr);
            mappedData = nullptr;
        }
    }

    uint64_t getHandle() const override {
        return (size_t)buffer.Get();
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> buffer{};
    uint8_t* mappedData{ nullptr };
    DescriptorHeapHandle handle;
};

// @TODO: refactor
struct D3d12GpuTexture : public GpuTexture {
    using GpuTexture::GpuTexture;

    uint64_t getHandle() const final { return srv_handle.gpu_handle.ptr; }

    uint64_t getResidentHandle() const final {
        uint64_t handle = srv_handle.index;
        switch (desc.dimension) {
            case Dimension::Texture2D:
            case Dimension::TextureCube:
                return handle;
            default:
                CRASH_NOW();
                return 0;
        }
    }

    uint64_t getUavHandle() const final { return uav_handle.index; }

    Microsoft::WRL::ComPtr<ID3D12Resource> texture;

    DescriptorHeapHandle srv_handle;
    DescriptorHeapHandle uav_handle;

    D3D12_RESOURCE_STATES current_state{ D3D12_RESOURCE_STATE_COMMON };
};

}  // namespace cave::render
