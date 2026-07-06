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
#define DEBUG_PRINT(...) LOG_INFO(cave::LogChannel::Scene, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

namespace cave {

using ecs::Entity;
namespace fs = std::filesystem;

class Scene;
struct CommandArgs;
struct CommandContext;

namespace {

std::string_view ToString(SceneSource source) {
    switch (source) {
        case SceneSource::Asset:
            return "Asset";
        case SceneSource::Editor:
            return "Editor";
        case SceneSource::Runtime:
            return "Runtime";
        case SceneSource::Thumbnail:
            return "Thumbnail";
    }

    return "???";
}

}  // namespace

class SceneRegistry::Impl : protected GenIdRegistry<Scene> {
    using Base = GenIdRegistry<Scene>;

public:
    SceneId createScene(SceneDesc&& desc);

    SceneId registerScene(SceneDesc&& desc, std::unique_ptr<Scene>&& scene);
    bool replaceScene(SceneId id, std::unique_ptr<Scene>&& scene);

    SceneId cloneScene(SceneDesc&& desc, SceneId scene_id);
    SceneId cloneScene(SceneDesc&& desc, const Scene& scene);

    void destroyScene(SceneId id);

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

SceneId SceneRegistry::createScene(SceneDesc desc) {
    return m_impl->createScene(std::move(desc));
}

SceneId SceneRegistry::registerScene(SceneDesc desc, std::unique_ptr<Scene>&& scene) {
    return m_impl->registerScene(std::move(desc), std::move(scene));
}

bool SceneRegistry::replaceScene(SceneId id, std::unique_ptr<Scene>&& scene) {
    return m_impl->replaceScene(id, std::move(scene));
}

SceneId SceneRegistry::cloneScene(SceneDesc desc, SceneId scene_id) {
    return m_impl->cloneScene(std::move(desc), scene_id);
}

SceneId SceneRegistry::cloneScene(SceneDesc desc, const Scene& scene) {
    return m_impl->cloneScene(std::move(desc), scene);
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

SceneId SceneRegistry::Impl::createScene(SceneDesc&& desc) {
    ASSERT_GAME_THREAD();

    return registerScene(std::move(desc), std::make_unique<Scene>());
}

SceneId SceneRegistry::Impl::registerScene(SceneDesc&& desc, std::unique_ptr<Scene>&& scene) {
    SceneId id = Base::create(std::move(scene));
    m_slots[id.index].debug_name = std::move(desc.debug_name);

    DEBUG_PRINT("+{} {} source={}", debugName(id), id.toString(), ToString(desc.source));
    return id;
}

bool SceneRegistry::Impl::replaceScene(SceneId id, std::unique_ptr<Scene>&& scene) {
    DEBUG_PRINT("~{} {}", debugName(id), id.toString());

    return Base::replace(id, std::move(scene));
}

SceneId SceneRegistry::Impl::cloneScene(SceneDesc&& desc, const Scene& scene) {
    auto copy = std::make_unique<Scene>();
    copy->copy(scene);

    SceneId id = registerScene(std::move(desc), std::move(copy));
    return id;
}

SceneId SceneRegistry::Impl::cloneScene(SceneDesc&& desc, SceneId scene_id) {
    const Scene* scene = Base::resolve(scene_id);
    if (!scene) return {};
    return cloneScene(std::move(desc), *scene);
}

void SceneRegistry::Impl::destroyScene(SceneId id) {
    DEBUG_PRINT("-{} {}", debugName(id), id.toString());
    Base::destroy(id);
}

#if USING(USE_COMMAND)
bool SceneRegistry::Cmd_dump(CommandContext& ctx, const CommandArgs& args) {
    return m_impl->Cmd_dump(ctx, args);
}

bool SceneRegistry::Impl::Cmd_dump(CommandContext& ctx, const CommandArgs&) {
    std::string msg;
    msg.reserve(512);
    msg.append("Scene Registry:\n");
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        const auto& slot = m_slots[idx];
        if (!slot.storage) continue;

        SceneId id = { idx, slot.gen };
        msg.append(std::format(" -- name: {}, {}\n",
                               debugName(id),
                               id.toString()));
    }

    ctx.log.Info(LogChannel::Console, std::move(msg));
    return true;
}
#endif

}  // namespace cave
