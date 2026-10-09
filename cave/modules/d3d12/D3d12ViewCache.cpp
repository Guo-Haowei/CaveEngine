#include "D3d12ViewCache.h"

#include "D3d12RenderDevice.h"
#include "D3d12Resources.h"

#define INCLUDE_AS_D3D12
#include "../d3d_common/D3dConvert.h"

namespace cave::render {

D3D12_CPU_DESCRIPTOR_HANDLE D3d12ViewCache::getOrCreateRtv(const ColorAttachmentDesc& attachment) {
    DEV_ASSERT(attachment.tex);
    auto* d3dTex = static_cast<D3d12GpuTexture*>(attachment.tex.get());
    ID3D12Resource* resource = d3dTex->texture.Get();
    DXGI_FORMAT format = d3d::Convert(d3dTex->desc.format);

    RtvKey key{
        .resource = resource,
        .format = format,
        .mipSlice = attachment.view.mip_slice,
        .firstArraySlice = attachment.view.first_array_slice,
        .arraySize = attachment.view.array_size > 0 ? attachment.view.array_size : 1u,
    };

    if (auto it = m_rtvCache.find(key); it != m_rtvCache.end()) {
        return it->second;
    }

    DescriptorHeapHandle handle = m_rtvHeap.AllocHandle();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = format;
    if (key.arraySize > 1) {
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = key.mipSlice;
        rtvDesc.Texture2DArray.FirstArraySlice = key.firstArraySlice;
        rtvDesc.Texture2DArray.ArraySize = key.arraySize;
    } else {
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = key.mipSlice;
    }

    m_device->CreateRenderTargetView(resource, &rtvDesc, handle.cpuHandle);
    m_rtvCache[key] = handle.cpuHandle;
    return handle.cpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3d12ViewCache::getOrCreateDsv(const DepthAttachmentDesc& attachment) {
    DEV_ASSERT(attachment.tex);
    auto* d3dTex = static_cast<D3d12GpuTexture*>(attachment.tex.get());
    ID3D12Resource* resource = d3dTex->texture.Get();

    const DXGI_FORMAT format = d3d::ToDsvFormat(d3d::Convert(d3dTex->desc.format));

    DsvKey key{
        .resource = resource,
        .format = format,
        .mipSlice = attachment.view.mip_slice,
        .firstArraySlice = attachment.view.first_array_slice,
        .arraySize = attachment.view.array_size > 0 ? attachment.view.array_size : 1u,
    };

    if (auto it = m_dsvCache.find(key); it != m_dsvCache.end()) {
        return it->second;
    }

    DescriptorHeapHandle handle = m_dsvHeap.AllocHandle();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = format;
    if (key.arraySize > 1) {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = key.mipSlice;
        dsvDesc.Texture2DArray.FirstArraySlice = key.firstArraySlice;
        dsvDesc.Texture2DArray.ArraySize = key.arraySize;
    } else {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = key.mipSlice;
    }

    m_device->CreateDepthStencilView(resource, &dsvDesc, handle.cpuHandle);
    m_dsvCache[key] = handle.cpuHandle;
    return handle.cpuHandle;
}

}  // namespace cave::render
#undef INCLUDE_AS_D3D12
