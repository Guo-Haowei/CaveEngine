// =============================================================================
// File: engine/private/runtime/framework/ModuleRegistry.h
// =============================================================================
#pragma once

#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/DisplayService.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/framework/IScriptService.h"

namespace cave {

IAssetManager* CreateAssetManager();

DisplayService* CreateDisplayManager();

render::IRenderDevice* CreateRenderDevice(rhi::Backend p_backend);

IPhysicsManager* CreatePhysicsManager();

IScriptService* CreateScriptService();

}  // namespace cave
