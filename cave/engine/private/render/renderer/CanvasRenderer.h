#pragma once
#include "cave/runtime/display/ICanvas.h"

// clang-format off
namespace cave { class AssetRegistry; }
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

class IRenderDevice;

class CanvasRenderer {
public:
    CanvasRenderer(AssetRegistry& asset_registry);

    void drawCanvas(IRenderDevice& device,
                    ICanvas& canvas,
                    ViewId view_id);

private:
    void ensureDefaultTexture();

    AssetRegistry& m_asset_reg;
    Ref<GpuTexture> m_default_texture{};
};

}  // namespace cave::render
