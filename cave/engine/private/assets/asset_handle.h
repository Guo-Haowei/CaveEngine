#pragma once
#include "asset_interface.h"
#include "guid.h"

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
class TileMapAsset;

class AssetHandle {
public:
    AssetHandle() {}

    AssetHandle(const Guid& p_guid, std::shared_ptr<AssetEntry> p_entry)
        : m_guid(p_guid)
        , m_entry(p_entry) {}

    void Invalidate() {
        m_guid = Guid::Null();
        m_entry.reset();
    }

    bool IsReady() const;

    IAsset* Get() const;

    [[nodiscard]] AssetRef Wait() const;

    template<typename T>
    [[nodiscard]] std::shared_ptr<T> Wait() const {
        auto ptr = Wait();
        if (!ptr) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<T>(ptr);
    }

    template<typename T>
    inline T* Get() const {
        return dynamic_cast<T*>(Get());
    }

    const Guid& GetGuid() const { return m_guid; }

    const AssetMetaData* GetMeta() const;

    static bool ReplaceGuidAndHandle(AssetType p_type,
                                     const Guid& p_guid,
                                     Guid& p_out_id,
                                     AssetHandle& p_out_handle);

private:
    Guid m_guid;
    std::weak_ptr<AssetEntry> m_entry;
};

template<typename T>
class Handle : private AssetHandle {
public:
    using AssetHandle::AssetHandle;
    using AssetHandle::GetGuid;
    using AssetHandle::GetMeta;
    using AssetHandle::Invalidate;
    using AssetHandle::IsReady;

    Handle(const AssetHandle& p_raw)
        : AssetHandle(p_raw) {}

    Handle(AssetHandle&& p_raw)
        : AssetHandle(std::move(p_raw)) {}

    [[nodiscard]] std::shared_ptr<T> Wait() const {
        return AssetHandle::Wait<T>();
    }

    T* Get() const {
        return AssetHandle::Get<T>();
    }

    AssetHandle& RawHandle() { return *this; }
    const AssetHandle& RawHandle() const { return *this; }
};

}  // namespace cave
