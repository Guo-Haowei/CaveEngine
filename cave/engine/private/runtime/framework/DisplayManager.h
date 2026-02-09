#pragma once
#include <tuple>
#include "cave/core/Singleton.h"
#include "cave/rhi/Backend.h"

#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/runtime/framework/Module.h"

namespace cave {

enum class Key : uint16_t;

struct WindowSpecfication {
    std::string title;
    int width;
    int height;
    rhi::Backend backend;
    bool decorated;
    bool fullscreen;
    bool vsync;
    bool enableImgui;
};

class DisplayService : public Singleton<DisplayService>,
                        public Module,
                        public ModuleCreateRegistry<DisplayService> {
public:
    DisplayService(std::string_view p_name)
        : Module(p_name) {}

    Result<void> InitializeImpl() final;

    virtual bool ShouldClose() = 0;

    virtual std::tuple<int, int> GetWindowSize() = 0;
    virtual std::tuple<int, int> GetWindowPos() = 0;
    virtual void* GetNativeWindow() = 0;

    virtual void BeginFrame() = 0;

    virtual std::string_view GetTitle() = 0;
    virtual void SetTitle(std::string_view p_title) = 0;

protected:
    virtual auto InitializeWindow(const WindowSpecfication& p_spec) -> Result<void> = 0;

    struct {
        int x, y;
    } m_frameSize, m_windowPos;
};

}  // namespace cave
