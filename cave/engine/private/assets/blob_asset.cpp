#include "blob_asset.h"

#include "engine/core/io/file_access.h"

namespace cave {

void BlobAsset::SetBlob(std::vector<char>&& p_blob) {
    m_blob = std::move(p_blob);
    m_blob_size = m_blob.size();
    m_blob.push_back('\0');
}

Result<void> BlobAsset::LoadFromDisk(const AssetMetaData& p_meta) {
    auto res = FileAccess::Open(p_meta.import_path, FileAccess::READ);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    std::shared_ptr<FileAccess> file_access = *res;

    const size_t size = file_access->GetLength();

    std::vector<char> buffer;
    buffer.resize(size);
    [[maybe_unused]] const size_t read = file_access->ReadBuffer(buffer.data(), size);
    DEV_ASSERT(read == size);

    SetBlob(std::move(buffer));

    return Result<void>();
}

Result<void> BlobAsset::SaveToDisk(const AssetMetaData& p_meta) const {
    return p_meta.SaveToDisk(this);
}

std::vector<Guid> BlobAsset::GetDependencies() const {
    return {};
}

}  // namespace cave
