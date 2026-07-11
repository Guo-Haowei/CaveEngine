#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/intent/IntentBus.h"

#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/core/variant/DvarParser.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

#if USING(USE_COMMAND)
class RegisterCommands {
public:
    static void dvar(CommandRegistry& cmd_reg);
    static void render(CommandRegistry& cmd_reg);
    static void intent(CommandRegistry& cmd_reg);
    static void scene(CommandRegistry& cmd_reg);
    static void asset(CommandRegistry& cmd_reg);
};

void RegisterCommands::dvar(CommandRegistry& cmd_reg) {
    cmd_reg.registerCmd({
        .name = "dvar.set",
        .help = "Set dvar value.",
        .usage = "Usage: dvar.set name [value]",
        .fn = [](CommandContext& ctx, const CommandArgs& args_) {
            if (args_.tokens.empty()) {
                ctx.log.Error(LogChannel::Console, std::string(ctx.desc.usage));
                return false;
            }
            std::span<const std::string_view> args = args_.tokens.subspan(1);
            DvarParser parser(args, DvarParser::Source::Console);

            std::string err;
            if (parser.parseSetCmd(err)) return true;

            ctx.log.Error(LogChannel::Console, std::move(err));
            return false;
        },
    });
}

void RegisterCommands::render(CommandRegistry& cmd_reg) {
    cmd_reg.registerCmd({
        .name = "render.pool.dump",
        .help = "List textures in transient pool.",
        .usage = "render.pool.dump",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            return ctx.services.renderer().Cmd_dump(ctx, args);
        },
    });
}

void RegisterCommands::intent(CommandRegistry& cmd_reg) {
    cmd_reg.registerCmd({
        .name = "intent.dump",
        .help = "List all registered intent handlers.",
        .usage = "intent.dump",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            return ctx.services.intentBus().Cmd_dump(ctx, args);
        },
    });
}

void RegisterCommands::scene(CommandRegistry& cmd_reg) {
    cmd_reg.registerCmd({
        .name = "scene.reg.dump",
        .help = "List registered scenes.",
        .usage = "scene.reg.dump",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            return ctx.services.sceneRegistry().Cmd_dump(ctx, args);
        },
    });
}

void RegisterCommands::asset(CommandRegistry& cmd_reg) {
    cmd_reg.registerCmd({
        .name = "asset.users",
        .help = "Dump assets that depend on this asset.",
        .usage = "Usage: asset.users <asset_path_or_guid>",
        .fn = [](CommandContext& ctx, const CommandArgs& args) {
            if (args.tokens.size() != 2) {
                ctx.log.Error(LogChannel::Console, std::string(ctx.desc.usage));
                return false;
            }

            auto& asset_reg = ctx.services.assetRegistry();

            std::string_view asset_str = args.tokens[1];
            Option<AssetHandle> handle;
            if (auto guid = Guid::parse(asset_str)) {
                handle = asset_reg.findByGuid(guid.unwrap_unchecked());
            } else {
                handle = asset_reg.findByPath(std::string(asset_str));
            }

            if (handle.is_none()) {
                ctx.log.Error(LogChannel::Console, std::format("Failed to find '{}'", asset_str));
                return false;
            }

            Guid guid = handle.unwrap_unchecked().guid();
            auto users = asset_reg.findReverseDependenciesTransitively(guid);
            auto msg = std::format("\nTransitive users of {}:", handle.unwrap_unchecked().meta()->name);

            if (users.empty()) {
                msg.append("  <none>");
            } else {
                for (const Guid& user : users) {
                    msg.append("\n  -> ");
                    auto entry = asset_reg.entry(user);
                    msg.append(entry ? entry->metadata.name : "???");
                }
            }

            msg.append(std::format("\nTotal: {} dependency asset(s).", users.size()));
            ctx.log.Info(LogChannel::Console, std::move(msg));
            return true;
        },
    });
}

void RegisterCommands(CommandRegistry& cmd_reg) {
    RegisterCommands::asset(cmd_reg);
    RegisterCommands::dvar(cmd_reg);
    RegisterCommands::render(cmd_reg);
    RegisterCommands::scene(cmd_reg);
    RegisterCommands::intent(cmd_reg);
}
#endif

}  // namespace cave
