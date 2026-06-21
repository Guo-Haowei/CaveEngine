#pragma once
#include "cave/runtime/display/DisplayService.h"

#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

IAssetManager* CreateAssetService();

DisplayService* CreateDisplayService();

render::IRenderDevice* CreateRenderDevice(rhi::Backend p_backend);

}  // namespace cave
