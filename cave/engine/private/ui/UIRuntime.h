#pragma once
#include "cave/runtime/framework/IUIService.h"
#include "cave/ui/UIDrawCommand.h"

namespace cave {

class UIRuntime final : public IUIService {
public:
    UIRuntime()
        : IUIService("UIRuntime") {}

    void BeginFrame(const UIInput& p_input) override;
    void EndFrame() override;

    bool Button(UIId p_id, UIRect p_rect) override;

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    UIInput m_input;
    UIDrawList m_draw_list;

    UIId m_hot = 0;     // hovered this frame
    UIId m_active = 0;  // pressed/captured this frame
};

}  // namespace cave
