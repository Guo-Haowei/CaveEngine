#pragma once
#include "cave/core/hash/Hash.h"
#include "cave/core/ids/Guid.h"

namespace cave {

struct ThumbnailKey {
    Guid guid;
    uint32_t size = 256;

    bool operator==(const ThumbnailKey&) const = default;
};

}  // namespace cave

namespace std {

template<>
struct hash<cave::ThumbnailKey> {
    std::size_t operator()(const cave::ThumbnailKey& p_key) const {
        size_t hash = std::hash<cave::Guid>{}(p_key.guid);
        cave::Hash::Add(hash, p_key.size);
        return hash;
    }
};

}  // namespace std
