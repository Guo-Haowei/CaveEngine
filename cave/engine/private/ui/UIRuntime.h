#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/framework/IUIRuntime.h"
#include "cave/ui/UIDrawCommand.h"

namespace cave {

class ViewManager;

class UIRuntime final : public IUIRuntime {
public:
    UIRuntime()
        : IUIRuntime("UIRuntime") {}

    void BeginFrame(const UIInput& p_input) override;
    void EndFrame() override;

    void BeginView(ViewId p_view_id) override;
    void EndView() override;

    bool Button(UIId p_id, UIRect p_rect) override;

    UIFrameDrawData TakeDrawData() override {
        return std::move(m_draw_data);
    }

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    ViewManager* m_view_manager{};
    UIInput m_input{};
    UIFrameDrawData m_draw_data{};
    ViewId m_current_view{};

    UIId m_hot = 0;     // hovered this frame
    UIId m_active = 0;  // pressed/captured this frame

    int m_stack = 0;
};

}  // namespace cave
