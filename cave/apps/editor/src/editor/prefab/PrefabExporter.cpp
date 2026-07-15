#include "PrefabExporter.h"

// @TODO: refactor
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/scene/SceneSerializer.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

Result<Guid> PrefabExporter::exportPrefab(std::string_view path,
                                          const Scene& scene,
                                          ecs::Entity root) {

    auto meta_opt = AssetMetaData::createMeta(path);
    if (meta_opt.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create meta '{}'", path);
    }

    auto meta = std::move(meta_opt.unwrap_unchecked());
    if (auto res = meta.saveToDisk(nullptr); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    ExportSubtree(yaml, scene, root, AssetRegistry::singletonPtr());
    if (auto res = SaveYaml(meta.import_path, yaml); !res) {
        return CAVE_ERROR(res.error());
    }

    Guid guid = meta.guid;
    AssetRegistry::singleton().startAsyncLoad(std::move(meta));
    return guid;
}

}  // namespace cave
