#pragma once
#include "cave/render/rhi/GpuResources.h"

namespace cave::render {

class IRenderDevice;

GpuTextureId CreateLTC1(IRenderDevice& p_device);

GpuTextureId CreateLTC2(IRenderDevice& p_device);

}  // namespace cave::render
