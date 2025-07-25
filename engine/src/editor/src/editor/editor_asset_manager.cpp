#include "editor_asset_manager.h"

#include "engine/assets/asset_loader.h"
#include "engine/assets/image_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/application.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

namespace fs = std::filesystem;

[[nodiscard]] static std::shared_ptr<ImageAsset> CreateImageAsset(const std::string& p_import_path) {
    auto loader = IAssetLoader::Create(p_import_path);
    if (!loader) {
        return nullptr;
    }

    auto res = loader->Load();
    if (!res) {
        LOG_ERROR("Failed to load {}", p_import_path);
        return nullptr;
    }

    return std::dynamic_pointer_cast<ImageAsset>(*res);
}

Result<void> EditorAssetManager::InitializeImpl() {
    if (auto res = AssetManager::InitializeImpl(); !res) {
        return std::unexpected(res.error());
    }

    std::string_view tmp = StringUtils::BasePath(__FILE__);
    tmp = StringUtils::BasePath(tmp);
    tmp = StringUtils::BasePath(tmp);
    fs::path image_folder = tmp;
    image_folder = image_folder / "resources" / "images";
    DEV_ASSERT(fs::is_directory(image_folder));
    for (const auto& entry : fs::directory_iterator(image_folder)) {
        if (entry.is_regular_file()) {
            fs::path path = entry.path();
            fs::path file_name = path.filename();

            if (auto image = CreateImageAsset(path.string()); image) {
                m_images[file_name.string()] = image;
                m_app->GetGraphicsManager()->RequestTexture(image.get());
            }
        }
    }

    return Result<void>();
}

std::shared_ptr<ImageAsset> EditorAssetManager::FindImage(const std::string& p_name) {
    auto it = m_images.find(p_name);
    if (it == m_images.end()) {
        return nullptr;
    }
    return it->second;
}

}  // namespace cave
