#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"

#include "engine/private/runtime/assets/AssetImporter.h"

namespace cave {

struct MaterialAsset;
class MeshAsset;
class SceneAsset;

class SceneImporter : public AssetImporter {
public:
    SceneImporter(const std::filesystem::path& source_path,
                  const std::filesystem::path& dest_dir);

protected:
    Result<void> PrepareImport();

    Result<Guid> RegisterImage(const std::filesystem::path& sys_path, bool srgb);

    Result<Guid> RegisterMaterial(std::string&& name,
                                  Ref<MaterialAsset>&& material);

    Result<Guid> RegisterMesh(std::string&& name,
                              Ref<MeshAsset>&& mesh);

    Result<void> RegisterScene(ecs::Entity root);

    std::string GenerateMeshName() { return nameGenerator("mesh", m_mesh_counter); }
    std::string GenerateMaterialName() { return nameGenerator("mat", m_material_counter); }
    std::string GenerateAnimationName() { return nameGenerator("anim", m_anim_counter); }

    std::string m_file_name;
    std::string m_file_path;
    std::string m_base_path;

    Vector<Guid> m_materials;
    Vector<Guid> m_meshes;

    Ref<SceneAsset> m_scene_asset;

private:
    std::string nameGenerator(std::string_view name, uint32_t& counter);

    uint32_t m_mesh_counter = 0;
    uint32_t m_material_counter = 0;
    uint32_t m_anim_counter = 0;
};

}  // namespace cave
