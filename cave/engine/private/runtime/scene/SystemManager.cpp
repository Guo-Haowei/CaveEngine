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
    lookup_.fill(nullptr);
}

SystemManager::~SystemManager() = default;

void SystemManager::addImpl(std::unique_ptr<ISceneSystem>&& system) {
    DEV_ASSERT(system);

    const SceneSystemId id = system->systemId();
    DEV_ASSERT(IsValidSystemId(id));

    const size_t index = ToIndex(id);
    DEV_ASSERT(lookup_[index] == nullptr);

    ISceneSystem* raw = system.get();

    systems_.push_back(std::move(system));
    lookup_[index] = raw;
}

ISceneSystem* SystemManager::get(SceneSystemId id) {
    if (!IsValidSystemId(id)) {
        return nullptr;
    }

    return lookup_[ToIndex(id)];
}

const ISceneSystem* SystemManager::get(SceneSystemId id) const {
    if (!IsValidSystemId(id)) {
        return nullptr;
    }

    return lookup_[ToIndex(id)];
}

void SystemManager::onSceneCreate(SceneContext& ctx) {
    DEV_ASSERT(!scene_created_);

    for (auto& system : systems_) {
        LOG_INFO(LogChannel::Core, "+{}", system->debugId().type);
        system->attach(ctx);
    }

    scene_created_ = true;
}

void SystemManager::onSceneDestroy() {
    if (!scene_created_) {
        return;
    }

    for (auto it = systems_.rbegin(); it != systems_.rend(); ++it) {
        (*it)->detach();
    }

    scene_created_ = false;
}

void SystemManager::fixedUpdate(float dt) {
    for (auto& system : systems_) {
        system->fixedUpdate(dt);
    }
}

void SystemManager::update(float dt) {
    for (auto& system : systems_) {
        system->update(dt);
    }
}

void SystemManager::lateUpdate(float dt) {
    for (auto& system : systems_) {
        system->lateUpdate(dt);
    }
}

}  // namespace cave