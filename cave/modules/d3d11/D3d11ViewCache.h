#pragma once
#include <wrl/client.h>
#include <d3d11_1.h>

#include "cave/core/hash/Hash.h"
#include "cave/core/base/NonCopyable.h"

namespace cave::render {

struct ColorAttachmentDesc;
struct DepthAttachmentDesc;

template<typename T>
struct D3d11TextureKey {
    using Self = D3d11TextureKey<T>;

    ID3D11Resource* resource;
    DXGI_FORMAT format;
    T dimension;
    uint16_t mip_slice;
    uint16_t first_array_slice;
    uint16_t array_size;

    friend bool operator==(const Self&, const Self&) = default;
};

using D3D11RtvKey = D3d11TextureKey<D3D11_RTV_DIMENSION>;
using D3D11DsvKey = D3d11TextureKey<D3D11_DSV_DIMENSION>;

class D3d11ViewCache : public NonCopyable {
public:
    explicit D3d11ViewCache(ID3D11Device* device) noexcept
        : m_device(device) {
    }

    ID3D11RenderTargetView* getOrCreateRtv(const ColorAttachmentDesc& desc);
    ID3D11DepthStencilView* getOrCreateDsv(const DepthAttachmentDesc& desc);

    ~D3d11ViewCache();

    void clear();

private:
    struct KeyHasher {
        template<typename T>
        size_t operator()(const T& key) const {
            size_t h = 0;
            Hash::add(h, key.resource);
            Hash::add(h, key.format);
            Hash::add(h, key.dimension);
            Hash::add(h, key.mip_slice);
            Hash::add(h, key.first_array_slice);
            Hash::add(h, key.array_size);
            return h;
        }
    };

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> createRtv(const D3D11RtvKey& key);
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> createDsv(const D3D11DsvKey& key);

    ID3D11Device* m_device{ nullptr };

    HashMap<D3D11RtvKey, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, KeyHasher> m_rtvs;
    HashMap<D3D11DsvKey, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>, KeyHasher> m_dsvs;
};

}  // namespace cave::render
