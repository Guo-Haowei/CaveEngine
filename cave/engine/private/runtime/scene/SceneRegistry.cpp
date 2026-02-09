#include "SceneRegistry.h"

#include "engine/private/core/os/threads.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;
namespace fs = std::filesystem;

#define ASSERT_GAME_THREAD()                        \
    do {                                            \
        DEV_ASSERT(::cave::thread::IsMainThread()); \
    } while (0)

SceneRegistry::SceneRegistry()
    : ISceneRegistry("SceneRegistry") {
}

// @TODO: register scene commands
auto SceneRegistry::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void SceneRegistry::FinalizeImpl() {
}

SceneId SceneRegistry::Create() {
    SceneId id = Base::Create(std::make_unique<Scene>());
    // @TODO: post update
    return id;
}

SceneId SceneRegistry::Clone(SceneId p_id) {
    const Scene* scene = Base::Resolve(p_id);
    if (!scene) return {};
    auto copy = std::make_unique<Scene>();
    copy->Copy(*scene);
    return Register(std::move(copy));
}

SceneId SceneRegistry::Register(std::unique_ptr<Scene> p_scene) {
    SceneId id = Base::Create(std::move(p_scene));
    // @TODO: post update
    return id;
}

void SceneRegistry::Destroy(SceneId p_id) {
    // @TODO: pre clean up
    Base::Destroy(p_id);
}

}  // namespace cave
