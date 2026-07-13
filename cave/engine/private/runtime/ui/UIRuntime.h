#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/framework/IUIRuntime.h"

#include "UILayoutResolver.h"

// @TODO: fix
#include "cave/ui/UIDrawCommand.h"

namespace cave {

class ViewManager;

class UIRuntime final : public IUIRuntime {
public:
    UIRuntime(ViewManager& view_manager) noexcept
        : m_view_manager(view_manager) {}

    void beginFrame(const UIInput& input) override;
    void endFrame() override;

    UIFrameDrawData takeDrawData() override {
        return std::move(m_draw_data);
    }

    void buildCanvas(const Scene& scene, ViewId view_id) override;

private:
    void buildDrawList(const ResolvedUITree& ui_tree, ViewId view_id);

    ViewManager& m_view_manager;
    UIInput m_ui_input{};
    UIFrameDrawData m_draw_data{};

    ecs::Entity m_hot;     // hovered this frame
    ecs::Entity m_active;  // pressed/captured this frame

    UILayoutResolver m_resolver;
};

}  // namespace cave
