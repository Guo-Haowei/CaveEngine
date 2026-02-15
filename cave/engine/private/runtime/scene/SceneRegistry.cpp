#include "SceneRegistry.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogger.h"
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
    SceneId Create(std::string p_name);

    SceneId Register(std::unique_ptr<Scene> p_scene);

    SceneId Clone(SceneId p_id);

    void Destroy(SceneId p_id);

    Scene* Resolve(SceneId p_id) {
        return Base::Resolve(p_id);
    }

    const Scene* Resolve(SceneId p_id) const {
        return Base::Resolve(p_id);
    }

    bool IsAlive(SceneId p_id) const {
        return Base::IsAlive(p_id);
    }

private:
    bool Dump_Cmd(CommandContext& p_ctx, const CommandArgs& p_args);

    friend class SceneRegistry;
};

SceneRegistry::SceneRegistry()
    : Module("SceneRegistry")
    , m_impl(std::make_unique<Impl>()) {
}

auto SceneRegistry::InitializeImpl() -> Result<void> {
    CommandRegistry& reg = m_app->CommandRegistry();
    reg.Register({
        .name = "scene.reg.dump",
        .help = "List registered scenes.",
        .usage = "scene.reg.dump",
        .fn = [this](CommandContext& p_ctx, const CommandArgs& p_args) {
            return m_impl->Dump_Cmd(p_ctx, p_args);
        },
    });

    return Result<void>();
}

void SceneRegistry::FinalizeImpl() {
}

SceneId SceneRegistry::Create(std::string p_name) {
    return m_impl->Create(std::move(p_name));
}

SceneId SceneRegistry::Register(std::unique_ptr<Scene> p_scene) {
    return m_impl->Register(std::move(p_scene));
}

SceneId SceneRegistry::Clone(SceneId p_id) {
    return m_impl->Clone(p_id);
}

void SceneRegistry::Destroy(SceneId p_id) {
    m_impl->Destroy(p_id);
}

Scene* SceneRegistry::Resolve(SceneId p_id) {
    return m_impl->Resolve(p_id);
}

const Scene* SceneRegistry::Resolve(SceneId p_id) const {
    return m_impl->Resolve(p_id);
}

bool SceneRegistry::IsAlive(SceneId p_id) const {
    return m_impl->IsAlive(p_id);
}

SceneId SceneRegistry::Impl::Create(std::string p_name) {
    ASSERT_GAME_THREAD();
    return Register(std::make_unique<Scene>(std::move(p_name)));
}

SceneId SceneRegistry::Impl::Register(std::unique_ptr<Scene> p_scene) {
    SceneId id = Base::Create(std::move(p_scene));
    // @TODO: post update

    DEBUG_PRINT("SceneRegistry::Register: registered {} ({},{})", p_scene->Name());

    return id;
}

SceneId SceneRegistry::Impl::Clone(SceneId p_id) {
    const Scene* scene = Base::Resolve(p_id);
    if (!scene) return {};
    auto copy = std::make_unique<Scene>(std::string(scene->Name()));
    copy->Copy(*scene);
    return Register(std::move(copy));
}

void SceneRegistry::Impl::Destroy(SceneId p_id) {
    // @TODO: clean up scene

    DEBUG_PRINT("SceneRegistry::Destroy: destroy {} ({},{})", m_descs[p_id.index].debug_name, p_id.index, p_id.gen);
    Base::Destroy(p_id);
}

bool SceneRegistry::Impl::Dump_Cmd(CommandContext& p_ctx, const CommandArgs&) {
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

    p_ctx.logger.Print(LogLevel::LOG_LEVEL_VERBOSE, msg);
    return true;
}

}  // namespace cave
