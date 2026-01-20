#pragma once
#include "engine/runtime/app_state.h"

// @TODO: refactor
#include "engine/runtime/layer.h"

namespace cave {

class EditorLayer;

class EditorState final : public AppState {
public:
    EditorState(Application& p_app)
        : AppState(p_app) {}

    void OnEnter(const StateRequest& p_args) final;

    void OnExit() final;

    void Tick(float p_timestep) final;

    StateRequest PopRequest() final;

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "EditorState"; }
#endif

    GameLayer* GetGameLayer();

private:
    // @TODO: refactor layer
    void AttachLayer(Layer* p_layer);
    void DetachLayer(Layer* p_layer);
    void AttachGameLayer();
    void DetachGameLayer();

    std::unique_ptr<EditorLayer> m_editorLayer;
    std::unique_ptr<GameLayer> m_game_layer;
    std::vector<Layer*> m_layers;
};

}  // namespace cave
