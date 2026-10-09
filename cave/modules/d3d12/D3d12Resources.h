#ifdef D3D12_RESOURCES_INCLUDED
#error DO NOT INCLUDE THIS FILE IN HEADER
#endif
#define D3D12_RESOURCES_INCLUDED

#include "D3d12Core.h"

namespace cave::render {

// @TODO: refactor
struct D3d12GpuTexture : public GpuTexture {
    using GpuTexture::GpuTexture;

    uint64_t GetHandle() const final { return srvHandle.gpuHandle.ptr; }

    uint64_t GetResidentHandle() const final {
        uint64_t handle = srvHandle.index;
        switch (desc.dimension) {
            case Dimension::Texture2D:
            case Dimension::TextureCubeArray:
                return handle;
            default:
                CRASH_NOW();
                return 0;
        }
    }

    uint64_t GetUavHandle() const final { return uavHandle.index; }

    Microsoft::WRL::ComPtr<ID3D12Resource> texture;

    DescriptorHeapHandle srvHandle;
    DescriptorHeapHandle uavHandle;

    D3D12_RESOURCE_STATES currentState{ D3D12_RESOURCE_STATE_COMMON };
};

}  // namespace cave::render
