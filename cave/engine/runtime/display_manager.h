#pragma once
#include <tuple>

#include "engine/core/base/singleton.h"
#include "engine/renderer/graphics_defines.h"
#include "engine/runtime/module.h"

namespace cave {

enum class KeyCode : uint16_t;

struct WindowSpecfication {
    std::string title;
    int width;
    int height;
    Backend backend;
    bool decorated;
    bool fullscreen;
    bool vsync;
    bool enableImgui;
};

class IDisplayManager : public Singleton<IDisplayManager>,
                        public Module,
                        public ModuleCreateRegistry<IDisplayManager> {
public:
    IDisplayManager(std::string_view p_name)
        : Module(p_name) {}

    Result<void> InitializeImpl() final;

    virtual bool ShouldClose() = 0;

    virtual std::tuple<int, int> GetWindowSize() = 0;
    virtual std::tuple<int, int> GetWindowPos() = 0;
    virtual void* GetNativeWindow() = 0;

    virtual void BeginFrame() = 0;

protected:
    virtual auto InitializeWindow(const WindowSpecfication& p_spec) -> Result<void> = 0;
    virtual void InitializeKeyMapping() = 0;

    struct {
        int x, y;
    } m_frameSize, m_windowPos;

    std::unordered_map<int, KeyCode> m_keyMapping;
};

}  // namespace cave
