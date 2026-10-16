#pragma once
#include "cave/runtime/display/ICanvas.h"

// clang-format off
namespace cave { class AssetRegistry; }
// clang-format on

namespace cave::render {

struct GpuResource;

class IRenderDevice;

class CanvasRenderer {
public:
    CanvasRenderer(AssetRegistry& asset_registry);

    void drawCanvas(IRenderDevice& device,
                    ICanvas& canvas,
                    ViewId view_id);

    void setScreenSpace(bool value) { m_screen_space = value; }

private:
    void ensureDefaultTexture();

    bool m_screen_space = false;

    AssetRegistry& m_asset_reg;
    Ref<GpuResource> m_default_texture{};
};

}  // namespace cave::render
