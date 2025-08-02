#include "scene_importer.h"

#include "engine/core/string/string_utils.h"

namespace cave {

namespace fs = std::filesystem;

SceneImporter::SceneImporter(const std::filesystem::path& p_source_path,
                             const std::filesystem::path& p_dest_dir)
    : AssetImporter(p_source_path, p_dest_dir) {

    m_file_name = StringUtils::RemoveExtension(m_source_path.filename().string());

    m_base_path = fs::path(m_base_path).remove_filename().string();

    m_dest_dir = m_dest_dir / m_file_name;
}

Result<void> SceneImporter::PrepareImport() {
    try {
        if (!fs::exists(m_dest_dir)) {
            fs::create_directories(m_dest_dir);
        }
    } catch (const fs::filesystem_error& e) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "{}", e.what());
    }

    return Result<void>();
}

}  // namespace cave
