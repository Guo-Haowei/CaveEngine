#include "scene_importer.h"

#include "engine/assets/mesh_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

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

std::string SceneImporter::NameGenerator(std::string_view p_name, uint32_t& p_counter) {
    ++p_counter;
    return std::format("{}_{}", p_name, p_counter);
}

Result<Guid> SceneImporter::RegisterMesh(std::string&& p_mesh_name,
                                         std::shared_ptr<MeshAsset>&& p_mesh) {
    fs::path sys_path = m_dest_dir / std::format("{}.mesh", p_mesh_name);

    Guid guid = Guid::Create();
    AssetMetaData meta;
    meta.type = AssetType::Mesh;
    meta.name = std::move(p_mesh_name);
    meta.guid = guid;
    meta.import_path = IAssetManager::GetSingleton().ResolvePath(sys_path);

    if (auto res = p_mesh->SaveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    AssetRegistry::GetSingleton().RegisterAsset(std::move(meta), p_mesh);

    // @TODO: move it to somewhere else, if it's headless, no need to create gpu data
    GraphicsManager::GetSingleton().RequestMesh(p_mesh.get());

    return Result<Guid>(guid);
}

Result<void> SceneImporter::RegisterScene(ecs::Entity p_root) {
    m_scene->m_root = p_root;
    m_scene->GetComponent<NameComponent>(p_root)->SetName(m_file_name);

    fs::path sys_path = m_dest_dir / std::format("{}.scene", m_file_name);

    AssetMetaData meta;
    meta.type = AssetType::Scene;
    meta.name = m_file_name;
    meta.guid = Guid::Create();
    meta.import_path = IAssetManager::GetSingleton().ResolvePath(sys_path);
    AssetRegistry::GetSingleton().RegisterAsset(std::move(meta), m_scene);

    return m_scene->SaveToDisk(meta);
}

}  // namespace cave
