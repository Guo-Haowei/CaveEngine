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

void InputManager::Update() {
    m_events.clear();
    m_actions.clear();

    // 1) Poll devices -> raw events
    for (auto& d : m_devices) {
        d->Poll(m_events);
    }

    // Update key state
    // 2) Build key/button state for this frame (from unconsumed events)
    m_key_state.BeginFrame();
    m_key_state.UpdateFromEvents(m_events.data(), m_events.size());

    // 3) Raw routing stage (shortcuts, viewport tools, gestures)
    //    Raw consumers can:
    //      - mark events consumed
    //      - optionally inject actions (via callback to InputSystem::PushAction)
    // m_raw_router.Dispatch(m_events);

    // 4) Feed ImGui from remaining raw events
    //    You can place this before raw routing if you want UI to get first dibs;
    //    for editor viewport control you usually gate by hit-test.
    if (ImguiManager* imgui = m_app->GetImguiManager()) {
        imgui->Feed(m_events);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // 5) Rebuild key state after raw consumption (critical for chords/drag gating)
    m_key_state.BeginFrame();
    m_key_state.UpdateFromEvents(m_events.data(), m_events.size());

    // 6) Mapping stage (non-consumed raw -> actions, with player assignment)
    // m_mapper.Map(m_events, m_keys, m_device_routing, m_actions);

    // @TODO: map input to action
    for (const auto& a : m_actions) {
        m_router.Dispatch(a);
    }
}

#if 0
#define STR_ID(x) (x)

// @TODO: refactor this
void InputManager::FillViewportInput(ViewportInput& p_out_viewport_input) {
    p_out_viewport_input.buttons = m_buttons;
    p_out_viewport_input.keys = m_keys;
    p_out_viewport_input.mouse_move = MouseMove();
    p_out_viewport_input.wheel_delta = static_cast<float>(m_wheel_y);
}
#endif

}  // namespace cave
