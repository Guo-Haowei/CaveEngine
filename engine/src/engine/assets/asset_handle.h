#pragma once
#include "asset_interface.h"
#include "guid.h"

namespace my {

class AssetEntry;
struct AssetMetaData;
struct IAsset;

class AssetHandle {
public:
    AssetHandle() {}

    AssetHandle(const Guid& p_guid, std::shared_ptr<AssetEntry> p_entry)
        : m_guid(p_guid)
        , m_entry(p_entry) {}

    bool IsReady() const;

    IAsset* Get();

    [[nodiscard]] auto Wait() const -> Result<AssetRef>;

    template<typename T>
    [[nodiscard]] auto Wait() -> Result<std::shared_ptr<T>> {
        auto res = Wait();
        if (!res) {
            return HBN_ERROR(res.error());
        }

        AssetRef ptr = *res;
        return std::dynamic_pointer_cast<T>(ptr);
    }

    template<typename T>
    T* Get() {
        return dynamic_cast<T*>(Get());
    }

    const Guid& GetGuid() const { return m_guid; }

    const AssetMetaData* GetMeta() const;

private:
    Guid m_guid;
    std::weak_ptr<AssetEntry> m_entry;
};

}  // namespace my
