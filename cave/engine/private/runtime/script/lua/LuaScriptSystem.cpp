#include "LuaScriptSystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/script/lua/LuaScriptComponent.h"

#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/script/lua/LuaBridgeInclude.h"
#include "engine/private/runtime/script/lua/LuaScriptBinding.h"

extern const char* g_lua_always_load;

namespace cave {

namespace {

int PushArg(lua_State*) {
    return 0;
}

template<typename T>
    requires std::is_integral_v<T>
int PushArg(lua_State* L, const T& value) {
    lua_pushinteger(L, value);
    return 1;
}

template<typename T>
    requires std::is_floating_point_v<T>
int PushArg(lua_State* L, const T& value) {
    lua_pushnumber(L, value);
    return 1;
}

template<typename T, typename... Args>
int PushArg(lua_State* L, T&& value, Args&&... args) {
    PushArg<T>(L, value);
    PushArg(L, std::forward<Args>(args)...);
    return 1 + sizeof...(args);
}

template<typename... Args>
static void EntityCall(lua_State* L, int ref, const char* method, Args&&... args) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    lua_getfield(L, -1, method);
    if (lua_isfunction(L, -1)) {
        // push self
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        int arg_count = PushArg(L, std::forward<Args>(args)...);
        const int result = lua_pcall(L, 1 + arg_count, 0, 0);
        if (result != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            LOG_ERROR("script error: {}", err);
        }
        lua_pop(L, 1);  // pop the return value
    } else {
        DEV_ASSERT(0);
        lua_pop(L, 1);
    }
}

template<typename... Args>
int CreateInstance(const ObjectFunctions& meta, lua_State* L, Args&&... args) {
    if (!meta.funcNew) {
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, meta.funcNew);
    const int arg_count = PushArg(L, std::forward<Args>(args)...);
    if (lua_pcall(L, arg_count, 1, 0) != LUA_OK) {
        LOG_ERROR("failed to create new instance, error: {}", lua_tostring(L, -1));
        return 0;
    }

    return luaL_ref(L, LUA_REGISTRYINDEX);
}

}  // namespace

LuaScriptSystem::LuaScriptSystem()
    : m_debug_id(MakeDebugId(this)) {
}

void LuaScriptSystem::start(SceneContext& ctx) {
    Scene& scene = ctx.scene;

    m_state = nullptr;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua::SetPreloadFunc(L);
    lua::OpenMathLib(L);
    lua::OpenSceneLib(L);
    lua::OpenInputLib(L);
    lua::OpenDisplayLib(L);
    lua::OpenLogLib(L);

    if (luaL_dostring(L, g_lua_always_load) != LUA_OK) {
        LOG_ERROR("failed to execute script, error: {}", lua_tostring(L, -1));
        lua_close(L);
        return;
    }

    if (auto res = luabridge::push(L, &scene); !res) {
        LOG_ERROR("failed to push scene, error: {}", res.message());
        lua_close(L);
        return;
    }
    lua_setglobal(L, LUA_GLOBAL_SCENE);

    for (auto [entity, script] : scene.view<LuaScriptComponent>()) {
        if (script.m_source_id.isNull()) {
            continue;
        }

        const auto& meta = findOrAdd(ctx, L, script.m_source_id, script.m_class_name.c_str());
        if (script.m_instance == 0) {
            const auto instance = CreateInstance(meta, L, entity.id());
            script.m_instance = instance;
        }
    }

    m_state = L;
    return;
}

void LuaScriptSystem::clear() {
    m_meta_lookup.clear();

    if (m_state) {
        lua_close(m_state);
        m_state = nullptr;
    }
}

void LuaScriptSystem::update(SceneTickContext& ctx) {
    CAVE_PROFILE_EVENT();

    Scene& scene = ctx.scene_ctx.scene;
    lua_State* L = m_state;

    if (DEV_VERIFY(L)) {
        const lua_Number timestep = ctx.dt;

        for (auto [entity, script] : scene.view<LuaScriptComponent>()) {
            if (script.m_source_id.isNull()) {
                continue;
            }

            if (DEV_VERIFY(script.m_instance)) {
                EntityCall(L, script.m_instance, "_process", timestep);
            }
        }
    }
}

Result<void> LuaScriptSystem::loadMetaTable(SceneContext& ctx,
                                            lua_State* L,
                                            const Guid& guid,
                                            const char* class_name,
                                            ObjectFunctions& meta) {
    auto& asset_reg = ctx.services.assetRegistry();
    auto _handle = asset_reg.findByGuid<BlobAsset>(guid);
    if (_handle.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_NOT_FOUND, "asset '{}' not found", guid.toString());
    }

    const BlobAsset* blob = _handle.unwrap_unchecked().get();
    if (!blob) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_NOT_FOUND, "asset '{}' not loaded", guid.toString());
    }

    if (luaL_dostring(L, blob->c_str()) != LUA_OK) {
        LOG_ERROR("failed to execute script '{}', error: '{}'", blob->c_str(), lua_tostring(L, -1));
        return CAVE_ERROR(ErrorCode::ERR_SCRIPT_FAILED);
    }

    // check if function exists
    lua_getglobal(L, class_name);
    if (!lua_istable(L, -1)) {
        CRASH_NOW();
    }

    lua_getfield(L, -1, "new");
    auto ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL) {
        CRASH_NOW();
    }

    meta.funcNew = ref;
    return Result<void>();
}

ObjectFunctions LuaScriptSystem::findOrAdd(SceneContext& ctx,
                                           lua_State* L,
                                           const Guid& guid,
                                           const char* class_name) {
    auto it = m_meta_lookup.find(guid);
    if (it != m_meta_lookup.end()) {
        return it->second;
    }

    ObjectFunctions meta;
    if (auto res = loadMetaTable(ctx, L, guid, class_name, meta); !res) {
        LOG_ERROR("{}", ToString(res.error()));
    } else {
        m_meta_lookup[guid] = meta;
    }

    return meta;
}

}  // namespace cave
