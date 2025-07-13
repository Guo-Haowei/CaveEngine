#pragma once
#include "tool.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace my {

class Viewer;

struct TileMapEditCommand {
    enum {
        INSERT,
        ERASE,
    } type;

    union {
        Vector2f cursor;
        struct {

        } dummy;
    };
};

class TileMapEditor : public ITool {
public:
    TileMapEditor(EditorLayer& p_editor, Viewer* p_viewer)
        : ITool(p_editor), m_viewer(p_viewer) {
        m_policy = ToolCameraPolicy::Only2D;
    }

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    void OnEnter(const Guid& p_guid) override;
    void OnExit() override;

    void Update(Scene* p_scene) override;

    virtual bool Is2D() const { return true; }

    const char* GetName() const override { return "TileMapEditor"; }

    const std::string& GetTile() const override { return m_title; }

protected:
    Viewer* m_viewer;

    Guid m_tile_map_guid;
    std::vector<TileMapEditCommand> m_commands;
    std::string m_title;
};

}  // namespace my
