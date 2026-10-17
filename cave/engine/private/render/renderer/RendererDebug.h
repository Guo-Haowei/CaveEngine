#pragma once
#include "cave/render/rhi/GpuResources.h"

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

    GpuTextureId texture;
};

struct PoolSnapshot {
    Vector<PoolTextureInfo> textures;
};

void RenderPoolDump_Cmd(TransientPool& pool, CommandContext& ctx, const CommandArgs& args);

}  // namespace cave::render
#endif
