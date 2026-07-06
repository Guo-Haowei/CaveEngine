#include "SystemManager.h"

namespace cave {

namespace {

constexpr size_t ToIndex(SceneSystemId id) {
    return static_cast<size_t>(id);
}

bool IsValidSystemId(SceneSystemId id) {
    const size_t index = ToIndex(id);
    return id != SceneSystemId::Invalid &&
           index < ToIndex(SceneSystemId::Count);
}

}  // namespace

SystemManager::SystemManager() {
    m_lookup.fill(nullptr);
}

SystemManager::~SystemManager() = default;

void SystemManager::addImpl(std::unique_ptr<ISceneSystem>&& system) {
    DEV_ASSERT(system);

    const SceneSystemId id = system->systemId();
    DEV_ASSERT(IsValidSystemId(id));

    const size_t index = ToIndex(id);
    DEV_ASSERT(m_lookup[index] == nullptr);

    ISceneSystem* raw = system.get();

    m_systems.push_back(std::move(system));
    m_lookup[index] = raw;
}

ISceneSystem* SystemManager::get(SceneSystemId id) {
    if (!IsValidSystemId(id)) {
        return nullptr;
    }

    return m_lookup[ToIndex(id)];
}

const ISceneSystem* SystemManager::get(SceneSystemId id) const {
    if (!IsValidSystemId(id)) {
        return nullptr;
    }

    return m_lookup[ToIndex(id)];
}

void SystemManager::start(SceneContext& ctx) {
    DEV_ASSERT(!m_scene_created);

    for (auto& system : m_systems) {
        LOG_TRACE(LogChannel::Scene, "+{}", system->debugId().type);
        system->start(ctx);
    }

    m_scene_created = true;
}

void SystemManager::shutdown() {
    if (!m_scene_created) {
        return;
    }

    m_systems.clear();
    m_scene_created = false;
}

void SystemManager::update(SceneTickContext& ctx) {
    for (auto& system : m_systems) {
        if (static_cast<int>(system->domain() & ctx.domain)) {
            system->update(ctx);
        }
    }
}

}  // namespace cave