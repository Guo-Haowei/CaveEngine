#include "SceneManager.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/debugger/profiler.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/core/os/threads.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/string/StringUtils.h"

namespace cave {

using ecs::Entity;
namespace fs = std::filesystem;

#define ASSERT_GAME_THREAD()                        \
    do {                                            \
        DEV_ASSERT(::cave::thread::IsMainThread()); \
    } while (0)

SceneManager::SceneManager()
    : Module("SceneManager") {
}

SceneManager::~SceneManager() = default;

auto SceneManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void SceneManager::FinalizeImpl() {
}

SceneId SceneManager::Create() {
    return {};
}

SceneId SceneManager::Clone(SceneId p_id) {
    if (!IsAlive(p_id)) {
        return {};
    }

    const Scene& source = *(m_slots[p_id.index].scene);
    auto copy = std::make_unique<Scene>();
    copy->Copy(source);

    return Register(std::move(copy));
}

SceneId SceneManager::Register(std::unique_ptr<Scene> p_scene) {
    unused(p_scene);
    return {};
}

void SceneManager::Destroy(SceneId p_id) {
    unused(p_id);
}

Scene* SceneManager::Resolve(SceneId ) {
    return nullptr;
}

const Scene* SceneManager::Resolve(SceneId ) const {
    return nullptr;
}

bool SceneManager::IsAlive(SceneId p_id) const {
    unused(p_id);
    return false;
}

SceneId SceneManager::Alloc() {
    return {};
}

void SceneManager::Free(SceneId p_id) {
    unused(p_id);
}

}  // namespace cave
