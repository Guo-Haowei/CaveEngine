#include "lua_script_manager.h"

#include "engine/assets/blob_asset.h"
#include "engine/core/debugger/profiler.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/input_manager.h"
#include "engine/core/string/string_builder.h"
#include "engine/core/string/string_utils.h"
#include "engine/scene/scene.h"

// lua include
#include "lua_binding.h"
#include "lua_bridge_include.h"

extern const char* g_lua_always_load;

namespace cave {

auto LuaScriptManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void LuaScriptManager::FinalizeImpl() {
}

inline int PushArg(lua_State*) {
    return 0;
}

template<typename T>
    requires std::is_integral_v<T>
static int PushArg(lua_State* L, const T& p_value) {
    lua_pushinteger(L, p_value);
    return 1;
}

template<typename T>
    requires std::is_floating_point_v<T>
static int PushArg(lua_State* L, const T& p_value) {
    lua_pushnumber(L, p_value);
    return 1;
}

template<typename T, typename... Args>
static int PushArg(lua_State* L, T&& p_value, Args&&... p_args) {
    PushArg<T>(L, p_value);
    PushArg(L, std::forward<Args>(p_args)...);
    return 1 + sizeof...(p_args);
}

template<typename... Args>
static void EntityCall(lua_State* L, int p_ref, const char* p_method, Args&&... p_args) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, p_ref);
    lua_getfield(L, -1, p_method);
    if (lua_isfunction(L, -1)) {
        // push self
        lua_rawgeti(L, LUA_REGISTRYINDEX, p_ref);
        int arg_count = PushArg(L, std::forward<Args>(p_args)...);
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
static int CreateInstance(const ObjectFunctions& p_meta, lua_State* L, Args&&... p_args) {
    if (!p_meta.funcNew) {
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, p_meta.funcNew);
    const int arg_count = PushArg(L, std::forward<Args>(p_args)...);
    if (lua_pcall(L, arg_count, 1, 0) != LUA_OK) {
        LOG_ERROR("failed to create new instance, error: {}", lua_tostring(L, -1));
        return 0;
    }

    return luaL_ref(L, LUA_REGISTRYINDEX);
}

void LuaScriptManager::OnSimBegin(Scene& p_scene) {
    m_state = nullptr;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua::SetPreloadFunc(L);
    lua::OpenMathLib(L);
    lua::OpenSceneLib(L);
    lua::OpenInputLib(L);
    lua::OpenDisplayLib(L);
    lua::OpenEngineLib(L);

    if (luaL_dostring(L, g_lua_always_load) != LUA_OK) {
        LOG_ERROR("failed to execute script, error: {}", lua_tostring(L, -1));
        lua_close(L);
        return;
    }

    if (auto res = luabridge::push(L, &p_scene); !res) {
        LOG_ERROR("failed to push scene, error: {}", res.message());
        lua_close(L);
        return;
    }
    lua_setglobal(L, LUA_GLOBAL_SCENE);

    for (auto [entity, script] : p_scene.m_LuaScriptComponents) {
        if (script.m_source_id.IsNull()) {
            continue;
        }

        const auto& meta = FindOrAdd(L, script.m_source_id, script.m_class_name.c_str());
        if (script.m_instance == 0) {
            const auto instance = CreateInstance(meta, L, entity.GetId());
            script.m_instance = instance;
        }
    }

    m_state = L;
    return;
}

void LuaScriptManager::OnSimEnd() {
    m_objects_meta.clear();

    if (m_state) {
        lua_close(m_state);
        m_state = nullptr;
    }
}

void LuaScriptManager::Update(Scene& p_scene, float p_timestep) {
    CAVE_PROFILE_EVENT();

    lua_State* L = m_state;

    if (DEV_VERIFY(L)) {
        const lua_Number timestep = p_timestep;

        for (auto [entity, script] : p_scene.View<LuaScriptComponent>()) {
            if (script.m_source_id.IsNull()) {
                continue;
            }

            if (DEV_VERIFY(script.m_instance)) {
                EntityCall(L, script.m_instance, "_process", timestep);
            }
        }
    }
}

void LuaScriptManager::OnCollision(Scene& p_scene, ecs::Entity p_entity_1, ecs::Entity p_entity_2) {
    lua_State* L = m_state;
    if (DEV_VERIFY(L)) {
        LuaScriptComponent* script_1 = p_scene.GetComponent<LuaScriptComponent>(p_entity_1);
        LuaScriptComponent* script_2 = p_scene.GetComponent<LuaScriptComponent>(p_entity_2);

        if (script_1 && script_1->m_instance) {
            EntityCall(L, script_1->m_instance, "_on_collision", p_entity_2.GetId());
        }

        if (script_2 && script_2->m_instance) {
            EntityCall(L, script_2->m_instance, "_on_collision", p_entity_1.GetId());
        }
    }
}

Result<void> LuaScriptManager::LoadMetaTable(lua_State* L, const Guid& p_guid, const char* p_class_name, ObjectFunctions& p_meta) {
    auto asset_registry = m_app->GetAssetRegistry();
    auto _handle = asset_registry->FindByGuid<BlobAsset>(p_guid);
    if (_handle.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_NOT_FOUND, "asset '{}' not found", p_guid.ToString());
    }

    const BlobAsset* blob = _handle.unwrap_unchecked().Get();
    if (!blob) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_NOT_FOUND, "asset '{}' not loaded", p_guid.ToString());
    }

    if (luaL_dostring(L, blob->c_str()) != LUA_OK) {
        LOG_ERROR("failed to execute script '{}', error: '{}'", blob->c_str(), lua_tostring(L, -1));
        return CAVE_ERROR(ErrorCode::ERR_SCRIPT_FAILED);
    }

    // check if function exists
    lua_getglobal(L, p_class_name);
    if (!lua_istable(L, -1)) {
        CRASH_NOW();
    }

    lua_getfield(L, -1, "new");
    auto ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL) {
        CRASH_NOW();
    }

    p_meta.funcNew = ref;
    return Result<void>();
}

ObjectFunctions LuaScriptManager::FindOrAdd(lua_State* L, const Guid& p_guid, const char* p_class_name) {
    auto it = m_objects_meta.find(p_guid);
    if (it != m_objects_meta.end()) {
        return it->second;
    }

    ObjectFunctions meta;
    if (auto res = LoadMetaTable(L, p_guid, p_class_name, meta); !res) {
        StringStreamBuilder builder;
        builder << res.error();
        LOG_ERROR("{}", builder.ToString());
    } else {
        m_objects_meta[p_guid] = meta;
    }

    return meta;
}

}  // namespace cave
