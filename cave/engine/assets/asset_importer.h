#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/renderer/graphics_dvars.h"

namespace cave {

class IAsset;

class AssetImporter {
    using CreateImporterFunc = std::unique_ptr<AssetImporter> (*)(const std::filesystem::path& p_source_path,
                                                                  const std::filesystem::path& p_dest_dir);

public:
    AssetImporter(const std::filesystem::path& p_source_path,
                  const std::filesystem::path& p_dest_dir);

    virtual ~AssetImporter() = default;

    virtual Result<void> Import() = 0;

    static bool RegisterImporter(const std::string& p_extension, CreateImporterFunc p_func);

    static std::unique_ptr<AssetImporter> Create(const std::filesystem::path& p_source_path,
                                                 const std::filesystem::path& p_dest_dir);

    inline static std::map<std::string, CreateImporterFunc> s_importer_creators;

protected:
    std::filesystem::path m_source_path;
    std::filesystem::path m_dest_dir;
};

}  // namespace cave
