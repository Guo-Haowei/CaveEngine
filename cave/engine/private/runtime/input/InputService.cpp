#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/display/DisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

InputService::InputService()
    : IInputService("InputService")
    , m_mapper(m_input_action_map) {}

auto InputService::InitializeImpl() -> Result<void> {
    // @TODO: read it from config file
    InputActionMap& map = ActionMap();

    map.AddAction(StringId("ui_accept"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_accept"), Key::Enter);
    map.BindDigital(StringId("ui_accept"), Key::Space);
    map.BindDigital(StringId("ui_accept"), Key::PadA);

    map.AddAction(StringId("ui_back"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_back"), Key::Backspace);
    map.BindDigital(StringId("ui_back"), Key::PadB);

    map.AddAction(StringId("ui_left"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_left"), Key::A);
    map.BindDigital(StringId("ui_left"), Key::Left);
    map.BindDigital(StringId("ui_left"), Key::PadLeft);

    map.AddAction(StringId("ui_right"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_right"), Key::D);
    map.BindDigital(StringId("ui_right"), Key::Right);
    map.BindDigital(StringId("ui_right"), Key::PadRight);

    map.AddAction(StringId("ui_up"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_up"), Key::W);
    map.BindDigital(StringId("ui_up"), Key::Up);
    map.BindDigital(StringId("ui_up"), Key::PadUp);

    map.AddAction(StringId("ui_down"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_down"), Key::S);
    map.BindDigital(StringId("ui_down"), Key::Down);
    map.BindDigital(StringId("ui_down"), Key::PadDown);

    // Movement scalar axes
    map.AddAction(StringId("ui_axis_x"), ActionValueType::Scalar);
    map.AddAction(StringId("ui_axis_y"), ActionValueType::Scalar);

    // Keyboard contributes scalar when held
    // map.BindScalar(StringId("ui_axis_x"), Key::A, -1.0f);
    // map.BindScalar(StringId("ui_axis_x"), Key::D, +1.0f);
    // map.BindScalar(StringId("ui_axis_y"), Key::S, -1.0f);
    // map.BindScalar(StringId("ui_axis_y"), Key::W, +1.0f);

    // Gamepad axes contribute scalar too
    map.BindScalar(StringId("ui_axis_x"), AxisCode::LX, 1.0f, 0.2f);
    map.BindScalar(StringId("ui_axis_y"), AxisCode::LY, 1.0f, 0.2f, /*invert=*/true);

    return Result<void>();
}

void InputService::FinalizeImpl() {
}

static const char* InputDeviceTypeToString(InputDeviceType p_type) {
    switch (p_type) {
        case cave::InputDeviceType::KeyboardMouse:
            return "KeyboardMouse";
        case cave::InputDeviceType::Gamepad:
            return "Gamepad";
        default:
            return "None";
    }
}

void InputService::AddDevice(std::unique_ptr<IInputDevice> p_device) {
    DEV_ASSERT(p_device);

    LOG("+{}#{}",
        InputDeviceTypeToString(p_device->Type()),
        p_device->Id().value);
    m_devices.emplace_back(std::move(p_device));
}

void InputService::UpdatePointers(std::vector<InputEvent>& p_events) {
    for (auto& [_, ps] : m_pointers) {
        ps.dx = 0.0f;
        ps.dy = 0.0f;
    }

    for (InputEvent& e : p_events) {
        if (e.consumed) continue;
        if (e.type != InputEventType::MouseMove) continue;

        auto& ps = m_pointers[e.device_id.value];

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

void InputService::UpdateActions(const DeviceRouting& p_routing) {
    m_action_events.clear();
    m_mapper.Map(m_input_events, m_key_state, m_axis_state, p_routing, m_action_events);

    m_action_state.BeginFrame();
    for (const auto& action : m_action_events) {
        m_action_state.Apply(action);
    }
}

UIInput InputService::BuildUIInput() {
    UIInput input{};

    const InputDeviceId device_id{ 0 };
    const int player_id = 0;

    DisplayService& display_service = *m_app->GetDisplayService();

    const auto it = m_pointers.find(device_id.value);
    if (it != m_pointers.end()) {
        const PointerState pointer = it->second;
        const auto [x_win, y_win] = display_service.GetWindowPos();
        input.cursor_os = math::Vector2f(pointer.x + x_win, pointer.y + y_win);
    }

    // Mouse buttons
    input.mouse_down = m_key_state.Down(device_id, Key::LMB);
    input.mouse_pressed = m_key_state.PressedThisFrame(device_id, Key::LMB);
    input.mouse_released = m_key_state.ReleasedThisFrame(device_id, Key::LMB);

    // Mapped UI actions
    input.submit_pressed = m_action_state.IsJustPressed(player_id, StringId("ui_accept"));
    input.cancel_pressed = m_action_state.IsJustPressed(player_id, StringId("ui_back"));

    input.left_pressed = m_action_state.IsJustPressed(player_id, StringId("ui_left"));
    input.right_pressed = m_action_state.IsJustPressed(player_id, StringId("ui_right"));
    input.up_pressed = m_action_state.IsJustPressed(player_id, StringId("ui_up"));
    input.down_pressed = m_action_state.IsJustPressed(player_id, StringId("ui_down"));

    return input;
}

void InputService::Tick(const FrameTime& p_time) {
    m_input_events.clear();
    m_action_events.clear();

    // *) Poll devices -> raw events
    for (auto& d : m_devices) {
        d->Poll(m_input_events);
    }

    // *) Update pointers
    UpdatePointers(m_input_events);

    // *) Build key/button state for this frame (from unconsumed events)
    m_key_state.BeginFrame();
    m_key_state.UpdateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Build axis state for this frame (from unconsumed events)
    m_axis_state.BeginFrame();
    m_axis_state.UpdateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Feed ImGui from remaining raw events
    if (ImguiManager* imgui = m_app->GetImguiManager()) {
        imgui->Feed(m_input_events);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // *) Raw routing stage (shortcuts, viewport tools, gestures)
    InputFrame input_frame{
        .dt = p_time.dt,
        .events = m_input_events,
        .keystate = m_key_state,
    };
    m_router.Dispatch(input_frame);

    // *) Rebuild key state after raw consumption (critical for chords/drag gating)
    // m_key_state.BeginFrame();
    // m_key_state.UpdateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Mapping stage (non-consumed raw -> actions, with player assignment)
    DeviceRouting routing;
    UpdateActions(routing);

    // *) Build UI Input
    m_ui_input = BuildUIInput();
}

}  // namespace cave
