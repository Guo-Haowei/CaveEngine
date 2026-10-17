#pragma once
#include "cave/render/rhi/Defines.h"
#include "cave/runtime/display/DisplayService.h"

struct GLFWwindow;

namespace cave {

class GlfwDisplayService final : public DisplayService {
public:
    GlfwDisplayService();

    void FinalizeImpl() override;

    bool shouldClose() override;

    void beginFrame() override;

    void* nativeWindow() override;
    GLFWwindow* GetGlfwWindow() const { return m_window; }

    std::string_view title() override;
    void title(std::string_view title) override;

private:
    auto initializeWindow(const WindowSpecfication& spec) -> Result<void> override;

    static void windowSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window{ nullptr };
    render::Backend m_backend;
    std::string m_title;
};

}  // namespace cave
