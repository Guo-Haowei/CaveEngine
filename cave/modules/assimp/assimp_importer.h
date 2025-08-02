#pragma once

#if USING(PLATFORM_WINDOWS)
#define USE_IMPORTER_ASSIMP IN_USE
#elif USING(PLATFORM_APPLE)
#define USE_IMPORTER_ASSIMP NOT_IN_USE
#elif USING(PLATFORM_WASM)
#define USE_IMPORTER_ASSIMP NOT_IN_USE
#else
#error "Platform not supported"
#endif

#if USING(USE_IMPORTER_ASSIMP)
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

    std::vector<Guid> m_materials;
    std::vector<Guid> m_meshes;

    const aiScene* m_raw_scene = nullptr;
};

}  // namespace cave

#endif  // #if USING(USE_IMPORTER_ASSIMP)
