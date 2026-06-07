#include "cave/core/diagnostics/Log.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

using namespace cave::literals;
using namespace cave::math;

InputService::InputService()
    : IService("InputService")
    , game_input_(pointers_.data()) {}

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
             inputDeviceTypeToString(device->type()),
             device->id().value);
    devices_.emplace_back(std::move(device));
}

void InputService::updatePointers(std::vector<InputEvent>& events) {
    for (PointerState& pointer : pointers_) {
        pointer.delta = Vector2f::Zero;
    }

    for (InputEvent& e : events) {
        if (e.consumed) continue;
        if (e.type != InputEventType::MouseMove) continue;

        PointerState& pointer = pointers_[e.dev_id.value];

        Vector2f new_pos{ e.x, e.y };

        if (pointer.has_pos) {
            pointer.delta = new_pos - pointer.pos_win;

            e.dx = pointer.delta.x;
            e.dy = pointer.delta.y;
        }

        pointer.pos_win = new_pos;
        pointer.has_pos = true;
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

    const PointerState& pointer = pointers_[device_id.value];
    Vector2f window_pos = display_service.windowPos();
    input.cursor_os = pointer.pos_win + window_pos;

    return input;
}

void InputService::tick(const FrameTime& time) {
    input_events_.clear();

    // *) Poll devices -> raw events
    for (auto& d : devices_) {
        d->poll(input_events_);
    }

    // *) Update pointers
    updatePointers(input_events_);

    // *) Build key/button state for this frame (from unconsumed events)
    key_state_.beginFrame();
    key_state_.updateFromEvents(input_events_.data(), input_events_.size());

    // *) Build axis state for this frame (from unconsumed events)
    axis_state_.beginFrame();
    axis_state_.updateFromEvents(input_events_.data(), input_events_.size());

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
    router_.dispatch(input_frame);

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
