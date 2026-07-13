#pragma once
#include "UISystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/ui/UIComponents.h"
// @TODO: move to ui/
#include "cave/runtime/framework/IUIRuntime.h"

#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using math::Vec2f;
using ecs::Entity;

UISystem::UISystem(SceneRuntime& runtime)
    : ISceneSystem(runtime)
    , m_debug_id(MakeDebugId(this)) {
}

UISystem::~UISystem() = default;

void UISystem::update(SceneTickContext& ctx) {
    RuntimeServices& services = m_runtime.services();

    const ViewRecord* view = services.viewManager().resolve(m_runtime.viewId());
    DEV_ASSERT(view);

    const auto& ui_input = services.inputService().getUIInput();
    const Vec2f point_fb = view->screenToFrameBufferPixel(ui_input.cursor_os);

    IUIRuntime& ui = services.UI();
    const Scene* scene = services.sceneRegistry().resolve(ctx.scene_id);
    DEV_ASSERT(scene);

    for (auto [ent, canvas] : scene->view<UICanvasComponent>()) {
        const ResolvedUICanvas* resolved = ui.findResolved(ctx.scene_id, ent);
        DEV_ASSERT(resolved);
        if (!resolved) continue;
        for (auto it = resolved->elements.rbegin(); it != resolved->elements.rend(); ++it) {
            const ResolvedUIElement& element = *it;

            if (!element.rect.contains(point_fb)) {
                continue;
            }

            // maybe buffer then execute?
            element.hovered = true;
            if (ui_input.submit_pressed) {
                element.active = true;

                if (element.clicked_event.is_some()) {
                    m_runtime.messageBus().emit(
                        element.clicked_event.unwrap_unchecked(),
                        element.entity);
                }
            }
        }
    }
}

}  // namespace cave
