#pragma once
#include "cave/runtime/display/DisplayService.h"

namespace cave {

class NullDisplayService : public DisplayService {
public:
    NullDisplayService()
        : DisplayService("NullDisplayService") {}

    void FinalizeImpl() override {}

    bool shouldClose() override { return true; }

    void beginFrame() override {}
    void* nativeWindow() override { return nullptr; }

    std::string_view title() override { return ""; }
    void title(std::string_view) override {}

protected:
    auto initializeWindow(const WindowSpecfication&) -> Result<void> override {
        return Result<void>();
    }
};

}  // namespace cave
