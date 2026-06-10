#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/framework/IUIRuntime.h"
#include "cave/ui/UIDrawCommand.h"

namespace cave {

class ViewManager;

class UIRuntime final : public IUIRuntime {
public:
    UIRuntime(ViewManager& view_manager) noexcept
        : view_manager_(view_manager) {}

    void beginFrame(const UIInput& input) override;
    void endFrame() override;

    void beginView(ViewId view_id) override;
    void endView() override;

    bool button(UIId id, UIRect rect) override;

    UIFrameDrawData takeDrawData() override {
        return std::move(draw_data_);
    }

private:
    ViewManager& view_manager_;
    UIInput ui_input_{};
    UIFrameDrawData draw_data_{};
    ViewId current_view_{};

    UIId hot_ = 0;     // hovered this frame
    UIId active_ = 0;  // pressed/captured this frame

    int stack_ = 0;
};

}  // namespace cave
