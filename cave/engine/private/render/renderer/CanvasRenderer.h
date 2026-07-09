#pragma once
#include "cave/runtime/display/ICanvas.h"

namespace cave::render {

class IRenderDevice;

class CanvasRenderer {
public:
    void drawCanvas(IRenderDevice& device,
                    ICanvas& canvas,
                    ViewId view_id);
};

}  // namespace cave::render
