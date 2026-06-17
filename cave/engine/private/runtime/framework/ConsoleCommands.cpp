#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

#if USING(USE_COMMAND)
static void registerRendererCommands(CommandRegistry& cmd_reg) {
    cmd_reg.Register({
        .name = "render.pool.dump",
        .help = "List textures in transient pool.",
        .usage = "render.pool.dump",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            return ctx.services.renderer().Cmd_dump(ctx, args);
        },
    });
}

static void registerIntentCommands(CommandRegistry& cmd_reg) {
    cmd_reg.Register({
        .name = "intent.dump",
        .help = "List all registered intent handlers.",
        .usage = "intent.dump",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            return ctx.services.intentDispatcher().Cmd_dump(ctx, args);
        },
    });
}

static void registerSceneCommands(CommandRegistry& cmd_reg) {
    cmd_reg.Register({
        .name = "scene.reg.dump",
        .help = "List registered scenes.",
        .usage = "scene.reg.dump",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            return ctx.services.sceneRegistry().Cmd_dump(ctx, args);
        },
    });
}

void registerCommands(CommandRegistry& cmd_reg) {
    registerRendererCommands(cmd_reg);
    registerIntentCommands(cmd_reg);
    registerSceneCommands(cmd_reg);
}
#endif

}  // namespace cave
