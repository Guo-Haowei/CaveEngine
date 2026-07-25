#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/tile_map/TileData.h"

#include "editor/document/DocId.h"
#include "editor/tile_map/GridPaintTool.h"

namespace cave {

struct TileEditContext {
    ecs::Entity layer_entity{};

    Vector<TileId> selected_tile;
    TileId hovered_tile = TileId::null();

    Guid tile_set_guid;
    Handle<TileSetAsset> tile_set;
    Handle<ImageAsset> image;
    GridPaintMode paint_mode = GridPaintMode::Brush;

    bool valid() const {
        return layer_entity.valid();
    }

    void clear() {
        layer_entity = ecs::Entity::null();
        selected_tile.clear();
        hovered_tile = TileId::null();
        paint_mode = GridPaintMode::Brush;
    }
};

struct SceneEditContext {
    DocId doc_id;
    SceneId scene_id;

    ecs::Entity selected_entity{};

    // SceneToolType active_tool = SceneToolType::Select;

    TileEditContext tile;

    bool valid() const {
        return doc_id.valid() && scene_id.valid();
    }
};

class SceneEditService {
public:
    void activate(SceneEditContext* context) {
        m_active = context;
    }

    void deactivate(const SceneEditContext* context) {
        if (m_active == context) {
            m_active = nullptr;
        }
    }

    SceneEditContext* current() {
        return m_active;
    }

    const SceneEditContext* current() const {
        return m_active;
    }

    bool hasActiveScene() const {
        return m_active && m_active->valid();
    }

private:
    SceneEditContext* m_active = nullptr;
};

}  // namespace cave
