#pragma once
#include "SceneOwner.h"

namespace cave {

struct EngineServices;
struct FrameTime;
class SceneRegistry;

class SceneScheduler {
public:
    SceneScheduler(EngineServices& services) noexcept
        : m_engine_services(services) {
    }

    bool add(SceneOwner* owner);
    bool remove(SceneOwner* owner);

    void flushSceneCommands();
    void tick(const FrameTime& time);

private:
    EngineServices& m_engine_services;

    std::vector<SceneOwner*> m_owners;
};

}  // namespace cave
