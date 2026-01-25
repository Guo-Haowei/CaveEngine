// =============================================================================
// File: engine/runtime/framework/ModuleRegistry.h
// =============================================================================
#pragma once

#include "engine/runtime/framework/IAssetManager.h"
#include "engine/runtime/framework/DisplayManager.h"
#include "engine/runtime/framework/IGraphicsManager.h"
#include "engine/runtime/framework/IPhysicsManager.h"
#include "engine/runtime/framework/ISceneManager.h"
#include "engine/runtime/framework/ScriptManager.h"

namespace cave {

IAssetManager* CreateAssetManager();

IDisplayManager* CreateDisplayManager();

IGraphicsManager* CreateGraphicsManager();

IPhysicsManager* CreatePhysicsManager();

ISceneManager* CreateSceneManager();

IScriptManager* CreateScriptManager();

}  // namespace cave
