#include "importer_interface.h"

#include "engine/assets/blob_asset.h"
#include "engine/assets/image_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/renderer/pixel_format.h"
#include "engine/scene/scene.h"

namespace cave {

namespace fs = std::filesystem;

IImporter::IImporter(const fs::path& p_source_path,
                     const fs::path& p_dest_dir)
    : m_source_path(p_source_path)
    , m_dest_dir(p_dest_dir) {
    m_file_name = m_source_path.filename().string();
    m_base_path = fs::path(m_base_path).remove_filename().string();
}

bool IImporter::RegisterImporter(const std::string& p_extension, CreateImporterFunc p_func) {
    DEV_ASSERT(!p_extension.empty());
    DEV_ASSERT(p_func);
    auto it = s_importer_creators.find(p_extension);
    if (it != s_importer_creators.end()) {
        LOG_WARN("Already exists a loader for p_extension '{}'", p_extension);
        it->second = p_func;
    } else {
        s_importer_creators[p_extension] = p_func;
    }
    return true;
}

std::unique_ptr<IImporter> IImporter::Create(const fs::path& p_source_path,
                                             const fs::path& p_dest_dir) {
    std::string ext = p_source_path.extension().string();

    auto it = s_importer_creators.find(ext);
    if (it == s_importer_creators.end()) {
        return nullptr;
    }

    return it->second(p_source_path, p_dest_dir);
}

}  // namespace cave
