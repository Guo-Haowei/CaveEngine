#include "luaScriptBinding.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/display/DisplayService.h"

#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/script/lua/LuaBridgeInclude.h"

// @TODO: refactor
#include "engine/private/core/math/Geomath.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave::lua {

using namespace cave::math;

// @TODO: refactor
struct Quat {
    Quat(const Vector3f& p_euler) {
        value = Quaternion(glm::vec3(p_euler.x, p_euler.y, p_euler.z));
    }

    Quaternion value;
};

static int CustomSearcher(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    auto asset = AssetRegistry::GetSingleton().FindByPath<BlobAsset>(std::format("{}", path));
    if (asset.is_none()) {
        return 0;
    }

    Handle<BlobAsset> handle = asset.unwrap_unchecked();
    if (const BlobAsset* blob = handle.Get()) {
        if (luaL_loadbuffer(L, blob->GetBufferPointer(), blob->GetBufferLength(), path) == LUA_OK) {
            return 1;
        }

        const char* error_message = lua_tostring(L, -1);
        LOG_ERROR("{}", error_message);
        return 1;
    }

    return 0;
}

void SetPreloadFunc(lua_State* L) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers");
    lua_pushcfunction(L, CustomSearcher);
    lua_rawseti(L, -2, 1);
}

bool OpenMathLib(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<Vector2f>("Vector2")
        .addConstructor<void (*)(float, float)>()
        .addProperty("x", &Vector2f::x)
        .addProperty("y", &Vector2f::y)
        .addFunction("__add", [](const Vector2f& p_lhs, const Vector2f& p_rhs) {
            return p_lhs + p_rhs;
        })
        .addFunction("__sub", [](const Vector2f& p_lhs, const Vector2f& p_rhs) {
            return p_lhs - p_rhs;
        })
        .addFunction("__mul", [](const Vector2f& p_lhs, const Vector2f& p_rhs) {
            return p_lhs * p_rhs;
        })
        .addFunction("__div", [](const Vector2f& p_lhs, const Vector2f& p_rhs) {
            return p_lhs / p_rhs;
        })
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<Vector3f>("Vector3")
        .addConstructor<void (*)(float, float, float)>()
        .addProperty("x", &Vector3f::x)
        .addProperty("y", &Vector3f::y)
        .addProperty("z", &Vector3f::z)
        .addFunction("__add", [](const Vector3f& p_lhs, const Vector3f& p_rhs) {
            return p_lhs + p_rhs;
        })
        .addFunction("__sub", [](const Vector3f& p_lhs, const Vector3f& p_rhs) {
            return p_lhs - p_rhs;
        })
        .addFunction("__mul", [](const Vector3f& p_lhs, const Vector3f& p_rhs) {
            return p_lhs * p_rhs;
        })
        .addFunction("__div", [](const Vector3f& p_lhs, const Vector3f& p_rhs) {
            return p_lhs / p_rhs;
        })
        .addFunction("normalize", [](Vector3f& p_self) {
            p_self = normalize(p_self);
        })
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<Quat>("Quaternion")
        .addConstructor<void (*)(const Vector3f)>()
        .endClass();
    return true;
}

bool OpenInputLib(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Input")
        .addFunction("is_action_pressed", [](const char* p_action) -> int {
            unused(p_action);
            LOG_WARN(LogChannel::Input, "TODO");
            // return InputService::GetSingleton().IsActionPressed(StringId(p_str_id));
            return 0;
        })
        .addFunction("is_action_just_pressed", [](const char* p_action) -> int {
            unused(p_action);
            LOG_WARN(LogChannel::Input, "TODO");
            // return InputService::GetSingleton().IsActionJustPressed(StringId(p_str_id));
            return 0;
        })
        .addFunction("is_action_just_released", [](const char* p_action) -> int {
            unused(p_action);
            LOG_WARN(LogChannel::Input, "TODO");
            // return InputService::GetSingleton().IsActionJustReleased(StringId(p_str_id));
            return 0;
        })
        .addFunction("get_action_strength", [](const char* p_action) -> float {
            unused(p_action);
            LOG_WARN(LogChannel::Input, "TODO");
            return 0;
            // return InputService::GetSingleton().GetActionStrength(StringId(p_str_id));
        })
        .endNamespace();
    return true;
}

bool OpenDisplayLib(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Display")
        //.addFunction("GetWindowSize", []() -> Vector2f {
        //    auto [width, height] = DisplayManager::GetSingleton().GetWindowSize();
        //    return Vector2f(width, height);
        //})
        .endNamespace();
    return true;
}

bool OpenLogLib(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("logger")
        .addFunction("trace", [](const char* p_message) {
            LogImpl(LOG_LEVEL_TRACE, LogChannel::Lua, "-- {}", p_message);
        })
        .addFunction("info", [](const char* p_message) {
            LogImpl(LOG_LEVEL_INFO, LogChannel::Lua, "-- {}", p_message);
        })
        .addFunction("ok", [](const char* p_message) {
            LogImpl(LOG_LEVEL_OK, LogChannel::Lua, "-- {}", p_message);
        })
        .addFunction("warn", [](const char* p_message) {
            LogImpl(LOG_LEVEL_WARN, LogChannel::Lua, "-- {}", p_message);
        })
        .addFunction("error", [](const char* p_file, int p_line, const char* p_error) {
            ReportErrorImpl("lua_function", p_file, p_line, p_error);
            GENERATE_TRAP();
        })
        .endNamespace();
    return true;
}

