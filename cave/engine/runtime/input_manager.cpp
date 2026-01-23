#include "input_manager.h"

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
    if (DEV_VERIFY(p_device)) {
        m_devices.push_back(std::move(p_device));
    }
}

void InputManager::Update() {
    m_events.clear();
    m_actions.clear();

    // Poll raw events from registered devices
    for (auto& d : m_devices) {
        d->Poll(m_events);
    }

    // Update key state

    /*
    Poll raw events
    Update key state
    Raw event dispatcher
    Feed ImGui before or after raw
    Mapper maps to remaining actions
    Action router
    */

#if 0
    // 2) Update key state (from *unconsumed* events only)
    m_keys.BeginFrame();
    m_keys.UpdateFromEvents(m_events.data(), m_events.size());

    // 4) IMPORTANT: update key state again to reflect any consumed events (Ctrl+S consumes S down)
    //    This prevents mapping from seeing S as down this frame if you rely on PressedThisFrame.
    //    For pure Down() checks, the consumed keydown already happened, so rebuild key state cleanly.
    //    Simple approach: rebuild from scratch (cheap for Phase 1).
    m_keys = KeyState{};
    m_keys.BeginFrame();
    m_keys.UpdateFromEvents(m_events.data(), m_events.size());

    // 5) Map remaining input -> actions
    m_mapper.Map(m_events, m_keys, m_actions);

    // 6) Route actions by priority/consumption
#endif

    // @TODO: map input to action
    for (const auto& a : m_actions) {
        m_router.Dispatch(a);
    }
}

#if 0
#define STR_ID(x) (x)

void InputManager::BeginFrame() {
    const bool alt = IsKeyDown(Key::LeftAlt) || IsKeyDown(Key::RightAlt);
    const bool ctrl = IsKeyDown(Key::LeftCtrl) || IsKeyDown(Key::RightCtrl);
    const bool shift = IsKeyDown(Key::LeftShift) || IsKeyDown(Key::RightShift);
    const bool modifier_pressed = alt || ctrl || shift;

    // Send key events
    for (int i = 0; i < std::to_underlying(Key::COUNT); ++i) {
        const auto value = m_keys[i];
        const auto prev_value = m_prev_keys[i];

        auto get_state = [&]() {
            if (value == true && prev_value == false) {
                return InputState::PRESSED;
            }
            if (value == false && prev_value == true) {
                return InputState::RELEASED;
            }
            if (value == true && !modifier_pressed) {
                return InputState::HOLD;
            }
            return InputState::UNKNOWN;
        };

        InputState state = get_state();
        if (state == InputState::UNKNOWN) {
            continue;
        }
        auto e = std::make_shared<InputEventKey>();
        e->m_key = static_cast<Key>(i);
        e->m_state = state;
        e->m_alt_pressed = alt;
        e->m_ctrl_pressed = ctrl;
        e->m_shift_pressed = shift;

        //m_router.Route(e);
    }

    // Send mouse wheel events
    if (m_wheel_x != 0 || m_wheel_y != 0) {
        auto e = std::make_shared<InputEventMouseWheel>(m_buttons,
                                                        m_prev_buttons,
                                                        m_cursor,
                                                        static_cast<float>(m_wheel_y));
        e->m_alt_pressed = alt;
        e->m_ctrl_pressed = ctrl;
        e->m_shift_pressed = shift;

        //m_router.Route(e);
    }

    // Send mouse moved event
    if (m_mouse_moved) {
        auto e = std::make_shared<InputEventMouseMove>(m_buttons, m_prev_buttons, m_cursor, m_prev_cursor);
        e->m_alt_pressed = alt;
        e->m_ctrl_pressed = ctrl;
        e->m_shift_pressed = shift;

        //m_router.Route(e);
    }
}

void InputManager::EndFrame() {
    m_prev_keys = m_keys;
    m_prev_buttons = m_buttons;
    m_prev_cursor = m_cursor;

    m_wheel_x = 0;
    m_wheel_y = 0;

    m_mouse_moved = false;
}

void InputManager::PushInputHandler(IInputHandler* p_input_handler) {
    m_router.PushHandler(p_input_handler);
}

IInputHandler* InputManager::PopInputHandler() {
    return m_router.PopHandler();
}

bool InputManager::IsKeyDown(Key p_key) {
    return m_keys[std::to_underlying(p_key)];
}

bool InputManager::IsKeyPressed(Key p_key) {
    return InputHasChanged(m_keys, m_prev_keys, std::to_underlying(p_key));
}

bool InputManager::IsKeyReleased(Key p_key) {
    return InputHasChanged(m_prev_keys, m_keys, std::to_underlying(p_key));
}

// @TODO: support controller, touch screen, etc
bool InputManager::IsActionPressed(StringId p_name) {
    auto it = m_input_binding.find(p_name);
    if (it == m_input_binding.end()) return false;
    return IsKeyDown(static_cast<Key>(it->second));
}

bool InputManager::IsActionJustPressed(StringId p_name) {
    auto it = m_input_binding.find(p_name);
    if (it == m_input_binding.end()) return false;
    const bool pressed = IsKeyPressed(static_cast<Key>(it->second));
    if (pressed) {
        return true;
    }
    return false;
}

bool InputManager::IsActionJustReleased(StringId p_name) {
    auto it = m_input_binding.find(p_name);
    if (it == m_input_binding.end()) return false;
    return IsKeyReleased(static_cast<Key>(it->second));
}

Vector2f InputManager::MouseMove() {
    Vector2f point;
    point = m_cursor - m_prev_cursor;
    return point;
}

void InputManager::SetButton(MouseButton p_button, bool p_pressed) {
    ERR_FAIL_INDEX(p_button, MouseButton::COUNT);
    m_buttons[std::to_underlying(p_button)] = p_pressed;
}

void InputManager::SetKey(Key p_key, bool p_pressed) {
    ERR_FAIL_INDEX(p_key, Key::COUNT);
    const auto index = std::to_underlying(p_key);
    m_keys[index] = p_pressed;
}

void InputManager::SetCursor(float p_x, float p_y) {
    m_cursor.x = p_x;
    m_cursor.y = p_y;

    m_mouse_moved = true;
}

void InputManager::SetWheel(double p_x, double p_y) {
    m_wheel_x = p_x;
    m_wheel_y = p_y;
}

Vector2f InputManager::GetWheel() const {
    return Vector2f(m_wheel_x, m_wheel_y);
}

// @TODO: refactor this
void InputManager::FillViewportInput(ViewportInput& p_out_viewport_input) {
    p_out_viewport_input.buttons = m_buttons;
    p_out_viewport_input.keys = m_keys;
    p_out_viewport_input.mouse_move = MouseMove();
    p_out_viewport_input.wheel_delta = static_cast<float>(m_wheel_y);
}
#endif

}  // namespace cave
