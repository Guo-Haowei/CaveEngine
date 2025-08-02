#pragma once

#if USING(PLATFORM_WINDOWS)
#define USE_IMPORTER_TINYGLTF IN_USE
#elif USING(PLATFORM_APPLE)
#define USE_IMPORTER_TINYGLTF IN_USE
#elif USING(PLATFORM_WASM)
#define USE_IMPORTER_TINYGLTF NOT_IN_USE
#else
#error "Platform not supported"
#endif

#if USING(USE_IMPORTER_TINYGLTF)
#include "engine/assets/asset_importer.h"
#include "engine/ecs/entity.h"

namespace tinygltf {
class Model;
struct Animation;
struct Mesh;
}  // namespace tinygltf

namespace cave {

class Scene;

class ImporterTinyGltf : public AssetImporter {
public:
    using AssetImporter::AssetImporter;

    static std::unique_ptr<AssetImporter> CreateImporter(const std::filesystem::path& p_source_path,
                                                         const std::filesystem::path& p_dest_dir) {
        return std::make_unique<ImporterTinyGltf>(p_source_path, p_dest_dir);
    }

    Result<void> Import() override;

protected:
    void ProcessNode(int p_node_index, ecs::Entity p_parent);
    void ProcessMesh(const tinygltf::Mesh& p_gltf_mesh, int p_id);
    void ProcessAnimation(const tinygltf::Animation& p_gltf_anim, int p_id);

    std::unordered_map<int, ecs::Entity> m_entityMap;
    std::shared_ptr<tinygltf::Model> m_model;
    std::shared_ptr<Scene> m_scene;
};

}  // namespace cave

#endif  // #if USING(USE_IMPORTER_TINYGLTF)
