#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include "engine/ecs/entity.h"

namespace cave {
class Scene;
}

namespace cave::lua {

#define LUA_GLOBAL_SCENE "g_scene"

void SetPreloadFunc(lua_State* L);

bool OpenMathLib(lua_State* L);

bool OpenInputLib(lua_State* L);

bool OpenDisplayLib(lua_State* L);

bool OpenEngineLib(lua_State* L);

bool OpenSceneLib(lua_State* L);

}  // namespace cave::lua
