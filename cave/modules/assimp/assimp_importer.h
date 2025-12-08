#pragma once

#include "engine/assets/scene_importer.h"
#include "engine/scene/scene.h"

struct aiMesh;
struct aiNode;
struct aiMaterial;
struct aiScene;
struct aiAnimation;

namespace cave {

class AssimpImporter : public SceneImporter {
public:
    using SceneImporter::SceneImporter;

    static std::unique_ptr<AssetImporter> CreateImporter(const std::filesystem::path& p_source_path,
                                                         const std::filesystem::path& p_dest_dir) {
        return std::make_unique<AssimpImporter>(p_source_path, p_dest_dir);
    }

    Result<void> Import() override;

protected:
    Guid ProcessMaterial(aiMaterial& p_material);
    Guid ProcessMesh(const aiMesh& p_mesh);

    ecs::Entity ProcessNode(const aiNode* p_node, ecs::Entity p_parent);

    const aiScene* m_raw_scene = nullptr;
};

}  // namespace cave
