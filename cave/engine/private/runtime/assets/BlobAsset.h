#pragma once
#include "cave/runtime/assets/IAsset.h"

namespace cave {

class BlobAsset : public IAsset {
    CAVE_ASSET(BlobAsset, AssetType::Blob, 0)

    std::vector<char> m_blob;

    size_t m_blob_size = 0;

public:
    void SetBlob(std::vector<char>&& p_blob);

    const char* GetBufferPointer() const { return m_blob.data(); }
    size_t GetBufferLength() const { return m_blob_size; }

    const char* c_str() const { return m_blob.data() ? m_blob.data() : ""; }

    Result<void> loadFromDisk(const AssetMetaData&) override;

    Result<void> saveToDisk(const AssetMetaData&) const override;

    std::vector<Guid> dependencies() const override;
};

}  // namespace cave
