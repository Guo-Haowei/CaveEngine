#include "lua_binding.h"

#include "engine/runtime/asset_registry.h"
#include "engine/runtime/display_manager.h"
#include "engine/runtime/input_manager.h"
#include "engine/math/vector.h"
#include "engine/scene/scene.h"
#include "lua_bridge_include.h"

namespace cave::lua {

// @TODO: refactor
struct Quat {
    Quat(const Vector3f& p_euler) {
        value = Quaternion(glm::vec3(p_euler.x, p_euler.y, p_euler.z));
    }

    Quaternion value;
};

static int CustomSearcher(lua_State* L) {
    unused(L);
    DEV_ASSERT(0);
#if 0
        const char* path = luaL_checkstring(L, 1);
        auto asset = dynamic_cast<const TextAsset*>(
            AssetRegistry::GetSingleton()
                .Request(std::format("{}", path)));
        if (!asset) {
            return 0;
        }

        const auto& source = asset->source;
        if (luaL_loadbuffer(L, source.data(), source.length(), path) == LUA_OK) {
            return 1;
        }

        const char* error_message = lua_tostring(L, -1);
        LOG_ERROR("{}", error_message);

        auto error = std::format("error loading '{}'", path);
        lua_pushstring(L, error.c_str());
#endif
    return 1;
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
    // @TODO: route the input
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Input")
        .addFunction("get_mouse_move", []() {
            return InputManager::GetSingleton().MouseMove();
        })
        .addFunction("get_cursor", []() -> Vector2f {
            return InputManager::GetSingleton().GetCursor();
        })
        .addFunction("is_action_pressed", [](StringId p_name) -> int {
            return InputManager::GetSingleton().IsActionPressed(p_name);
        })
        .addFunction("is_action_just_pressed", [](StringId p_name) -> int {
            return InputManager::GetSingleton().IsActionJustPressed(p_name);
        })
        .addFunction("is_action_just_released", [](StringId p_name) -> int {
            return InputManager::GetSingleton().IsActionJustReleased(p_name);
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

bool OpenEngineLib(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Engine")
        .addFunction("log", [](const char* p_message) {
            LogImpl(LOG_LEVEL_NORMAL, "{}", p_message);
        })
        .addFunction("log_ok", [](const char* p_message) {
            LogImpl(LOG_LEVEL_OK, "{}", p_message);
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
        .beginClass<AnimatorComponent>("AnimatorComponent")
        .addFunction("set_clip", &AnimatorComponent::SetClip)
        .endClass();

    // CameraComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<CameraComponent>("CameraComponent")
        .addFunction("get_fovy", [](CameraComponent* p_camera) -> float {
            return p_camera->GetFovy().GetDegree();
        })
        .addFunction("set_fovy", [](CameraComponent* p_camera, float p_degree) {
            p_camera->SetFovy(Degree(p_degree));
        })
        .endClass();

    // Velocity
    luabridge::getGlobalNamespace(L)
        .beginClass<VelocityComponent>("VelocityComponent")
        .addProperty("linear", &VelocityComponent::linear)
        .endClass();

#if 0
    // RigidBodyComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<RigidBodyComponent>("RigidBodyComponent")
        .addProperty("collision_type", &RigidBodyComponent::collisionType)
        .endClass();

    // LuaScriptComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<LuaScriptComponent>("LuaScriptComponent")
        .addFunction("GetClass", [](LuaScriptComponent* p_component) {
            return p_component->GetClassName();
        })
        .addFunction("GetRef", [](LuaScriptComponent* p_component) {
            return p_component->GetInstance();
        })
        .endClass();

    // MeshEmitterComponent
    luabridge::getGlobalNamespace(L)
        .beginClass<MeshEmitterComponent>("MeshEmitterComponent")
        .addFunction("Reset", [](MeshEmitterComponent* p_component) {
            return p_component->Reset();
        })
        .addFunction("Start", [](MeshEmitterComponent* p_component) {
            return p_component->Start();
        })
        .addFunction("Stop", [](MeshEmitterComponent* p_component) {
            return p_component->Stop();
        })
        .addFunction("IsRunning", [](MeshEmitterComponent* p_component) {
            return p_component->IsRunning();
        })
        .endClass();
#endif

    luabridge::getGlobalNamespace(L)
        .beginClass<Scene>("Scene")
        .addFunction("get_name", [](Scene* p_scene, uint32_t p_entity) {
            auto ret = p_scene->GetComponent<NameComponent>(ecs::Entity(p_entity));
            return ret->GetName();
        })
        .addFunction("get_transform", [](Scene* p_scene, uint32_t p_entity) {
            return p_scene->GetComponent<TransformComponent>(ecs::Entity(p_entity));
        })
        .addFunction("get_animator", [](Scene* p_scene, uint32_t p_entity) {
            return p_scene->GetComponent<AnimatorComponent>(ecs::Entity(p_entity));
        })
        .addFunction("get_velocity", [](Scene* p_scene, uint32_t p_entity) {
            return p_scene->GetComponent<VelocityComponent>(ecs::Entity(p_entity));
        })
        .addFunction("get_camera", [](Scene* p_scene, uint32_t p_entity) {
            return p_scene->GetComponent<CameraComponent>(ecs::Entity(p_entity));
        })
        .addFunction("get_rigid_body", [](Scene* p_scene, uint32_t p_entity) {
            return p_scene->GetComponent<RigidBodyComponent>(ecs::Entity(p_entity));
        })
        .addFunction("find_entity_by_name", [](Scene* p_scene, const char* p_name) {
            return p_scene->FindEntityByName(p_name).GetId();
        })
        //.addFunction("GetMeshEmitter", [](Scene* p_scene, uint32_t p_entity) {
        //    return p_scene->GetComponent<MeshEmitterComponent>(ecs::Entity(p_entity));
        //})
        //.addFunction("GetScript", [](Scene* p_scene, uint32_t p_entity) {
        //    return p_scene->GetComponent<LuaScriptComponent>(ecs::Entity(p_entity));
        //})
        //.addFunction("GetAllLuaScripts", lua_GetAllLuaScripts)
        .endClass();
    return true;
}

}  // namespace cave::lua
