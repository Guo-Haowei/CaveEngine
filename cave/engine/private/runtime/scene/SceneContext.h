#pragma once

namespace cave {

struct EngineServices;
class Scene;
class NativeScriptRegistry;

struct SceneContext {
    EngineServices& engine_services;
    NativeScriptRegistry& native_scripts;
    Scene& scene;
};

}  // namespace cave
