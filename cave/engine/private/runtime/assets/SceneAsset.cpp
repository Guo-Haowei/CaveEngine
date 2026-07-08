#include "SceneAsset.h"

// @TODO: refactor
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneSerializer.h"

namespace cave {

SceneAsset::SceneAsset()
    : m_scene(std::make_unique<Scene>()) {
}

SceneAsset::~SceneAsset() = default;

void SceneAsset::initializeDefault() {
    if (DEV_VERIFY(m_scene)) {
        auto root = m_scene->createEntity();
        m_scene->create(TransformComponent_Id, root);
        m_scene->create<NameComponent>(root).setName("root");

        auto ent = m_scene->createEntity();
        m_scene->create(TransformComponent_Id, ent);
        m_scene->create<NameComponent>(ent).setName("untitled");

        m_scene->setRoot(root);
        m_scene->attachChild(ent);
    }
}

std::vector<Guid> SceneAsset::dependencies() const {
    DEV_ASSERT(m_scene);

    std::vector<Guid> dependencies;
    for (const auto& [id, material] : m_scene->view<MaterialComponent>()) {
        dependencies.push_back(material.m_material_id);
    }
    for (const auto& [id, mesh_renderer] : m_scene->view<MeshRendererComponent>()) {
        dependencies.push_back(mesh_renderer.GetResourceGuid());
    }
    for (const auto& [id, prefab] : m_scene->view<PrefabInstanceComponent>()) {
        dependencies.push_back(prefab.prefabGuid());
    }
    for (const auto& [id, tile_map_renderer] : m_scene->view<TileMapInstanceComponent>()) {
        dependencies.push_back(tile_map_renderer.GetResourceGuid());
    }
    for (const auto& [id, animator] : m_scene->view<SpriteAnimatorComponent>()) {
        dependencies.push_back(animator.GetResourceGuid());
    }

    dependencies.erase(
        std::remove_if(dependencies.begin(), dependencies.end(),
                       [](Guid guid) {
                           // @HACK: replace the last two digits to see if guid is 0
                           uint8_t* data = const_cast<uint8_t*>(guid.data());
                           data[15] = 0;
                           return guid.isNull();
                       }),
        dependencies.end());

    return dependencies;
}

auto SceneAsset::saveToDisk(const AssetMetaData& meta) const -> Result<void> {
    auto res = meta.saveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    SerializeScene(yaml, *m_scene, AssetRegistry::singletonPtr(), true);
    return SaveYaml(meta.import_path, yaml);
}

auto SceneAsset::loadFromDisk(const AssetMetaData& meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer yaml;
    yaml.Initialize(root);
    DeserializeScene(yaml, *m_scene);
    return Result<void>();
}

}  // namespace cave
