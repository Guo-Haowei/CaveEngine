#pragma once

namespace cave {

struct EngineServices;
class Scene;

struct SceneContext {
    EngineServices& engine_services;
    Scene& scene;
};

}  // namespace cave
