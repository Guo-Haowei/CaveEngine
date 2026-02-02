#pragma once
#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

class IRenderDevice;

GpuTextureId CreateLTC1(IRenderDevice& p_device);

GpuTextureId CreateLTC2(IRenderDevice& p_device);

}  // namespace cave::render
