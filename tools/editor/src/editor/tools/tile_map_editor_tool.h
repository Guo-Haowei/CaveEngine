#pragma once
#include <variant>

#include "tool.h"

#include "engine/assets/asset_handle.h"
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

    void DrawAssetInspector() override;

    bool Is2D() const override { return true; }

    const char* GetName() const override { return "TileMapEditor"; }

    const std::string& GetTile() const override { return m_title; }

    const Guid& GetTileMapGuid() const { return m_tile_map_guid; }

protected:
    struct Point {
        int16_t x, y;
    };
    bool CursorToTile(const Vector2f& p_in, Point& p_out) const;

    void DrawLayerOverview(TileMapAsset& p_tile_map);

    std::string m_title;
    Viewer* m_viewer;
    AssetRegistry* m_asset_registry;

    Guid m_tile_map_guid;

    Handle<ImageAsset> m_checkerboard_handle;
    Handle<TileMapAsset> m_tile_map_handle;

    std::vector<Command> m_commands;
};

}  // namespace my
