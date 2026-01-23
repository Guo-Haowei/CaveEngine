#include "input_manager.h"

#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

namespace cave {

auto InputManager::InitializeImpl() -> Result<void> {
#if 0
    m_input_binding[STR_ID("ui_left")] = std::to_underlying(Key::KEY_LEFT);
    m_input_binding[STR_ID("ui_right")] = std::to_underlying(Key::KEY_RIGHT);
    m_input_binding[STR_ID("ui_up")] = std::to_underlying(Key::KEY_UP);
    m_input_binding[STR_ID("ui_down")] = std::to_underlying(Key::KEY_DOWN);
    m_input_binding[STR_ID("ui_accept")] = std::to_underlying(Key::KEY_ENTER);
    m_input_binding[STR_ID("ui_back")] = std::to_underlying(Key::KEY_BACKSPACE);
    m_input_binding[STR_ID("ui_cancel")] = std::to_underlying(Key::KEY_ESCAPE);
#endif
    return Result<void>();
}

void InputManager::FinalizeImpl() {
}

void InputManager::AddDevice(std::unique_ptr<IInputDevice> p_device) {
    DEV_ASSERT(p_device);

    LOG_VERBOSE("InputManager::AddDevice: device '{}' added", p_device->Id().value);
    m_devices.emplace_back(std::move(p_device));
}

void InputManager::UpdatePointers(std::vector<InputEvent>& p_events) {
    // reset deltas each frame
    for (auto& [_, ps] : m_pointers) {
        ps.dx = 0.0f;
        ps.dy = 0.0f;
    }

    for (InputEvent& e : p_events) {
        if (e.consumed) continue;
        if (e.type != InputEventType::MouseMove) continue;

        auto& ps = m_pointers[e.device.value];

        const float new_x = e.x;
        const float new_y = e.y;

        if (ps.has_pos) {
            ps.dx = new_x - ps.x;
            ps.dy = new_y - ps.y;

            e.dx = ps.dx;
            e.dy = ps.dy;
        }

        ps.x = new_x;
        ps.y = new_y;
        ps.has_pos = true;
    }
}

void InputManager::Update() {
    m_events.clear();
    m_actions.clear();

    // *) Poll devices -> raw events
    for (auto& d : m_devices) {
        d->Poll(m_events);
    }

    // *) Update pointers
    UpdatePointers(m_events);

    // *) Build key/button state for this frame (from unconsumed events)
    m_key_state.BeginFrame();
    m_key_state.UpdateFromEvents(m_events.data(), m_events.size());

    // *) Feed ImGui from remaining raw events
    if (ImguiManager* imgui = m_app->GetImguiManager()) {
        imgui->Feed(m_events);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // *) Raw routing stage (shortcuts, viewport tools, gestures)
    m_raw_router.Dispatch(m_events);

    // *) Rebuild key state after raw consumption (critical for chords/drag gating)
    m_key_state.BeginFrame();
    m_key_state.UpdateFromEvents(m_events.data(), m_events.size());

    // *) Mapping stage (non-consumed raw -> actions, with player assignment)
    // m_mapper.Map(m_events, m_keys, m_device_routing, m_actions);

    // *) Action routing stage (gameplay)
    for (const auto& a : m_actions) {
        m_router.Dispatch(a);
    }
}

}  // namespace cave
