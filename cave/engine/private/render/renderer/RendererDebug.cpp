#include "RendererDebug.h"

#if USING(USE_RENDERER_DEBUG)
#include "cave/core/diagnostics/ILogSink.h"

#include "engine/private/render/renderer/TransientPool.h"

namespace cave::render {

void RenderPoolDump_Cmd(TransientPool& p_pool,
                        CommandContext& p_ctx,
                        const CommandArgs&) {

    const PoolSnapshot snapshot = p_pool.Snapshot();
    std::string msg;
    msg.reserve(512);
    msg.append("Trasient Pool:\n");
    for (const PoolTextureInfo& info : snapshot.textures) {
        msg.append(std::format(" -- name: {}\n", info.debug_name));
    }
    p_ctx.sink.Submit(LogLevel::LOG_LEVEL_TRACE, msg);
}

}  // namespace cave::render
#endif
