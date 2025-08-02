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
#include "engine/assets/importer_interface.h"
#include "engine/scene/scene.h"

struct aiMesh;
struct aiNode;
struct aiMaterial;
struct aiScene;
struct aiAnimation;

namespace cave {

class ImporterAssimp : public IImporter {
public:
    using IImporter::IImporter;

    static std::unique_ptr<IImporter> CreateImporter(const std::filesystem::path& p_source_path,
                                                     const std::filesystem::path& p_dest_dir) {
        return std::make_unique<ImporterAssimp>(p_source_path, p_dest_dir);
    }

    Result<AssetRef> Import() override;

protected:
    void ProcessMaterial(aiMaterial& p_material);
    std::shared_ptr<MeshAsset> ProcessMesh(const aiMesh& p_mesh);

    ecs::Entity ProcessNode(const aiNode* p_node, ecs::Entity p_parent);

    std::vector<ecs::Entity> m_materials;
    std::vector<ecs::Entity> m_meshes;

    std::shared_ptr<Scene> m_scene;
};

}  // namespace cave

#endif  // #if USING(USE_IMPORTER_ASSIMP)
