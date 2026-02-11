#include "SceneRegistry.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogger.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/os/threads.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;
namespace fs = std::filesystem;

#define ASSERT_GAME_THREAD()                        \
    do {                                            \
        DEV_ASSERT(::cave::thread::IsMainThread()); \
    } while (0)

#define DEBUG_SCENE_REG IN_USE
#if USING(DEBUG_SCENE_REG)
#define DEBUG_PRINT(...) LOG_VERBOSE(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

SceneRegistry::SceneRegistry()
    : ISceneRegistry("SceneRegistry") {
}

// @TODO: register scene commands
auto SceneRegistry::InitializeImpl() -> Result<void> {
    CommandRegistry& reg = m_app->CommandRegistry();
    reg.Register({
        .name = "scene.reg.dump",
        .help = "List registered scenes.",
        .usage = "scene.reg.dump",
        .fn = [this](CommandContext& p_ctx, const CommandArgs& p_args) {
            return Dump_Cmd(p_ctx, p_args);
        },
    });

    return Result<void>();
}

void SceneRegistry::FinalizeImpl() {
}

SceneId SceneRegistry::Create(SceneDesc p_desc) {
    ASSERT_GAME_THREAD();
    return Register(std::move(p_desc), std::make_unique<Scene>());
}

SceneId SceneRegistry::Register(SceneDesc p_desc, std::unique_ptr<Scene> p_scene) {
    SceneId id = Base::Create(std::move(p_scene));
    // @TODO: post update

    DEBUG_PRINT("SceneRegistry::Register: registered {} ({},{})", p_desc.debug_name, id.index, id.gen);

    if (m_descs.size() <= id.index) {
        m_descs.resize(id.index + 1);
        m_descs[id.index] = std::move(p_desc);
    }

    return id;
}

SceneId SceneRegistry::Clone(SceneDesc p_desc, SceneId p_id) {
    const Scene* scene = Base::Resolve(p_id);
    if (!scene) return {};
    auto copy = std::make_unique<Scene>();
    copy->Copy(*scene);
    return Register(std::move(p_desc), std::move(copy));
}

void SceneRegistry::Destroy(SceneId p_id) {
    // @TODO: clean up scene

    DEBUG_PRINT("SceneRegistry::Destroy: destroy {} ({},{})", m_descs[p_id.index].debug_name, p_id.index, p_id.gen);
    Base::Destroy(p_id);
}

bool SceneRegistry::Dump_Cmd(CommandContext& p_ctx, const CommandArgs&) {
    DEV_ASSERT(m_slots.size() == m_descs.size());

    std::string msg;
    msg.reserve(512);
    msg.append("Scene Registry:");
    for (int i = 0; i < m_slots.size(); ++i) {
        const auto& slot = m_slots[i];
        if (!slot.storage) continue;
        const SceneDesc& desc = m_descs[i];

        msg.append(std::format("\n -- name: {}, id: {},{}",
                               desc.debug_name,
                               i,
                               slot.gen));
    }

    p_ctx.logger.Print(LogLevel::LOG_LEVEL_VERBOSE, msg);
    return true;
}

}  // namespace cave
