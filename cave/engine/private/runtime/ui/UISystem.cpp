#pragma once
#include "UISystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneRuntime.h"
// @TODO: move to ui/
#include "cave/runtime/framework/IUIRuntime.h"

namespace cave {

UISystem::UISystem(SceneRuntime& runtime)
    : ISceneSystem(runtime)
    , m_debug_id(MakeDebugId(this)) {
}

UISystem::~UISystem() = default;

void UISystem::update(SceneTickContext&) {
    IUIRuntime& ui = m_runtime.services().UI();
    for (const auto& e : ui.events()) {
        m_runtime.messageBus().emit(e.event, e.source);
    }
}

}  // namespace cave
