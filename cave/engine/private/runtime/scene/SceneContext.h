#pragma once

namespace cave {

class EngineServices;
class Scene;

struct SceneContext {
    EngineServices& engine_services;
    Scene& scene;
};

}  // namespace cave
