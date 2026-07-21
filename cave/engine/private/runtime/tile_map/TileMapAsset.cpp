#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

void TileMapLayer::setTileSetGuid(const Guid& guid) {
    if (m_tile_set_guid == guid) {
        return;
    }

    m_tile_set_guid = guid;
    refreshTileSetHandle();
}

void TileMapLayer::refreshTileSetHandle() {
    if (auto handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(m_tile_set_guid)) {
        m_tile_set_handle = std::move(handle.unwrap());
    } else {
        m_tile_set_handle.invalidate();
    }

    // @TODO: set dirty
}

Vector<Guid> TileMapAsset::dependencies() const {
    HashSet<Guid> guids;
    for (const TileMapLayer& layer : m_layers) {
        guids.insert(layer.tileSetGuid());
    }
    return Vector<Guid>(guids.begin(), guids.end());
}

Result<void> TileMapAsset::saveToDisk(const AssetMetaData& meta) const {
    auto res = meta.saveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.beginMap(false)
        .beginKey("version")
        .write(kVersion)
        .beginKey("content")
        .write(*this)
        .endMap();
    return SaveYaml(meta.import_path, yaml);
}

Result<void> TileMapAsset::loadFromDisk(const AssetMetaData& meta) {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer d;
    d.initialize(root);

    if (d.tryEnterKey("content")) {
        if (!d.read(*this)) {
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA,
                              "failed to read content of {}",
                              meta.name);
        }
        d.leaveKey();
    }

    for (auto& layer : m_layers) {
        layer.refreshTileSetHandle();
    }
    return Result<void>();
}

}  // namespace cave
