#include "display_manager.h"
#include "graphics_manager_interface.h"
#include "physics_manager_interface.h"
#include "scene_manager_interface.h"
#include "script_manager.h"

namespace cave {

DisplayManager* CreateDisplayManager();

IGraphicsManager* CreateGraphicsManager();

IPhysicsManager* CreatePhysicsManager();

ISceneManager* CreateSceneManager();

IScriptManager* CreateScriptManager();

}  // namespace cave
