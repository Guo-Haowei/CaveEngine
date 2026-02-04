#include "IconCache.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "editor/EditorAssetManager.h"

namespace cave {

IconCache::IconCache(AssetRegistry& p_asset_reg,
                     IAssetManager& p_asset_manager)
    : m_asset_reg(p_asset_reg)
    , m_asset_manager(p_asset_manager) {
    Init();
}

IconCache::~IconCache() {
    Clear();
}

void IconCache::Init() {
    std::array<const char*, kMaxIcons> paths = {
        "@persist://textures/checkerboard",
        "folder_icon.png",
        "meta_icon.png",
        "scene@256x256.png",
        "anim@256x256.png",
        "script@256x256.png",
        "tileset@256x256.png",
        "tileset@256x256.png",
    };

    for (int i = 0; i < std::to_underlying(IconName::Folder); ++i) {
        auto handle = m_asset_reg.FindByPath<ImageAsset>(paths[i]);
        if (handle.is_none()) {
            continue;
        }
        m_cache[i] = handle.unwrap_unchecked().Wait();
    }

    auto& asset_manager = static_cast<EditorAssetManager&>(m_asset_manager);
    for (int i = std::to_underlying(IconName::Folder); i < kMaxIcons; ++i) {
        m_cache[i] = asset_manager.FindImage(paths[i]);
    }
}

void IconCache::Clear() {
    for (auto& asset : m_cache) {
        if (asset) {
            asset.reset();
        }
    }
}

GpuTextureId IconCache::GetIcon(IconName p_name) const {
    DEV_ASSERT_INDEX(p_name, IconName::Count);
    const auto& image = m_cache[std::to_underlying(p_name)];
    return image ? image->gpu_texture : nullptr;
}

uint64_t IconCache::GetIconHandle(IconName p_name) const {
    auto texture = GetIcon(p_name);
    return texture ? texture->GetHandle() : 0;
}

}  // namespace cave
