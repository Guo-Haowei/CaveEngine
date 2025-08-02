#pragma once
#include "engine/assets/asset_importer.h"

namespace cave {

class Scene;

class SceneImporter : public AssetImporter {
public:
    SceneImporter(const std::filesystem::path& p_source_path,
                  const std::filesystem::path& p_dest_dir);

protected:
    Result<void> PrepareImport();

    std::string m_file_name;
    std::string m_file_path;
    std::string m_base_path;

    std::shared_ptr<Scene> m_scene;
};

}  // namespace cave
