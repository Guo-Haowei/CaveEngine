#include "SceneRegistry.h"

#include "cave/core/diagnostics/ILogSink.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/ids/GenIdRegistry.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/runtime/scene/Scene.h"

#define ASSERT_GAME_THREAD()                        \
    do {                                            \
        DEV_ASSERT(::cave::thread::IsMainThread()); \
    } while (0)

#define DEBUG_SCENE_REG NOT_IN_USE
#if USING(DEBUG_SCENE_REG)
#define DEBUG_PRINT(...) LOG_VERBOSE(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

namespace cave {

using ecs::Entity;
namespace fs = std::filesystem;

class Scene;
struct CommandArgs;
struct CommandContext;

class SceneRegistry::Impl : protected GenIdRegistry<Scene> {
    using Base = GenIdRegistry<Scene>;

public:
    SceneId createScene(std::string name);

    SceneId registerScene(std::unique_ptr<Scene> scene);

    SceneId cloneScene(SceneId scene_id);

    void destroyScene(SceneId scene_id);

    Scene* resolve(SceneId scene_id) {
        return Base::Resolve(scene_id);
    }

    const Scene* resolve(SceneId scene_id) const {
        return Base::Resolve(scene_id);
    }

    bool isAlive(SceneId scene_id) const {
        return Base::IsAlive(scene_id);
    }

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif
};

SceneRegistry::SceneRegistry()
    : impl_(std::make_unique<Impl>()) {
}

SceneRegistry::~SceneRegistry() = default;

SceneId SceneRegistry::createScene(std::string name) {
    return impl_->createScene(std::move(name));
}

SceneId SceneRegistry::registerScene(std::unique_ptr<Scene> scene) {
    return impl_->registerScene(std::move(scene));
}

SceneId SceneRegistry::cloneScene(SceneId scene_id) {
    return impl_->cloneScene(scene_id);
}

void SceneRegistry::destroyScene(SceneId scene_id) {
    impl_->destroyScene(scene_id);
}

Scene* SceneRegistry::resolve(SceneId scene_id) {
    return impl_->resolve(scene_id);
}

const Scene* SceneRegistry::resolve(SceneId scene_id) const {
    return impl_->resolve(scene_id);
}

bool SceneRegistry::isAlive(SceneId scene_id) const {
    return impl_->isAlive(scene_id);
}

SceneId SceneRegistry::Impl::createScene(std::string name) {
    ASSERT_GAME_THREAD();
    return registerScene(std::make_unique<Scene>(std::move(name)));
}

SceneId SceneRegistry::Impl::registerScene(std::unique_ptr<Scene> scene) {
    SceneId id = Base::Create(std::move(scene));
    // @TODO: post update

    DEBUG_PRINT("SceneRegistry::Register: registered {} ({},{})", scene->Name());

    return id;
}

SceneId SceneRegistry::Impl::cloneScene(SceneId scene_id) {
    const Scene* scene = Base::Resolve(scene_id);
    if (!scene) return {};
    auto copy = std::make_unique<Scene>(std::string(scene->Name()));
    copy->Copy(*scene);
    return registerScene(std::move(copy));
}

void SceneRegistry::Impl::destroyScene(SceneId scene_id) {
    // @TODO: clean up scene

    DEBUG_PRINT("SceneRegistry::Destroy: destroy {} ({},{})", m_descs[scene_id.index].debug_name, p_id.index, p_id.gen);
    Base::Destroy(scene_id);
}

#if USING(USE_COMMAND)
bool SceneRegistry::Cmd_dump(CommandContext& ctx, const CommandArgs& args) {
    return impl_->Cmd_dump(ctx, args);
}

bool SceneRegistry::Impl::Cmd_dump(CommandContext& ctx, const CommandArgs&) {
    std::string msg;
    msg.reserve(512);
    msg.append("Scene Registry:\n");
    for (int i = 0; i < m_slots.size(); ++i) {
        const auto& slot = m_slots[i];
        if (!slot.storage) continue;

        msg.append(std::format(" -- name: {}, id: {},{}\n",
                               slot.storage->Name(),
                               i,
                               slot.gen));
    }

    ctx.log.Info(LogChannel::Console, std::move(msg));
    return true;
}
#endif

}  // namespace cave
