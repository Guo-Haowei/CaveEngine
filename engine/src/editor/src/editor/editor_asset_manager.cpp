#include "editor_asset_manager.h"

#include "engine/assets/asset_loader.h"
#include "engine/assets/image_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/application.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

namespace fs = std::filesystem;

[[nodiscard]] static auto CreateImageAsset(const std::string& p_import_path) -> Result<std::shared_ptr<ImageAsset>> {
    auto loader = IAssetLoader::Create(p_import_path);
    if (!loader) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "no loader found for '{}'", p_import_path);
    }

    auto res = loader->Load();
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    return std::dynamic_pointer_cast<ImageAsset>(*res);
}

Result<void> EditorAssetManager::AddAlwaysLoadImages() {
    // @TODO: fix this path, it won't work if the file is moved
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

            auto res = CreateImageAsset(path.string());
            if (!res) {
                return CAVE_ERROR(res.error());
            }
            auto image = *res;
            m_images[file_name.string()] = image;
            m_app->GetGraphicsManager()->RequestTexture(image.get());
        }
    }

    return Result<void>();
}

Result<void> EditorAssetManager::InitializeImpl() {
    if (auto res = AssetManager::InitializeImpl(); !res) {
        return std::unexpected(res.error());
    }

    return AddAlwaysLoadImages();
}

std::shared_ptr<ImageAsset> EditorAssetManager::FindImage(const std::string& p_name) {
    auto it = m_images.find(p_name);
    if (it == m_images.end()) {
        return nullptr;
    }
    return it->second;
}

}  // namespace cave