[[maybe_unused]] static int lua_GetAllLuaScripts(lua_State* L) {
    Scene* scene = luabridge::getGlobal(L, LUA_GLOBAL_SCENE);
    auto view = scene->View<LuaScriptComponent>();
    int i = 0;

    lua_newtable(L);
    for (auto [id, script] : view) {
        lua_pushinteger(L, ++i);
        lua_pushinteger(L, id.GetId());
        lua_settable(L, -3);
    }

    return 1;
}

bool OpenSceneLib(lua_State* L) {
    // TransformComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<TransformComponent>("TransformComponent")
        .addFunction("translate", [](TransformComponent& p_transform, const Vector3f& p_translation) {
            p_transform.Translate(p_translation);
        })
        .addFunction("get_translation", [](TransformComponent& p_transform) -> Vector3f {
            return p_transform.GetTranslation();
        })
        .addFunction("set_translation", [](TransformComponent& p_transform, const Vector3f& p_translation) {
            p_transform.SetTranslation(p_translation);
        })
        .addFunction("get_world_translation", [](const TransformComponent& p_transform) {
            glm::vec3 v = p_transform.GetWorldMatrix()[3];
            return Vector3f(v.x, v.y, v.z);
        })
        .addFunction("rotate", &TransformComponent::Rotate)
        .addFunction("set_rotation", [](TransformComponent& p_transform, const Quat& p_quat) {
            Vector4f rotation(p_quat.value.x, p_quat.value.y, p_quat.value.z, p_quat.value.w);
            p_transform.SetRotation(rotation);
        })
        .addFunction("get_scale", [](const TransformComponent& p_transform) -> Vector3f {
            return p_transform.GetScale();
        })
        .addFunction("set_scale", &TransformComponent::SetScale)
        .endClass();

    // Animator
    luabridge::getGlobalNamespace(L)
        .beginClass<SpriteAnimatorComponent>("SpriteAnimatorComponent")
        .addFunction("set_clip", &SpriteAnimatorComponent::SetClip)
        .endClass();

    // CameraComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<CameraComponent>("CameraComponent")
        .addFunction("get_fovy", [](CameraComponent* p_camera) -> float {
            return p_camera->GetFovy();
        })
        .addFunction("set_fovy", [](CameraComponent* p_camera, float p_degree) {
            p_camera->SetFovy(p_degree);
        })
        .endClass();

    // Velocity
    luabridge::getGlobalNamespace(L)
        .beginClass<VelocityComponent>("VelocityComponent")
        .addProperty("linear", &VelocityComponent::linear)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<MeshRendererComponent>("MeshRendererComponent")
        .addFunction("is_visible", [](MeshRendererComponent* p_mesh_renderer) {
            return p_mesh_renderer->IsVisible();
        })
        .addFunction("set_visible", [](MeshRendererComponent* p_mesh_renderer, bool p_visible) {
            p_mesh_renderer->SetVisible(p_visible);
        })
        .addFunction("cast_shadow", [](MeshRendererComponent* p_mesh_renderer) {
            return p_mesh_renderer->CastShadow();
        })
        .addFunction("set_cast_shadow", [](MeshRendererComponent* p_mesh_renderer, bool p_cast_shadow) {
            p_mesh_renderer->SetCastShadow(p_cast_shadow);
        })
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<Scene>("Scene")
        .addFunction("get_name", [](Scene* p_scene, uint32_t p_ent) {
            auto ret = p_scene->GetComponent<NameComponent>(ecs::Entity(p_ent));
            return ret->GetName();
        })
        .addFunction("get_transform", [](Scene* p_scene, uint32_t p_ent) {
            return p_scene->GetComponent<TransformComponent>(ecs::Entity(p_ent));
        })
        .addFunction("get_animator", [](Scene* p_scene, uint32_t p_ent) {
            return p_scene->GetComponent<SpriteAnimatorComponent>(ecs::Entity(p_ent));
        })
        .addFunction("get_velocity", [](Scene* p_scene, uint32_t p_ent) {
            return p_scene->GetComponent<VelocityComponent>(ecs::Entity(p_ent));
        })
        .addFunction("get_mesh_renderer", [](Scene* p_scene, uint32_t p_ent) {
            return p_scene->GetComponent<MeshRendererComponent>(ecs::Entity(p_ent));
        })
        .addFunction("get_camera", [](Scene* p_scene, uint32_t p_ent) {
            return p_scene->GetComponent<CameraComponent>(ecs::Entity(p_ent));
        })
        .addFunction("find_entity_by_name", [](Scene* p_scene, const char* p_name) {
            return p_scene->FindEntityByName(p_name).GetId();
        })
        //.addFunction("GetMeshEmitter", [](Scene* p_scene, uint32_t p_ent) {
        //    return p_scene->GetComponent<MeshEmitterComponent>(ecs::Entity(p_ent));
        //})
        //.addFunction("GetScript", [](Scene* p_scene, uint32_t p_ent) {
        //    return p_scene->GetComponent<LuaScriptComponent>(ecs::Entity(p_ent));
        //})
        //.addFunction("GetAllLuaScripts", lua_GetAllLuaScripts)
        .endClass();
    return true;
}

}  // namespace cave::lua
