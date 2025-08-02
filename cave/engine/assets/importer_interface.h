#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/renderer/graphics_dvars.h"

namespace cave {

class IAsset;

class IImporter {
    using CreateImporterFunc = std::unique_ptr<IImporter> (*)(const std::filesystem::path& p_source_path,
                                                              const std::filesystem::path& p_dest_dir);

public:
    IImporter(const std::filesystem::path& p_source_path,
              const std::filesystem::path& p_dest_dir);

    virtual ~IImporter() = default;

    [[nodiscard]] virtual Result<AssetRef> Import() = 0;

    static bool RegisterImporter(const std::string& p_extension, CreateImporterFunc p_func);

    static std::unique_ptr<IImporter> Create(const std::filesystem::path& p_source_path,
                                             const std::filesystem::path& p_dest_dir);

    inline static std::map<std::string, CreateImporterFunc> s_importer_creators;

protected:
    const std::filesystem::path m_source_path;
    std::filesystem::path m_dest_dir;

    std::string m_file_name;
    std::string m_file_path;
    std::string m_base_path;
};

}  // namespace cave
