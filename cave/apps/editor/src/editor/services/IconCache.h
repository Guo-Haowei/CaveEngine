#pragma once
#include "engine/private/runtime/assets/ImageAsset.h"

namespace cave {

class AssetRegistry;
class IAssetManager;

enum class IconName {
    Checkerboard,
    Folder,
    Meta,
    Scene,
    Anim,
    Lua,
    TileMap,
    TileSet,
    Count,
};

constexpr int kMaxIcons = std::to_underlying(IconName::Count);

class IconCache {
public:
    explicit IconCache(AssetRegistry& p_asset_reg,
                       IAssetManager& p_asset_manager);
    ~IconCache();

    void Init();
    void Clear();

    GpuTextureId GetIcon(IconName p_name) const;
    uint64_t GetIconHandle(IconName p_name) const;

private:
    AssetRegistry& m_asset_reg;
    IAssetManager& m_asset_manager;

    std::array<std::shared_ptr<ImageAsset>, kMaxIcons> m_cache;
};

}  // namespace cave
