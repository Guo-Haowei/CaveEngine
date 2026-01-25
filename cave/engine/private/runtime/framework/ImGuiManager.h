#pragma once
#include "engine/private/runtime/framework/Module.h"

enum ImGuiKey : int;

namespace cave {

enum class Key : uint16_t;

struct InputEvent;

class ImguiManager : public Module {
    using Callback = std::function<void()>;

public:
    ImguiManager()
        : Module("ImguiManager") {}

    auto InitializeImpl() -> Result<void> final;
    void FinalizeImpl() final;

    void Feed(std::vector<InputEvent>& p_events);

    bool WantKeyboard() const;
    bool WantMouse() const;
    bool WantTextInput() const;

    void SetDisplayCallbacks(Callback p_initialize_func,
                             Callback p_finalize_func,
                             Callback p_begin_frame_func) {
        m_displayInitializeFunc = p_initialize_func;
        m_displayFinalizeFunc = p_finalize_func;
        m_displayBeginFrameFunc = p_begin_frame_func;
    }

    void SetRenderCallbacks(Callback p_initialize_func, Callback p_finalize_func) {
        m_rendererInitializeFunc = p_initialize_func;
        m_rendererFinalizeFunc = p_finalize_func;
    }

    void BeginFrame();

private:
    static ImGuiKey ToImGuiKey(Key k);

    Callback m_displayInitializeFunc;
    Callback m_displayBeginFrameFunc;
    Callback m_displayFinalizeFunc;

    Callback m_rendererInitializeFunc;
    Callback m_rendererFinalizeFunc;

    std::string m_imguiSettingsPath;
};

}  // namespace cave
