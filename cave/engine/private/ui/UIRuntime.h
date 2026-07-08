#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/framework/IUIRuntime.h"
#include "cave/ui/UIDrawCommand.h"

namespace cave {

class ViewManager;

class UIRuntime final : public IUIRuntime {
public:
    UIRuntime(ViewManager& view_manager) noexcept
        : m_view_manager(view_manager) {}

    void beginFrame(const UIInput& input) override;
    void endFrame() override;

    void beginView(ViewId view_id) override;
    void endView() override;

    bool button(UIId id, UIRect rect) override;

    UIFrameDrawData takeDrawData() override {
        return std::move(m_draw_data);
    }

private:
    ViewManager& m_view_manager;
    UIInput m_ui_input{};
    UIFrameDrawData m_draw_data{};
    ViewId m_current_view{};

    UIId m_hot = 0;     // hovered this frame
    UIId m_active = 0;  // pressed/captured this frame

    int m_stack = 0;
};

}  // namespace cave
