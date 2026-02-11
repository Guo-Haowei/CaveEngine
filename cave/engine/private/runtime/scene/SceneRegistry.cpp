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

SceneRegistry::SceneRegistry()
    : ISceneRegistry("SceneRegistry") {
}

bool SceneRegistry::Dump_Cmd(CommandContext& p_ctx, const CommandArgs&) {
    std::string msg;
    msg.reserve(512);

    msg.append("Scene Registry:");
    for (const auto& slot : m_slots) {
        std::string_view name = "Untitled";
        msg.append(std::format("\n -- name: {}, id: {}", name, slot.gen));
    }

    p_ctx.logger.Print(LogLevel::LOG_LEVEL_VERBOSE, msg);
    return true;
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
