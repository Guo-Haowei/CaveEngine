#include "scene_manager.h"

#include <imgui/imgui.h>

#include "engine/core/debugger/profiler.h"
#include "engine/core/os/timer.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/common_dvars.h"
#include "engine/scene/scene.h"

namespace cave {

using ecs::Entity;
namespace fs = std::filesystem;

Scene* SceneManager::CreateDefaultScene() {
    return nullptr;
}

auto SceneManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void SceneManager::FinalizeImpl() {}

bool SceneManager::TrySwapScene() {
#if 0
    auto queued_scene = m_loadingQueue.pop_all();

    if (queued_scene.empty()) {
        return false;
    }

    while (!queued_scene.empty()) {
        auto task = queued_scene.front();
        queued_scene.pop();
        DEV_ASSERT(task.scene);
        if (m_active_scene && !task.replace) {
            m_active_scene->Merge(*task.scene);
            m_active_scene->Update(0.0f);
        } else {
            m_active_scene = task.scene;
        }
        ++m_revision;
    }
#endif

    return true;
}

void SceneManager::Update() {
    CAVE_PROFILE_EVENT();

    TrySwapScene();

    if (m_lastRevision < m_revision) {
    }
}

std::shared_ptr<Scene> SceneManager::GetActiveScene() const {
    return nullptr;
}

}  // namespace cave
