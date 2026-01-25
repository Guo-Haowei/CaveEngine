// =============================================================================
// File: engine/private/runtime/framework/ModuleRegistry.h
// =============================================================================
#pragma once

#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/framework/IGraphicsManager.h"
#include "engine/private/runtime/framework/IPhysicsManager.h"
#include "engine/private/runtime/framework/ISceneManager.h"
#include "engine/private/runtime/framework/ScriptManager.h"

namespace cave {

IAssetManager* CreateAssetManager();

IDisplayManager* CreateDisplayManager();

IGraphicsManager* CreateGraphicsManager();

IPhysicsManager* CreatePhysicsManager();

ISceneManager* CreateSceneManager();

IScriptManager* CreateScriptManager();

}  // namespace cave
