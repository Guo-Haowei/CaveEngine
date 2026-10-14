#pragma once
#include "cave/render/rhi/Defines.h"
#include "cave/runtime/framework/IService.h"

enum ImGuiKey : int;

struct ImDrawList;
struct ImVec2;

namespace cave {

enum class Key : uint16_t;

struct InputEvent;

class ImGuiService : public IService {
    using Callback = std::function<void()>;

public:
    ImGuiService(render::Backend backend)
        : IService("ImGuiService")
        , m_backend(backend) {}

    void feed(std::vector<InputEvent>& events);

    bool wantKeyboard() const;
    bool wantMouse() const;
    bool wantTextInput() const;

    void setDisplayCallbacks(Callback initialize_func,
                             Callback finalize_func,
                             Callback begin_frame_func) {
        m_displayInitializeFunc = initialize_func;
        m_displayFinalizeFunc = finalize_func;
        m_displayBeginFrameFunc = begin_frame_func;
    }

    void setRenderCallbacks(Callback initialize_func, Callback finalize_func) {
        m_rendererInitializeFunc = initialize_func;
        m_rendererFinalizeFunc = finalize_func;
    }

    void beginFrame();

    void image(uint64_t tex, const ImVec2& size) const;

    void drawTexture(ImDrawList& list,
                     uint64_t tex,
                     const ImVec2& min,
                     const ImVec2& max) const;

protected:
    auto InitializeImpl() -> Result<void> final;
    void FinalizeImpl() final;

private:
    static ImGuiKey toImGuiKey(Key k);

    const render::Backend m_backend;

    Callback m_displayInitializeFunc;
    Callback m_displayBeginFrameFunc;
    Callback m_displayFinalizeFunc;

    Callback m_rendererInitializeFunc;
    Callback m_rendererFinalizeFunc;

    std::string m_imguiSettingsPath;
};

}  // namespace cave
