#pragma once
#include <d3d12.h>
#include "D3d12Core.h"
#include "engine/private/render/rhi/RenderTarget.h"

#include "cave/core/base/NonCopyable.h"

namespace cave::render {

class D3d12ViewCache : public NonCopyable {
public:
    explicit D3d12ViewCache(ID3D12Device4* device, DescriptorHeap& rtvHeap, DescriptorHeap& dsvHeap)
        : m_device(device), m_rtvHeap(rtvHeap), m_dsvHeap(dsvHeap) {}

    D3D12_CPU_DESCRIPTOR_HANDLE getOrCreateRtv(const ColorAttachmentDesc& attachment);
    D3D12_CPU_DESCRIPTOR_HANDLE getOrCreateDsv(const DepthAttachmentDesc& attachment);

private:
    struct RtvKey {
        ID3D12Resource* resource{ nullptr };
        DXGI_FORMAT format{ DXGI_FORMAT_UNKNOWN };
        uint32_t mipSlice{ 0 };
        uint32_t firstArraySlice{ 0 };
        uint32_t arraySize{ 1 };

        bool operator==(const RtvKey& other) const = default;
    };

    struct DsvKey {
        ID3D12Resource* resource{ nullptr };
        DXGI_FORMAT format{ DXGI_FORMAT_UNKNOWN };
        uint32_t mipSlice{ 0 };
        uint32_t firstArraySlice{ 0 };
        uint32_t arraySize{ 1 };

        bool operator==(const DsvKey& other) const = default;
    };

    struct KeyHasher {
        template<typename T>
        size_t operator()(const T& k) const {
            size_t h = std::hash<void*>{}(k.resource);
            h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(k.format)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(k.mipSlice) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(k.firstArraySlice) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(k.arraySize) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    ID3D12Device4* m_device{ nullptr };
    DescriptorHeap& m_rtvHeap;
    DescriptorHeap& m_dsvHeap;

    HashMap<RtvKey, D3D12_CPU_DESCRIPTOR_HANDLE, KeyHasher> m_rtvCache;
    HashMap<DsvKey, D3D12_CPU_DESCRIPTOR_HANDLE, KeyHasher> m_dsvCache;
};

}  // namespace cave::render