#pragma once
#include "engine/runtime/display_manager.h"

namespace cave {

class NullDisplayManager : public IDisplayManager {
public:
    NullDisplayManager()
        : IDisplayManager("NullDisplayManager") {}

    void FinalizeImpl() override {}

    bool ShouldClose() override { return true; }

    std::tuple<int, int> GetWindowSize() override {
        return std::make_tuple(0, 0);
    }
    std::tuple<int, int> GetWindowPos() override {
        return std::make_tuple(0, 0);
    }

    void BeginFrame() override {}
    void* GetNativeWindow() override { return nullptr; }

protected:
    auto InitializeWindow(const WindowSpecfication&) -> Result<void> override {
        return Result<void>();
    }
    void InitializeKeyMapping() override {};
};

}  // namespace cave
