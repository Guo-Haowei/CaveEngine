// =============================================================================
// File: cave/runtime/display/DisplayService.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"
#include "cave/core/base/Singleton.h"
#include "cave/rhi/Backend.h"
#include "cave/runtime/framework/IService.h"

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
                       public IService,
                       public ServiceCreateRegistry<DisplayService> {
public:
    DisplayService(std::string_view name)
        : IService(name) {}

    Result<void> InitializeImpl() final;

    virtual bool shouldClose() = 0;

    virtual void* nativeWindow() = 0;

    virtual void beginFrame() = 0;

    virtual std::string_view title() = 0;
    virtual void title(std::string_view title) = 0;

    math::Vector2i windowSize() const { return frame_size_; }
    math::Vector2f windowPos() const { return window_pos_; }

protected:
    virtual auto initializeWindow(const WindowSpecfication& spec) -> Result<void> = 0;

    math::Vector2i frame_size_;
    math::Vector2f window_pos_;
};

}  // namespace cave
