#include "SceneRegistry.h"

#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/threading/Threads.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/ids/GenIdRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

#define ASSERT_GAME_THREAD()                        \
    do {                                            \
        DEV_ASSERT(::cave::thread::IsMainThread()); \
    } while (0)

#define DEBUG_SCENE_REG IN_USE
#if USING(DEBUG_SCENE_REG)
#define DEBUG_PRINT(...) LOG_INFO(__VA_ARGS__)
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

    SceneId registerScene(std::unique_ptr<Scene>&& scene);
    bool replaceScene(SceneId id, std::unique_ptr<Scene>&& scene);

    SceneId cloneScene(SceneId scene_id);
    SceneId cloneScene(const Scene& scene);

    void destroyScene(SceneId scene_id);

    Scene* resolve(SceneId scene_id) {
        return Base::resolve(scene_id);
    }

    const Scene* resolve(SceneId scene_id) const {
        return Base::resolve(scene_id);
    }

    bool isAlive(SceneId scene_id) const {
        return Base::isAlive(scene_id);
    }

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif
};

SceneRegistry::SceneRegistry()
    : m_impl(std::make_unique<Impl>()) {
}

SceneRegistry::~SceneRegistry() = default;

SceneId SceneRegistry::createScene(std::string name) {
    return m_impl->createScene(std::move(name));
}

SceneId SceneRegistry::registerScene(std::unique_ptr<Scene>&& scene) {
    return m_impl->registerScene(std::move(scene));
}

bool SceneRegistry::replaceScene(SceneId id, std::unique_ptr<Scene>&& scene) {
    return m_impl->replaceScene(id, std::move(scene));
}

SceneId SceneRegistry::cloneScene(SceneId scene_id) {
    return m_impl->cloneScene(scene_id);
}

SceneId SceneRegistry::cloneScene(const Scene& scene) {
    return m_impl->cloneScene(scene);
}

void SceneRegistry::destroyScene(SceneId scene_id) {
    m_impl->destroyScene(scene_id);
}

Scene* SceneRegistry::resolve(SceneId scene_id) {
    return m_impl->resolve(scene_id);
}

const Scene* SceneRegistry::resolve(SceneId scene_id) const {
    return m_impl->resolve(scene_id);
}

bool SceneRegistry::isAlive(SceneId scene_id) const {
    return m_impl->isAlive(scene_id);
}

SceneId SceneRegistry::Impl::createScene(std::string name) {
    ASSERT_GAME_THREAD();
    return registerScene(std::make_unique<Scene>(std::move(name)));
}

SceneId SceneRegistry::Impl::registerScene(std::unique_ptr<Scene>&& scene) {
    std::string_view sv = scene->name();
    SceneId id = Base::create(std::move(scene));

    DEBUG_PRINT("SceneRegistry::Register: registered {} {}", sv, id.toString());

    return id;
}

bool SceneRegistry::Impl::replaceScene(SceneId id, std::unique_ptr<Scene>&& scene) {
    // @TODO: post update
    DEBUG_PRINT("SceneRegistry::Register: registered {} {}", scene->name(), id.toString());

    return Base::replace(id, std::move(scene));
}

SceneId SceneRegistry::Impl::cloneScene(const Scene& scene) {
    auto copy = std::make_unique<Scene>(std::string(scene.name()));
    copy->copy(scene);
    return registerScene(std::move(copy));
}

SceneId SceneRegistry::Impl::cloneScene(SceneId scene_id) {
    const Scene* scene = Base::resolve(scene_id);
    if (!scene) return {};
    return cloneScene(*scene);
}

void SceneRegistry::Impl::destroyScene(SceneId scene_id) {
    DEBUG_PRINT("SceneRegistry::Destroy: destroy {} {}",
                // descs_[scene_id.index].debug_name,
                "xxx",
                scene_id.toString());
    Base::destroy(scene_id);
}

#if USING(USE_COMMAND)
bool SceneRegistry::Cmd_dump(CommandContext& ctx, const CommandArgs& args) {
    return m_impl->Cmd_dump(ctx, args);
}

bool SceneRegistry::Impl::Cmd_dump(CommandContext& ctx, const CommandArgs&) {
    std::string msg;
    msg.reserve(512);
    msg.append("Scene Registry:\n");
    for (int i = 0; i < slots_.size(); ++i) {
        const auto& slot = slots_[i];
        if (!slot.storage) continue;

        msg.append(std::format(" -- name: {}, id: {},{}\n",
                               slot.storage->name(),
                               i,
                               slot.gen));
    }

    ctx.log.Info(LogChannel::Console, std::move(msg));
    return true;
}
#endif

}  // namespace cave
