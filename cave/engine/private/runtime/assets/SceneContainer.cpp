#include "SceneContainer.h"

// @TODO: refactor
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"
#include "engine/private/runtime/scene/SceneSerializer.h"

namespace cave {

SceneContainer::SceneContainer()
    : m_scene(MakeOwner<Scene>()) {
}

SceneContainer::~SceneContainer() = default;

Vector<Guid> SceneContainer::dependencies() const {
    DEV_ASSERT(m_scene);

    Set<Guid> deps;
    for (const auto& [id, material] : m_scene->view<MaterialComponent>()) {
        deps.insert(material.m_material_id);
    }
    for (const auto& [id, mesh_renderer] : m_scene->view<MeshRendererComponent>()) {
        deps.insert(mesh_renderer.meshGuid());
    }
    for (const auto& [id, prefab] : m_scene->view<PrefabInstanceComponent>()) {
        deps.insert(prefab.prefabGuid());
    }
    for (const auto& [id, tile_map_layer] : m_scene->view<TileMapLayerComponent>()) {
        deps.insert(tile_map_layer.tileSetGuid());
    }
    for (const auto& [id, animator] : m_scene->view<SpriteAnimatorComponent>()) {
        deps.insert(animator.animGuid());
    }

    std::erase_if(deps, [](Guid guid) {
        // @HACK: replace the last two digits to see if guid is 0
        uint8_t* data = const_cast<uint8_t*>(guid.data());
        data[15] = 0;
        return guid.isNull();
    });

    return Vector<Guid>(deps.begin(), deps.end());
}

auto SceneContainer::saveToDisk(const AssetMetaData& meta) const -> Result<void> {
    auto res = meta.saveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    SerializeScene(yaml, *m_scene, AssetRegistry::singletonPtr());
    return SaveYaml(meta.import_path, yaml);
}

auto SceneContainer::loadFromDisk(const AssetMetaData& meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer yaml;
    yaml.initialize(root);
    DeserializeScene(yaml, *m_scene);

    return Result<void>();
}

}  // namespace cave
