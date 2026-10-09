#include "D3d11ViewCache.h"

#include "engine/private/render/rhi/RenderTarget.h"

// @TODO: refactor
#include "D3d11Resources.h"
#include "../d3d_common/D3dConvert.h"

namespace cave::render {

using Microsoft::WRL::ComPtr;

D3d11ViewCache::~D3d11ViewCache() {
    clear();
    m_device = nullptr;
}

void D3d11ViewCache::clear() {
    m_rtvs.clear();
    m_dsvs.clear();
}

static D3D11RtvKey MakeRtvKey(const ColorAttachmentDesc& desc);
static D3D11DsvKey MakeDsvKey(const DepthAttachmentDesc& desc);

ID3D11RenderTargetView* D3d11ViewCache::getOrCreateRtv(const ColorAttachmentDesc& desc) {
    const D3D11RtvKey key = MakeRtvKey(desc);

    auto [it, inserted] = m_rtvs.try_emplace(key);
    if (inserted) {
        it->second = createRtv(key);
    }

    return it->second.Get();
}

ID3D11DepthStencilView* D3d11ViewCache::getOrCreateDsv(const DepthAttachmentDesc& desc) {
    const D3D11DsvKey key = MakeDsvKey(desc);

    auto [it, inserted] = m_dsvs.try_emplace(key);
    if (inserted) {
        it->second = createDsv(key);
    }

    return it->second.Get();
}

ComPtr<ID3D11RenderTargetView> D3d11ViewCache::createRtv(const D3D11RtvKey& key) {
    D3D11_RENDER_TARGET_VIEW_DESC desc{};
    desc.Format = key.format;
    desc.ViewDimension = key.dimension;
    desc.Texture2DArray.MipSlice = key.mip_slice;
    desc.Texture2DArray.FirstArraySlice = key.first_array_slice;
    desc.Texture2DArray.ArraySize = key.array_size;

    ComPtr<ID3D11RenderTargetView> rtv;
    m_device->CreateRenderTargetView(key.resource, &desc, rtv.GetAddressOf());
    return rtv;
}

ComPtr<ID3D11DepthStencilView> D3d11ViewCache::createDsv(const D3D11DsvKey& key) {
    D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
    desc.Format = key.format;
    desc.ViewDimension = key.dimension;
    desc.Texture2DArray.MipSlice = key.mip_slice;
    desc.Texture2DArray.FirstArraySlice = key.first_array_slice;
    desc.Texture2DArray.ArraySize = key.array_size;

    ComPtr<ID3D11DepthStencilView> dsv;
    m_device->CreateDepthStencilView(key.resource, &desc, dsv.GetAddressOf());
    return dsv;
}

static D3D11RtvKey MakeRtvKey(const ColorAttachmentDesc& desc) {
    const D3d11GpuTexture* tex = reinterpret_cast<const D3d11GpuTexture*>(desc.tex.get());

    DXGI_FORMAT format = d3d::Convert(tex->desc.format);
    D3D11_RTV_DIMENSION dimension{};
    switch (tex->desc.type) {
        case AttachmentType::COLOR_2D: {
            dimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        } break;
        case AttachmentType::COLOR_CUBE: {
            dimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        } break;
        default: {
            CRASH_NOW();
        } break;
    }

    return {
        .resource = tex->texture.Get(),
        .format = format,
        .dimension = dimension,
        .mip_slice = desc.view.mip_slice,
        .first_array_slice = desc.view.first_array_slice,
        .array_size = desc.view.array_size,
    };
}

static D3D11DsvKey MakeDsvKey(const DepthAttachmentDesc& desc) {
    const D3d11GpuTexture* tex = reinterpret_cast<const D3d11GpuTexture*>(desc.tex.get());

    DXGI_FORMAT format = d3d::ToDsvFormat(d3d::Convert(tex->desc.format));
    D3D11_DSV_DIMENSION dimension{};

    // @TODO: do not rely on attachment type
    switch (tex->desc.type) {
        case AttachmentType::DEPTH_STENCIL_2D: {
            dimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        } break;
        case AttachmentType::DEPTH_2D:
        case AttachmentType::SHADOW_2D: {
            dimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        } break;
        case AttachmentType::SHADOW_CUBE_ARRAY: {
            dimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            CRASH_NOW();
        } break;
        default: {
            CRASH_NOW();
        } break;
    }

    return {
        .resource = tex->texture.Get(),
        .format = format,
        .dimension = dimension,
        .mip_slice = desc.view.mip_slice,
        .first_array_slice = desc.view.first_array_slice,
        .array_size = desc.view.array_size,
    };
}

}  // namespace cave::render
