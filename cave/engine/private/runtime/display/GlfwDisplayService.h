#pragma once
#include "cave/rhi/Backend.h"
#include "cave/runtime/display/DisplayService.h"

struct GLFWwindow;

namespace cave {

class GlfwDisplayManager : public DisplayService {
public:
    GlfwDisplayManager()
        : DisplayService("GlfwDisplayManager") {}

    void FinalizeImpl() final;

    bool shouldClose() final;

    void beginFrame() final;

    void* nativeWindow() final;
    GLFWwindow* GetGlfwWindow() const { return m_window; }

    std::string_view title() override;
    void title(std::string_view p_title) override;

private:
    auto initializeWindow(const WindowSpecfication& p_spec) -> Result<void> final;

    static void WindowSizeCallback(GLFWwindow* p_window, int p_width, int p_height);

    GLFWwindow* m_window{ nullptr };
    rhi::Backend m_backend;
    std::string m_title;
};

}  // namespace cave
