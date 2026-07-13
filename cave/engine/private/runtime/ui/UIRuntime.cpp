#include "UIRuntime.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using ecs::Entity;
using math::Vec2f;

constexpr Color kButtonNormal = Color::Hex(static_cast<ColorCode>(0x303030));
constexpr Color kButtonHover = Color::Hex(static_cast<ColorCode>(0x505050));
constexpr Color kButtonActive = Color::Hex(static_cast<ColorCode>(0x707070));

void UIRuntime::beginFrame(const UIInput& input) {
    m_ui_input = input;
    m_draw_data.clear();
    m_events.clear();
    m_hot = Entity{};
}

void UIRuntime::endFrame() {
    if (!m_ui_input.submit_down) {
        m_active = Entity{};
    }
}

void UIRuntime::buildCanvas(const Scene& scene,
                            SceneId scene_id,
                            ViewId view_id) {
    auto count = scene.count<UICanvasComponent>();
    if (!count) {
        return;
    }

    int counter = 0;
    for (auto [ent, canvas] : scene.view<UICanvasComponent>()) {
        if (counter > 1) {
            LOG_WARN(LogChannel::UI, "Only support one canvas per scene");
            break;
        }
        auto ui_tree = m_resolver.resolve(scene, ent, canvas.resolution);
        buildDrawList(ui_tree, scene, scene_id, view_id);

        m_ui_trees[scene_id] = std::move(ui_tree);

        ++counter;
    }
}

void UIRuntime::buildDrawList(const ResolvedUITree& ui_tree,
                              const Scene& scene,
                              SceneId scene_id,
                              ViewId view_id) {
    const ViewRecord* view = m_view_manager.resolve(view_id);
    DEV_ASSERT(view);

    const Vec2f point_fb = view->screenToFrameBufferPixel(m_ui_input.cursor_os);

    for (const ResolvedUIElement& element : ui_tree.elements) {
        auto min = element.rect.min();
        auto size = element.rect.size();

        OldUIRect rect{ min.x, min.y, size.x, size.y };

        Entity uiid = element.entity;

        const bool hovered = rect.Contains(point_fb.x, point_fb.y);
        if (hovered) {
            m_hot = uiid;
        }

        if (hovered && m_ui_input.submit_pressed) {
            m_active = uiid;

            auto* button = scene.component<UIButtonComponent>(element.entity);
            if (button && button->interactable && !button->clicked_event.empty()) {
                m_events.emplace_back(UIButtonClicked{
                    scene_id,
                    StringId(button->clicked_event),
                    element.entity,
                });
            }
        }

        bool clicked = false;

        if (m_active == uiid && m_ui_input.submit_released) {
            clicked = hovered;
        }

        Color color = kButtonNormal;
        if (m_active == uiid) {
            color = kButtonActive;
        } else if (m_hot == uiid) {
            color = kButtonHover;
        }

        m_draw_data.draw_lists[view_id].addRect(rect, color);
    }
}

}  // namespace cave
