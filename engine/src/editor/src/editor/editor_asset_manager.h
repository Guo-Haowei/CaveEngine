#pragma once
#include "engine/assets/asset_manager.h"

namespace cave {

struct ImageAsset;

class EditorAssetManager : public AssetManager {
public:
    Result<void> InitializeImpl() override;

    std::shared_ptr<ImageAsset> FindImage(const std::string& p_name);

protected:
    std::unordered_map<std::string, std::shared_ptr<ImageAsset>> m_images;
};

}  // namespace cave
