#pragma once

namespace cave::render {

struct RenderGraphConfig;
class CompiledGraph;

auto RenderGraph2D(RenderGraphConfig& p_config) -> Result<std::shared_ptr<CompiledGraph>>;

}  // namespace cave::render
