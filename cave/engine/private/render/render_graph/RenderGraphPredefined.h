#pragma once

namespace cave::render {

struct RenderGraphBuilderConfig;
class RenderGraph;

auto RenderGraph2D(RenderGraphBuilderConfig& p_config) -> Result<std::shared_ptr<RenderGraph>>;

}  // namespace cave::render
