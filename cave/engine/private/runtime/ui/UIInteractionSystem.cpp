#include "UIInteractionSystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/ui/UIComponents.h"
// @TODO: move to ui/
#include "cave/runtime/ui/IUIRuntime.h"

#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using ecs::Entity;
using math::Vec2f;

UIInteractionSystem::UIInteractionSystem(SceneRuntime& runtime)
    : ISceneSystem(runtime)
    , m_debug_id(MakeDebugId(this)) {
}

UIInteractionSystem::~UIInteractionSystem() = default;

void UIInteractionSystem::update(SceneTickContext& ctx) {
    RuntimeServices& services = m_runtime.services();

    const ViewRecord* view = services.viewManager().resolve(m_runtime.viewId());
    DEV_ASSERT(view);

    const auto& ui_input = services.inputService().getUIInput();
    const Vec2f point_fb = view->screenToFrameBufferPixel(ui_input.cursor_os);

    IUIRuntime& ui = services.UI();
    const Scene* scene = services.sceneRegistry().resolve(ctx.scene_id);
    DEV_ASSERT(scene);

    UIInteractionState& state = ui.interactionState();

    for (auto [canvas_ent, canvas] : scene->view<UICanvasComponent>()) {
        const ResolvedUICanvas* resolved = ui.findResolved(ctx.scene_id, canvas_ent);
        DEV_ASSERT(resolved);
        if (!resolved) continue;
        for (auto it = resolved->elements.rbegin(); it != resolved->elements.rend(); ++it) {
            const ResolvedUIElement& element = *it;
            const auto button_ent = element.entity;
            const auto* button = scene->component<UIButtonComponent>(button_ent);
            if (!button) {
                continue;
            }

            if (!element.rect.contains(point_fb)) {
                continue;
            }

            state.hovered = Some(UIControlId(ctx.scene_id, button_ent));

            if (ui_input.submit_pressed) {
                state.active = state.hovered;
            }

            if (ui_input.submit_released) {
                const bool clicked =
                    state.active.is_some() &&
                    state.hovered.is_some() &&
                    state.active.unwrap_unchecked() == state.hovered.unwrap_unchecked();

                if (clicked && button->clicked_event.empty()) {
                    const StringId signal(button->clicked_event);
                    m_runtime.messageBus().emit(signal, button_ent);
                }

                state.active = None();

                break;
            }
        }
    }
}

}  // namespace cave
