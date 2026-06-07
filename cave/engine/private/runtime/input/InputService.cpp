#include "cave/core/diagnostics/Log.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/display/DisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

using namespace cave::literals;

InputService::InputService()
    : IService("InputService") {}

auto InputService::InitializeImpl() -> Result<void> {
    game_input_.initialize();
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

UIInput InputService::buildUIInput() {
    const InputDeviceId device_id{ 0 };
    constexpr int player_id = 0;

    UIInput input{};

    input.submit_down = game_input_.isPressed("ui_accept"_sid, player_id);
    input.submit_pressed = game_input_.isJustPressed("ui_accept"_sid, player_id);
    input.submit_released = game_input_.isJustReleased("ui_accept"_sid, player_id);

    input.left_pressed = game_input_.isJustPressed("ui_left"_sid, player_id);
    input.right_pressed = game_input_.isJustPressed("ui_right"_sid, player_id);
    input.up_pressed = game_input_.isJustPressed("ui_up"_sid, player_id);
    input.down_pressed = game_input_.isJustPressed("ui_down"_sid, player_id);

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
    input_events_.clear();

    // *) Poll devices -> raw events
    for (auto& d : devices_) {
        d->Poll(input_events_);
    }

    // *) Update pointers
    updatePointers(input_events_);

    // *) Build key/button state for this frame (from unconsumed events)
    key_state_.BeginFrame();
    key_state_.UpdateFromEvents(input_events_.data(), input_events_.size());

    // *) Build axis state for this frame (from unconsumed events)
    axis_state_.BeginFrame();
    axis_state_.UpdateFromEvents(input_events_.data(), input_events_.size());

    // *) Feed ImGui from remaining raw events
    if (ImguiManager* imgui = m_app->GetImguiManager()) {
        imgui->Feed(input_events_);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // *) Raw routing stage (shortcuts, viewport tools, gestures)
    InputFrame input_frame{
        .dt = time.dt,
        .events = input_events_,
        .keystate = key_state_,
    };
    router_.Dispatch(input_frame);

    // *) Mapping stage (non-consumed raw -> actions, with player assignment)
    DeviceRouting routing;
    game_input_.updateActions(input_events_,
                              key_state_,
                              axis_state_,
                              routing);

    // *) Build UI Input
    ui_input_ = buildUIInput();
}

}  // namespace cave
