#include "SceneManager.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/debugger/profiler.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/core/os/threads.h"
#include "engine/private/runtime/string/StringUtils.h"
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

SceneManager::Slot::Slot()
    : gen(SceneId::kInitialGen)
    , scene(nullptr) {
    debug_name[0] = '\0';
}

auto SceneManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void SceneManager::FinalizeImpl() {
}

SceneId SceneManager::Create(const SceneDesc& p_desc) {
    return Register(std::make_unique<Scene>(), p_desc);
}

SceneId SceneManager::Register(std::unique_ptr<Scene> p_scene, const SceneDesc& p_desc) {
    SceneId id = Alloc();
    Slot& slot = m_slots[id.index];
    DEV_ASSERT(slot.scene == nullptr);
    slot.scene = std::move(p_scene);
#if USING(DEBUG_BUILD)
    StringUtils::Strcpy(slot.debug_name,
                        p_desc.debug_name.data(),
                        p_desc.debug_name.size());
#endif
    return id;
}

void SceneManager::Destroy(SceneId p_id) {
    if (!IsAlive(p_id)) {
        return;
    }
    Free(p_id);
}

Scene* SceneManager::Resolve(SceneId p_id) {
    return IsAlive(p_id) ? m_slots[p_id.index].scene.get() : nullptr;
}

const Scene* SceneManager::Resolve(SceneId p_id) const {
    return IsAlive(p_id) ? m_slots[p_id.index].scene.get() : nullptr;
}

bool SceneManager::IsAlive(SceneId p_id) const {
    ASSERT_GAME_THREAD();
    if (p_id.index >= static_cast<uint32_t>(m_slots.size())) {
        return false;
    }

    const Slot& slot = m_slots[p_id.index];
    if (slot.gen != p_id.gen) {
        return false;
    }
    return slot.scene != nullptr;
}

#if USING(DEBUG_BUILD)
const char* SceneManager::GetDebugName(SceneId p_id) const {
    if (!IsAlive(p_id)) {
        return "<invalid-scene>";
    }
    return m_slots[p_id.index].debug_name;
}
#endif

SceneId SceneManager::Alloc() {
    uint32_t index;
    if (m_free.empty()) {
        index = static_cast<uint32_t>(m_slots.size());
        m_slots.emplace_back();
    } else {
        index = m_free.back();
        m_free.pop_back();
        DEV_ASSERT(m_slots[index].scene == nullptr);
    }

    return { index, m_slots[index].gen };
}

void SceneManager::Free(const SceneId& p_id) {
    Slot& slot = m_slots[p_id.index];
    ++slot.gen;
    slot.scene.reset();
#if USING(DEBUG_BUILD)
    slot.debug_name[0] = '\0';
#endif
    m_free.push_back(p_id.index);
}

std::shared_ptr<Scene> SceneManager::GetActiveScene() const {
    return nullptr;
}

}  // namespace cave
