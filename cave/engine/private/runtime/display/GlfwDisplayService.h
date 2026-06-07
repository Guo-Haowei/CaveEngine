#pragma once
#include "cave/rhi/Backend.h"
#include "cave/runtime/display/DisplayService.h"

struct GLFWwindow;

namespace cave {

class GlfwDisplayService final : public DisplayService {
public:
    GlfwDisplayService()
        : DisplayService("GlfwDisplayService") {}

    void FinalizeImpl() override;

    bool shouldClose() override;

    void beginFrame() override;

    void* nativeWindow() override;
    GLFWwindow* GetGlfwWindow() const { return window_; }

    std::string_view title() override;
    void title(std::string_view title) override;

private:
    auto initializeWindow(const WindowSpecfication& spec) -> Result<void> override;

    static void windowSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window_{ nullptr };
    rhi::Backend backend_;
    std::string title_;
};

}  // namespace cave
