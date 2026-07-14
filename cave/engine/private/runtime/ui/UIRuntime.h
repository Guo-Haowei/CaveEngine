#pragma once
#include "cave/core/hash/Hash.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ui/IUIRuntime.h"

#include "UILayoutResolver.h"

namespace cave {

class ICanvas;
class ViewManager;
struct ResolvedView;

struct UICanvasKey {
    SceneId scene_id;
    ecs::Entity canvas_entity;

    bool operator==(const UICanvasKey&) const = default;
};

struct UICanvasKeyHash {
    size_t operator()(const UICanvasKey& key) const noexcept {
        size_t hash = std::hash<cave::SceneId>{}(key.scene_id);
        cave::Hash::add(hash, key.canvas_entity.id());
        return hash;
    }
};

class UIRuntime final : public IUIRuntime {
public:
    UIRuntime(ICanvas& ui_canvas, ViewManager& view_manager) noexcept
        : m_ui_canvas(ui_canvas)
        , m_view_manager(view_manager) {}

    void beginFrame() override;
    void endFrame(const UIInput& ui_input) override;

    void resolve(const Scene& scene, SceneId scene_id) override;

    const ResolvedUICanvas* findResolved(SceneId scene_id,
                                         ecs::Entity canvas_entity) const override;

    void paint(const ResolvedView& view);

    UIInteractionState& interactionState() override { return m_interaction_state; }

private:
    ICanvas& m_ui_canvas;
    ViewManager& m_view_manager;

    UILayoutResolver m_resolver;
    HashMap<UICanvasKey, ResolvedUICanvas, UICanvasKeyHash> m_resolved;

    UIInteractionState m_interaction_state;
};

}  // namespace cave
