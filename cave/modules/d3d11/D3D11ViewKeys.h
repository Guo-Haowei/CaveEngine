#pragma once
#include <d3d11_1.h>

#include "cave/core/hash/Hash.h"

namespace cave::render {

template<typename T>
struct D3D11TextureKey {
    using Self = D3D11TextureKey<T>;

    ID3D11Resource* resource;
    DXGI_FORMAT format;
    T dimension;
    uint16_t mip_slice;
    uint16_t first_array_slice;
    uint16_t array_size;

    friend bool operator==(const Self&, const Self&) = default;
};

using D3D11RtvKey = D3D11TextureKey<D3D11_RTV_DIMENSION>;
using D3D11DsvKey = D3D11TextureKey<D3D11_DSV_DIMENSION>;

}  // namespace cave::render

namespace std {

template<typename T>
struct hash<cave::render::D3D11TextureKey<T>> {
    std::size_t operator()(const cave::render::D3D11TextureKey<T>& p_key) const {
        size_t hash = 0;
        cave::Hash::Add(hash, p_key.resource);
        cave::Hash::Add(hash, p_key.format);
        cave::Hash::Add(hash, p_key.dimension);
        cave::Hash::Add(hash, p_key.mip_slice);
        cave::Hash::Add(hash, p_key.first_array_slice);
        cave::Hash::Add(hash, p_key.array_size);
        return hash;
    }
};

}  // namespace std
