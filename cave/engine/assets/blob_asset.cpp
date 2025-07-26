#include "blob_asset.h"

namespace cave {

void BlobAsset::SetBlob(std::vector<char>&& p_blob) {
    m_blob = std::move(p_blob);
    m_blob_size = m_blob.size();
    m_blob.push_back('\0');
}

Result<void> BlobAsset::LoadFromDisk(const AssetMetaData&) {
    return Result<void>();
}

Result<void> BlobAsset::SaveToDisk(const AssetMetaData&) const {
    return Result<void>();
}

std::vector<Guid> BlobAsset::GetDependencies() const {
    return {};
}

}  // namespace cave
