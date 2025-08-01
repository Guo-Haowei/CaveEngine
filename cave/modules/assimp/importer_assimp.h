#pragma once
#ifdef _NO_ASSIMP
#define USING_ASSIMP NOT_IN_USE
#else
#define USING_ASSIMP USE_IF(USING(PLATFORM_WINDOWS))
#endif

#if USING(USING_ASSIMP)
#include "engine/assets/asset_loader.h"
#include "engine/scene/scene.h"

struct aiMesh;
struct aiNode;
struct aiMaterial;
struct aiScene;
struct aiAnimation;

namespace cave {

class ImporterAssimp : public IAssetLoader {
public:
    using IAssetLoader::IAssetLoader;

    static std::unique_ptr<IAssetLoader> CreateLoader(const std::string& p_import_path) {
        return std::make_unique<ImporterAssimp>(p_import_path);
    }

    Result<AssetRef> Load() override;

protected:
    void ProcessMaterial(aiMaterial& p_material);
    std::shared_ptr<MeshAsset> ProcessMesh(const aiMesh& p_mesh);

    ecs::Entity ProcessNode(const aiNode* p_node, ecs::Entity p_parent);

    std::vector<ecs::Entity> m_materials;
    std::vector<ecs::Entity> m_meshes;

    Scene* m_scene;
};

}  // namespace cave
#endif
