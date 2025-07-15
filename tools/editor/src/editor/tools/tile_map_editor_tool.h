#pragma once
#include <variant>

#include "tool.h"

#include "engine/assets/asset_handle.h"
#include "engine/assets/tile_map_asset.h"
#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace my {

class AssetRegistry;
class TileMapAsset;
class Viewer;

class TileMapEditor : public ITool {
public:
    struct CommandAddTile {
        Vector2f cursor;
        int tile;
    };

    struct CommandEraseTile {
        Vector2f cursor;
    };

    using Command = std::variant<CommandAddTile, CommandEraseTile>;

    TileMapEditor(EditorLayer& p_editor, Viewer* p_viewer);

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    void OnEnter(const Guid& p_guid) override;
    void OnExit() override;

    void Update(Scene* p_scene) override;

    bool Is2D() const override { return true; }

    const char* GetName() const override { return "TileMapEditor"; }

    const std::string& GetTile() const override { return m_title; }

    const Guid& GetTileMapGuid() const { return m_tile_map_guid; }

    int GetActiveLayerIndex() const { return m_current_layer_id; }
    TileMapLayer* GetActiveLayer();

    bool SetActiveLayer(int p_index);

protected:
    bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;

    void UndoableSetTile(TileMapLayer& p_layer,
                         int p_layer_id,
                         TileIndex p_index,
                         TileData p_new_tile);

    void Reset() {
        m_tile_map_guid = Guid();
        m_commands.clear();
        m_current_layer_id = -1;
        m_tile_map_handle = Handle<TileMapAsset>();
        m_undo_stack.Clear();
    }

    std::string m_title;
    Viewer* m_viewer;
    AssetRegistry* m_asset_registry;

    Guid m_tile_map_guid;
    Handle<TileMapAsset> m_tile_map_handle;

    std::vector<Command> m_commands;
    int m_current_layer_id;
};

}  // namespace my
