#pragma once
#define USE_RENDERER_DEBUG IN_USE

#if USING(USE_RENDERER_DEBUG)
#include "cave/core/diagnostics/Command.h"

namespace cave::render {

class TransientPool;

struct PoolTextureInfo {
    std::string_view debug_name;
    uint32_t width, height, depth;
    uint16_t mips;
    uint64_t bytes;
};

struct PoolSnapshot {
    std::vector<PoolTextureInfo> textures;
};

void RenderPoolDump_Cmd(TransientPool& p_pool, CommandContext& p_ctx, const CommandArgs& p_args);

}  // namespace cave::render
#endif
