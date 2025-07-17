#pragma once
#include <variant>

#include "editor/viewer/viewer_tab.h"

#include "engine/assets/asset_handle.h"
#include "engine/input/input_event.h"
#include "engine/scene/scene.h"
#include "engine/tile_map/tile_map_asset.h"

namespace cave {

class AssetRegistry;
class TileMapAsset;
class Viewer;

class TileMapEditor : public ViewerTab {
public:
    struct CommandAddTile {
        Vector2f cursor;
        int tile;
    };

    struct CommandEraseTile {
        Vector2f cursor;
    };

    using Command = std::variant<CommandAddTile, CommandEraseTile>;

    TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    void OnCreate(const Guid& p_guid) override;
    void OnDestroy() override;

    void Update(Scene* p_scene) override;

    const Guid& GetTileMapGuid() const { return m_tile_map_guid; }

    int GetActiveLayerIndex() const { return m_current_layer_id; }

    TileMapAsset* GetActiveLayer();

    bool SetActiveLayer(int p_index);

protected:
    bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;

    void UndoableSetTile(TileMapAsset& p_layer,
                         int p_layer_id,
                         TileIndex p_index,
                         Option<TileId> p_new_tile);

    void Reset() {
        m_tile_map_guid = Guid();
        m_commands.clear();
        m_current_layer_id = -1;
        m_tile_map_handle = Handle<TileMapAsset>();
        m_undo_stack.Clear();
    }

    std::string m_title;
    AssetRegistry* m_asset_registry;

    Guid m_tile_map_guid;
    Handle<TileMapAsset> m_tile_map_handle;

    std::vector<Command> m_commands;
    int m_current_layer_id;
};

}  // namespace cave
