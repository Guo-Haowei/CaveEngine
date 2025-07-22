#pragma once
#include "asset_interface.h"
#include "asset_handle.h"
#include "asset_meta_data.h"

namespace cave {

class BlobAsset : public IAsset {
    CAVE_ASSET(BlobAsset, AssetType::Blob, 0)

    std::vector<char> m_blob;

    size_t m_blob_size = 0;

public:
    void SetBlob(std::vector<char>&& p_blob);

    const char* GetBufferPoiner() const { return m_blob.data(); }
    size_t GetBufferLength() const { return m_blob_size; }

    const char* c_str() const { return m_blob.data() ? m_blob.data() : ""; }

    Result<void> LoadFromDisk(const AssetMetaData&) override;

    Result<void> SaveToDisk(const AssetMetaData&) const override;

    std::vector<Guid> GetDependencies() const override;
};

}  // namespace cave
