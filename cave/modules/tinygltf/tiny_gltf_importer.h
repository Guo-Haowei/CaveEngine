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
#include "engine/assets/scene_importer.h"
#include "engine/ecs/entity.h"

namespace tinygltf {
class Model;
struct Animation;
struct Mesh;
}  // namespace tinygltf

namespace cave {

class TinyGltfImporter : public SceneImporter {
public:
    using SceneImporter::SceneImporter;

    static std::unique_ptr<AssetImporter> CreateImporter(const std::filesystem::path& p_source_path,
                                                         const std::filesystem::path& p_dest_dir) {
        return std::make_unique<TinyGltfImporter>(p_source_path, p_dest_dir);
    }

    Result<void> Import() override;

protected:
    void ProcessNode(int p_node_index, ecs::Entity p_parent);
    void ProcessMesh(const tinygltf::Mesh& p_mesh, int p_id);
    void ProcessAnimation(const tinygltf::Animation& p_anim, int p_id);

    std::unordered_map<int, ecs::Entity> m_node_map;
    std::shared_ptr<tinygltf::Model> m_model;
};

}  // namespace cave

#endif  // #if USING(USE_IMPORTER_TINYGLTF)
