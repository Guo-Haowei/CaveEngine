#include "D3D11ViewCache.h"

#include "engine/private/render/rhi/RenderTarget.h"

// @TODO: refactor
#include "d3d11_resources.h"
#include "../d3d_common/d3d_convert.h"

namespace cave::render {

using Microsoft::WRL::ComPtr;

D3D11ViewCache::D3D11ViewCache(ID3D11Device* p_device) noexcept
    : m_device(p_device) {
}

D3D11ViewCache::~D3D11ViewCache() {
    Clear();
    m_device = nullptr;
}

void D3D11ViewCache::Clear() {
    ResetStats();
    m_rtvs.clear();
    m_dsvs.clear();
}

static D3D11RtvKey MakeRtvKey(const ColorAttachmentDesc& p_desc);
static D3D11DsvKey MakeDsvKey(const DepthAttachmentDesc& p_desc);

ID3D11RenderTargetView* D3D11ViewCache::GetOrCreateRtv(const ColorAttachmentDesc& p_desc) {
    const D3D11RtvKey key = MakeRtvKey(p_desc);

    auto [it, inserted] = m_rtvs.try_emplace(key);
    if (inserted) {
        it->second = CreateRtv(key);
        ++m_stats.rtv_misses;
        // LOG("rtv cache miss {}", p_desc.tex->desc.name);
    } else {
        ++m_stats.rtv_hits;
    }

    return it->second.Get();
}

ID3D11DepthStencilView* D3D11ViewCache::GetOrCreateDsv(const DepthAttachmentDesc& p_desc) {
    const D3D11DsvKey key = MakeDsvKey(p_desc);

    auto [it, inserted] = m_dsvs.try_emplace(key);
    if (inserted) {
        it->second = CreateDsv(key);
        ++m_stats.dsv_misses;
        // LOG("dsv cache miss {}", p_desc.tex->desc.name);
    } else {
        ++m_stats.dsv_hits;
    }

    return it->second.Get();
}

ComPtr<ID3D11RenderTargetView> D3D11ViewCache::CreateRtv(const D3D11RtvKey& p_key) {
    D3D11_RENDER_TARGET_VIEW_DESC desc{};
    desc.Format = p_key.format;
    desc.ViewDimension = p_key.dimension;
    desc.Texture2DArray.MipSlice = p_key.mip_slice;
    desc.Texture2DArray.FirstArraySlice = p_key.first_array_slice;
    desc.Texture2DArray.ArraySize = p_key.array_size;

    ComPtr<ID3D11RenderTargetView> rtv;
    m_device->CreateRenderTargetView(p_key.resource, &desc, rtv.GetAddressOf());
    return rtv;
}

ComPtr<ID3D11DepthStencilView> D3D11ViewCache::CreateDsv(const D3D11DsvKey& p_key) {
    D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
    desc.Format = p_key.format;
    desc.ViewDimension = p_key.dimension;
    desc.Texture2DArray.MipSlice = p_key.mip_slice;
    desc.Texture2DArray.FirstArraySlice = p_key.first_array_slice;
    desc.Texture2DArray.ArraySize = p_key.array_size;

    ComPtr<ID3D11DepthStencilView> dsv;
    m_device->CreateDepthStencilView(p_key.resource, &desc, dsv.GetAddressOf());
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

    DXGI_FORMAT format{};
    D3D11_DSV_DIMENSION dimension{};

    // @TODO: do not rely on attachment type
    switch (tex->desc.type) {
        case AttachmentType::DEPTH_STENCIL_2D: {
            format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            dimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        } break;
        case AttachmentType::DEPTH_2D:
        case AttachmentType::SHADOW_2D: {
            format = DXGI_FORMAT_D32_FLOAT;
            dimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        } break;
        case AttachmentType::SHADOW_CUBE_ARRAY: {
            format = DXGI_FORMAT_D32_FLOAT;
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

D3D11ViewCache::Stats D3D11ViewCache::GetStats() const {
    m_stats.rtv_count = static_cast<uint32_t>(m_rtvs.size());
    m_stats.dsv_count = static_cast<uint32_t>(m_dsvs.size());

    return m_stats;
}

}  // namespace cave::render
