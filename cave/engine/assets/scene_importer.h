#pragma once
#include "engine/assets/asset_importer.h"
#include "engine/assets/guid.h"

namespace cave {

class MeshAsset;
class Scene;

class SceneImporter : public AssetImporter {
public:
    SceneImporter(const std::filesystem::path& p_source_path,
                  const std::filesystem::path& p_dest_dir);

protected:
    Result<void> PrepareImport();

    Result<Guid> RegisterMesh(std::string&& p_mesh_name,
                              std::shared_ptr<MeshAsset>&& p_mesh);

    Result<void> RegisterScene(ecs::Entity p_root);

    std::string GenerateMeshName() { return NameGenerator("mesh", m_mesh_counter); }
    std::string GenerateMatName() { return NameGenerator("mat", m_mat_counter); }

    std::string m_file_name;
    std::string m_file_path;
    std::string m_base_path;

    std::vector<Guid> m_materials;
    std::vector<Guid> m_meshes;

    std::shared_ptr<Scene> m_scene;

private:
    std::string NameGenerator(std::string_view p_name, uint32_t& p_counter);

    uint32_t m_mesh_counter = 0;
    uint32_t m_mat_counter = 0;
};

}  // namespace cave
