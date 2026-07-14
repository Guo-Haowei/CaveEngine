#include "UIRuntime.h"

#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/runtime/view/ResolvedView.h"

namespace cave {

using ecs::Entity;
using math::Vec2f;

void UIRuntime::beginFrame() {
    m_resolved.clear();

    m_interaction_state.hovered = None();  // only reset hovered, active needs to survive
}

void UIRuntime::endFrame(const UIInput& ui_input) {
    if (!ui_input.submit_down) {
        m_interaction_state.active = None();
    }
}

void UIRuntime::resolve(const Scene& scene, SceneId scene_id) {
    auto count = scene.count<UICanvasComponent>();
    if (!count) {
        return;
    }

    for (auto [ent, canvas] : scene.view<UICanvasComponent>()) {
        UICanvasKey key = {
            .scene_id = scene_id,
            .canvas_entity = ent,
        };

        auto it = m_resolved.find(key);
        if (it != m_resolved.end()) {
            LOG_WARN(LogChannel::UI, "Canvas already resolved");
            break;
        }

        auto resolved = m_resolver.resolve(scene, ent, canvas.resolution);
        m_resolved[key] = std::move(resolved);
    }
}

const ResolvedUICanvas* UIRuntime::findResolved(SceneId scene_id,
                                                ecs::Entity canvas_entity) const {
    UICanvasKey key = {
        .scene_id = scene_id,
        .canvas_entity = canvas_entity,
    };

    auto it = m_resolved.find(key);
    if (it == m_resolved.end()) {
        return nullptr;
    }
    return &it->second;
}

void UIRuntime::buildDrawList(const ResolvedView& resolved_view) {
    constexpr Color kButtonNormal = Color::Hex(static_cast<ColorCode>(0x303030));
    constexpr Color kButtonHover = Color::Hex(static_cast<ColorCode>(0x505050));
    constexpr Color kButtonActive = Color::Hex(static_cast<ColorCode>(0x707070));

    m_ui_canvas.pushView(resolved_view.view_id);

    for (const auto [ent, canvas] : resolved_view.scene->view<UICanvasComponent>()) {
        const auto* resolved_canvas = findResolved(resolved_view.scene_id, ent);
        if (!resolved_canvas) continue;

        for (const auto& element : resolved_canvas->elements) {
            const UIControlId control_id{ resolved_view.scene_id, element.entity };

            Color color = kButtonNormal;
            if (control_id == m_interaction_state.active.unwrap_or(UIControlId{})) {
                color = kButtonActive;
            } else if (control_id == m_interaction_state.hovered.unwrap_or(UIControlId{})) {
                color = kButtonHover;
            }

            const auto& rect = element.rect;
            m_ui_canvas.addBox2(rect.min(), rect.max(), color);
        }
    }

    m_ui_canvas.popView();
}

}  // namespace cave
