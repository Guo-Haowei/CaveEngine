#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include "engine/private/render/rhi/gpu_resource.h"

namespace cave::render {

struct D3d11GpuTexture : public GpuTexture {
    using GpuTexture::GpuTexture;

    uint64_t getResidentHandle() const final { return 0; }
    uint64_t getHandle() const final { return (uint64_t)srv.Get(); }
    uint64_t getUavHandle() const final { return (uint64_t)uav.Get(); }

    Microsoft::WRL::ComPtr<ID3D11Resource> texture;

    // @TODO: refactor render target and texture entirely
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    // @TODO: this shouldn't be here
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
};

struct D3d11Buffer : public GpuBuffer {
    using GpuBuffer::GpuBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> internal_buffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    mutable const char* data = nullptr;

    uint64_t getHandle() const final {
        return (size_t)internal_buffer.Get();
    }
};

struct D3d11MeshBuffers : GpuMesh {
    using GpuMesh::GpuMesh;
};

}  // namespace cave::render