#pragma once
#include "cave/core/hash/Hash.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ui/IUIRuntime.h"

#include "UILayoutResolver.h"

// @TODO: fix
#include "cave/ui/UIDrawCommand.h"

namespace cave {

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
    UIRuntime(ViewManager& view_manager) noexcept
        : m_view_manager(view_manager) {}

    void beginFrame() override;
    void endFrame(const UIInput& ui_input) override;

    UIFrameDrawData takeDrawData() override {
        return std::move(m_draw_data);
    }

    void resolve(const Scene& scene, SceneId scene_id) override;

    const ResolvedUICanvas* findResolved(SceneId scene_id,
                                         ecs::Entity canvas_entity) const override;

    void buildDrawList(const ResolvedView& view);

    UIInteractionState& interactionState() override { return m_interaction_state; }

private:
    UIInteractionState m_interaction_state;

    ViewManager& m_view_manager;
    UIFrameDrawData m_draw_data{};

    UILayoutResolver m_resolver;
    HashMap<UICanvasKey, ResolvedUICanvas, UICanvasKeyHash> m_resolved;
};

}  // namespace cave
