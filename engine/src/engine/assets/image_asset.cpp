#include "image_asset.h"

namespace cave {

Result<void> ImageAsset::LoadFromDisk(const AssetMetaData&) {
    // @TODO: get sampler from meta
    // @TODO: load with xxx
    return Result<void>();
}

Result<void> ImageAsset::SaveToDisk(const AssetMetaData&) const {
    // @TODO: get sampler from meta
    // @TODO: load with xxx
    return Result<void>();
}

std::vector<Guid> ImageAsset::GetDependencies() const {
    return {};
}

}  // namespace cave
