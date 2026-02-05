#include "RendererDebug.h"

#if USING(USE_RENDERER_DEBUG)
#include "cave/core/diagnostics/ILogger.h"

#include "engine/private/render/renderer/TransientPool.h"

namespace cave::render {

void RendererPoolTextures_Cmd(TransientPool& p_pool,
                              CommandContext& p_ctx,
                              const CommandArgs& p_args) {
    unused(p_args);

    const PoolSnapshot snapshot = p_pool.Snapshot();
    std::string msg;
    msg.reserve(512);
    msg.append("Trasient Pool:");
    for (const PoolTextureInfo& info : snapshot.textures) {
        msg.append(std::format("\n -- name: {}, dim: {}x{}x{}",
                               info.debug_name,
                               info.width,
                               info.height,
                               info.depth));
    }
    p_ctx.logger.Print(LogLevel::LOG_LEVEL_VERBOSE, msg);
}

}  // namespace cave::render
#endif
