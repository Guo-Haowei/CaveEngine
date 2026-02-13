#pragma once
#include "cave/core/NonCopyable.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/time/FrameTime.h"
#include "cave/game/GameModuleHandle.h"

namespace cave {

class Scene;
class IApplication;
class IGameModule;

struct PIEStartDesc {
    std::string game_dll;
    std::string game_id;
    SceneId edit_scene;
};

class PIESession : public NonCopyable {
public:
    explicit PIESession(IApplication& p_app);

    bool IsRunning() const { return m_running; }

    bool Start(const PIEStartDesc& p_desc);
    void Stop();

    void OnSimBegin();
    void OnSimEnd();

    void Tick(const FrameTime& p_time);

private:
    bool EnsureGameModuleLoaded();
    void BuildPIESceneFromEdit(Scene& p_edit, Scene& p_pie);

private:
    IApplication& m_app;

    bool m_running = false;

    PIEStartDesc m_desc{};

    GameModuleHandle m_game_handle;
    IGameModule* m_game = nullptr;

    SceneId m_pie_scene_id{};
};

}  // namespace cave
