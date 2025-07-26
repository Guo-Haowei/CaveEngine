#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/renderer/graphics_dvars.h"

namespace cave {

class IAsset;

class IAssetLoader {
    using CreateLoaderFunc = std::unique_ptr<IAssetLoader> (*)(const std::string& p_import_path);

public:
    IAssetLoader(const std::string& p_import_path);

    virtual ~IAssetLoader() = default;

    [[nodiscard]] virtual auto Load() -> Result<AssetRef> = 0;

    static bool RegisterLoader(const std::string& p_extension, CreateLoaderFunc p_func);

    static std::unique_ptr<IAssetLoader> Create(const std::string& p_import_path);

    inline static std::map<std::string, CreateLoaderFunc> s_loaderCreator;

protected:
    const std::string& m_import_path;

    std::string m_fileName;
    std::string m_filePath;
    std::string m_basePath;
    std::string m_extension;
};

class BufferAssetLoader : public IAssetLoader {
public:
    using IAssetLoader::IAssetLoader;

    static std::unique_ptr<IAssetLoader> CreateLoader(const std::string& p_import_path) {
        return std::make_unique<BufferAssetLoader>(p_import_path);
    }

    auto Load() -> Result<AssetRef> override;
};

}  // namespace cave
