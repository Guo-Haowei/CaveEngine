#include "display_manager.h"
#include "graphics_manager_interface.h"
#include "physics_manager.h"
#include "scene_manager_interface.h"

namespace my {

DisplayManager* CreateDisplayManager();

IGraphicsManager* CreateGraphicsManager();

IPhysicsManager* CreatePhysicsManager();

ISceneManager* CreateSceneManager();

}  // namespace my
