#pragma once
#include "cave/rhi/Backend.h"

#include "engine/private/runtime/framework/DisplayService.h"

struct GLFWwindow;

namespace cave {

class GlfwDisplayManager : public DisplayService {
public:
    GlfwDisplayManager()
        : DisplayService("GlfwDisplayManager") {}

    void FinalizeImpl() final;

    bool ShouldClose() final;

    std::tuple<int, int> GetWindowSize() final;
    std::tuple<int, int> GetWindowPos() final;

    void BeginFrame() final;

    void* GetNativeWindow() final;
    GLFWwindow* GetGlfwWindow() const { return m_window; }

    std::string_view GetTitle() override;
    void SetTitle(std::string_view p_title) override;

private:
    auto InitializeWindow(const WindowSpecfication& p_spec) -> Result<void> final;

    static void WindowSizeCallback(GLFWwindow* p_window, int p_width, int p_height);

    GLFWwindow* m_window{ nullptr };
    rhi::Backend m_backend;
    std::string m_title;
};

}  // namespace cave
