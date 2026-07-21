// =============================================================================
// File: cave/runtime/assets/AssetHandle.h
// =============================================================================
#pragma once
#include "cave/core/ids/Guid.h"
#include "cave/runtime/assets/IAsset.h"

namespace cave {

class AssetEntry;
struct AssetMetaData;

// asset forward declaration
class BlobAsset;
struct ImageAsset;
struct MaterialAsset;
class MeshAsset;
class SpriteAnimationAsset;
class TileSetAsset;

class AssetHandle {
public:
    AssetHandle() {}

    AssetHandle(const Guid& guid, Ref<AssetEntry> entry)
        : m_guid(guid)
        , m_asset_entry(std::move(entry)) {}

    void invalidate() {
        m_guid = Guid::null();
        m_asset_entry.reset();
    }

    bool isReady() const;

    IAsset* get() const;

    [[nodiscard]] AssetRef wait() const;

    template<AssetClass T>
    [[nodiscard]] Ref<T> wait() const {
        auto ptr = wait();
        if (!ptr) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<T>(ptr);
    }

    template<AssetClass T>
    inline T* get() const {
        return dynamic_cast<T*>(get());
    }

    const Guid& guid() const { return m_guid; }

    AssetMetaData* meta();
    const AssetMetaData* meta() const;

    static bool replaceGuidAndHandle(AssetType type,
                                     const Guid& guid,
                                     Guid& out_id,
                                     AssetHandle& out_handle);

private:
    Guid m_guid;
    WeakRef<AssetEntry> m_asset_entry;
};

template<typename T>
class Handle : private AssetHandle {
public:
    using AssetHandle::AssetHandle;
    using AssetHandle::guid;
    using AssetHandle::invalidate;
    using AssetHandle::isReady;
    using AssetHandle::meta;

    Handle(const AssetHandle& raw)
        : AssetHandle(raw) {}

    Handle(AssetHandle&& raw)
        : AssetHandle(std::move(raw)) {}

    [[nodiscard]] Ref<T> wait() const {
        static_assert(AssetClass<T>);
        return AssetHandle::wait<T>();
    }

    T* get() const {
        static_assert(AssetClass<T>);
        return AssetHandle::get<T>();
    }

    AssetHandle& rawHandle() { return *this; }
    const AssetHandle& rawHandle() const { return *this; }
};

}  // namespace cave
