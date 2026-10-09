#pragma once
#include <d3d11_1.h>

#include "cave/core/hash/Hash.h"

namespace cave::render {

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

}  // namespace cave::render

namespace std {

template<typename T>
struct hash<cave::render::D3d11TextureKey<T>> {
    std::size_t operator()(const cave::render::D3d11TextureKey<T>& key) const {
        size_t hash = 0;
        cave::Hash::add(hash, key.resource);
        cave::Hash::add(hash, key.format);
        cave::Hash::add(hash, key.dimension);
        cave::Hash::add(hash, key.mip_slice);
        cave::Hash::add(hash, key.first_array_slice);
        cave::Hash::add(hash, key.array_size);
        return hash;
    }
};

}  // namespace std
