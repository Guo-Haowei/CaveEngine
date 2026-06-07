#include "cave/core/diagnostics/Log.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/display/DisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

using namespace cave::literals;

InputService::InputService()
    : IService("InputService")
    , mapper_(input_action_map_) {}

auto InputService::InitializeImpl() -> Result<void> {
    // @TODO: read it from config file
    InputActionMap& map = actionMap();

    map.AddAction("ui_accept"_sid, ActionValueType::Digital);
    map.BindDigital("ui_accept"_sid, Key::Enter);
    map.BindDigital("ui_accept"_sid, Key::Space);
    map.BindDigital("ui_accept"_sid, Key::PadA);
    map.BindDigital("ui_accept"_sid, Key::LMB);

    map.AddAction("ui_back"_sid, ActionValueType::Digital);
    map.BindDigital("ui_back"_sid, Key::Backspace);
    map.BindDigital("ui_back"_sid, Key::PadB);
    map.BindDigital("ui_accept"_sid, Key::RMB);

    map.AddAction("ui_left"_sid, ActionValueType::Digital);
    map.BindDigital("ui_left"_sid, Key::A);
    map.BindDigital("ui_left"_sid, Key::Left);
    map.BindDigital("ui_left"_sid, Key::PadLeft);

    map.AddAction("ui_right"_sid, ActionValueType::Digital);
    map.BindDigital("ui_right"_sid, Key::D);
    map.BindDigital("ui_right"_sid, Key::Right);
    map.BindDigital("ui_right"_sid, Key::PadRight);

    map.AddAction("ui_up"_sid, ActionValueType::Digital);
    map.BindDigital("ui_up"_sid, Key::W);
    map.BindDigital("ui_up"_sid, Key::Up);
    map.BindDigital("ui_up"_sid, Key::PadUp);

    map.AddAction("ui_down"_sid, ActionValueType::Digital);
    map.BindDigital("ui_down"_sid, Key::S);
    map.BindDigital("ui_down"_sid, Key::Down);
    map.BindDigital("ui_down"_sid, Key::PadDown);

    // Movement scalar axes
    map.AddAction("ui_axis_x"_sid, ActionValueType::Scalar);
    map.AddAction("ui_axis_y"_sid, ActionValueType::Scalar);

    // Keyboard contributes scalar when held
    // map.BindScalar("ui_axis_x"_sid, Key::A, -1.0f);
    // map.BindScalar("ui_axis_x"_sid, Key::D, +1.0f);
    // map.BindScalar("ui_axis_y"_sid, Key::S, -1.0f);
    // map.BindScalar("ui_axis_y"_sid, Key::W, +1.0f);

    // Gamepad axes contribute scalar too
    map.BindScalar("ui_axis_x"_sid, AxisCode::LX, 1.0f, 0.2f);
    map.BindScalar("ui_axis_y"_sid, AxisCode::LY, 1.0f, 0.2f, /*invert=*/true);

    return Result<void>();
}

void InputService::FinalizeImpl() {
}

#if USING(USE_LOG)
static const char* inputDeviceTypeToString(InputDeviceType type) {
    switch (type) {
        case cave::InputDeviceType::KeyboardMouse:
            return "KeyboardMouse";
        case cave::InputDeviceType::Gamepad:
            return "Gamepad";
        default:
            return "None";
    }
}
#endif

void InputService::addDevice(std::unique_ptr<IInputDevice> device) {
    DEV_ASSERT(device);

    LOG_INFO(LogChannel::Input,
             "+{}#{}",
             inputDeviceTypeToString(device->Type()),
             device->Id().value);
    devices_.emplace_back(std::move(device));
}

void InputService::updatePointers(std::vector<InputEvent>& events) {
    for (auto& [_, ps] : pointers_) {
        ps.dx = 0.0f;
        ps.dy = 0.0f;
    }

    for (InputEvent& e : events) {
        if (e.consumed) continue;
        if (e.type != InputEventType::MouseMove) continue;

        auto& ps = pointers_[e.device_id.value];

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

void InputService::updateActions(const DeviceRouting& routing) {
    actionEvents_.clear();
    mapper_.Map(inputEvents_, key_state_, axis_state_, routing, actionEvents_);

    game_input_.m_state.BeginFrame();
    for (const auto& action : actionEvents_) {
        game_input_.m_state.Apply(action);
    }
}

UIInput InputService::buildUIInput() {
    const InputDeviceId device_id{ 0 };

    UIInput input = game_input_.buildUIInput();

    DisplayService& display_service = *m_app->GetDisplayService();

    const auto it = pointers_.find(device_id.value);
    if (it != pointers_.end()) {
        const PointerState pointer = it->second;
        const auto [x_win, y_win] = display_service.GetWindowPos();
        input.cursor_os = math::Vector2f(pointer.x + x_win, pointer.y + y_win);
    }

    return input;
}

void InputService::tick(const FrameTime& time) {
    inputEvents_.clear();
    actionEvents_.clear();

    // *) Poll devices -> raw events
    for (auto& d : devices_) {
        d->Poll(inputEvents_);
    }

    // *) Update pointers
    updatePointers(inputEvents_);

    // *) Build key/button state for this frame (from unconsumed events)
    key_state_.BeginFrame();
    key_state_.UpdateFromEvents(inputEvents_.data(), inputEvents_.size());

    // *) Build axis state for this frame (from unconsumed events)
    axis_state_.BeginFrame();
    axis_state_.UpdateFromEvents(inputEvents_.data(), inputEvents_.size());

    // *) Feed ImGui from remaining raw events
    if (ImguiManager* imgui = m_app->GetImguiManager()) {
        imgui->Feed(inputEvents_);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // *) Raw routing stage (shortcuts, viewport tools, gestures)
    InputFrame input_frame{
        .dt = time.dt,
        .events = inputEvents_,
        .keystate = key_state_,
    };
    router_.Dispatch(input_frame);

    // *) Rebuild key state after raw consumption (critical for chords/drag gating)
    // m_key_state.BeginFrame();
    // m_key_state.UpdateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Mapping stage (non-consumed raw -> actions, with player assignment)
    DeviceRouting routing;
    updateActions(routing);

    // *) Build UI Input
    ui_input_ = buildUIInput();
}

}  // namespace cave
