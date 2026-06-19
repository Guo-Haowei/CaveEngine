// =============================================================================
// File: cave/runtime/tile_map/TileMapAsset.h
// =============================================================================
#pragma once
#include "TileCoord.h"
#include "TileData.h"

#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/assets/IAsset.h"

#include "cave/core/serialization/Concepts.h"

namespace cave {

class ISerializer;
class IDeserializer;

ISerializer& WriteObject(ISerializer& s, const TileData& tile_data);

bool ReadObject(IDeserializer& d, TileData& tile_data);

static_assert(Serializable<TileData>);

class TileMapAsset : public IAsset {
    CAVE_ASSET(TileMapAsset, AssetType::TileMap, 1)

    CAVE_META(TileMapAsset)

private:
    CAVE_PROP()
    std::string name_;

    CAVE_PROP(editor = Asset)
    Guid tile_set_id_;

    CAVE_PROP(editor = Toggle)
    bool visible_ = true;

    CAVE_PROP()
    TileData tiles_;

    // Non serialized
    Handle<TileSetAsset> tile_set_handle_;
    uint32_t revision_{ 1 };  // make sure revision is ahead of renderer the first frame

public:
    Option<TileId> tileAt(TileCoord coord) const;

    bool addTile(TileCoord coord, TileId id);

    bool removeTile(TileCoord coord);

    const Handle<TileSetAsset>& tileSetHandle() const { return tile_set_handle_; }

    std::string& name() { return name_; }
    const std::string& name() const { return name_; }
    void name(std::string&& name) { name_ = std::move(name); }

    const Guid& tileSetGuid() const { return tile_set_id_; }
    void tileSetGuid(const Guid& guid, bool force = false);

    const TileData& tiles() const { return tiles_; }

    uint32_t revision() const { return revision_; }
    void incRevision() { ++revision_; }

    bool visible() const { return visible_; }
    void visible(bool visible) { visible_ = visible; }

    Result<void> SaveToDisk(const AssetMetaData& meta) const override;
    Result<void> LoadFromDisk(const AssetMetaData& meta) override;

    std::vector<Guid> GetDependencies() const override;
};

}  // namespace cave
