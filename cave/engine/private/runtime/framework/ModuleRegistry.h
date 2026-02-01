// =============================================================================
// File: engine/private/runtime/framework/ModuleRegistry.h
// =============================================================================
#pragma once

#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
#include "engine/private/runtime/framework/IScriptManager.h"

namespace cave {

IAssetManager* CreateAssetManager();

IDisplayManager* CreateDisplayManager();

render::IRenderDevice* CreateRenderDevice();

IPhysicsManager* CreatePhysicsManager();

IScriptManager* CreateScriptManager();

}  // namespace cave
