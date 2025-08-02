#include "asset_loader.h"

#include "engine/assets/blob_asset.h"
#include "engine/assets/image_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/pixel_format.h"
#include "engine/scene/scene.h"

namespace cave {

IAssetLoader::IAssetLoader(const std::string& p_import_path)
    : m_import_path(p_import_path) {
    std::filesystem::path system_path{ m_filePath };
    m_fileName = system_path.filename().string();
    m_basePath = system_path.remove_filename().string();
}

bool IAssetLoader::RegisterLoader(const std::string& p_extension, CreateLoaderFunc p_func) {
    DEV_ASSERT(!p_extension.empty());
    DEV_ASSERT(p_func);
    auto it = s_loaderCreator.find(p_extension);
    if (it != s_loaderCreator.end()) {
        LOG_WARN("Already exists a loader for p_extension '{}'", p_extension);
        it->second = p_func;
    } else {
        s_loaderCreator[p_extension] = p_func;
    }
    return true;
}

std::unique_ptr<IAssetLoader> IAssetLoader::Create(const std::string& p_import_path) {
    std::string_view extension = StringUtils::Extension(p_import_path);
    auto it = s_loaderCreator.find(std::string(extension));
    if (it == s_loaderCreator.end()) {
        return nullptr;
    }
    return it->second(p_import_path);
}

}  // namespace cave
