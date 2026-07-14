#include "cave/core/time/FrameTime.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

using namespace cave::literals;
using namespace cave::math;

InputService::InputService(GameInput& game_input)
    : IService("InputService")
    , m_game_input(game_input) {}

auto InputService::InitializeImpl() -> Result<void> {
    m_game_input.initialize();
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
    m_devices.emplace_back(std::move(device));
}

void InputService::updatePointers(std::vector<InputEvent>& events) {
    for (PointerState& pointer : m_pointers) {
        pointer.delta = Vec2f::Zero;
    }

    for (InputEvent& e : events) {
        if (e.consumed) continue;
        if (e.type != InputEventType::MouseMove) continue;

        PointerState& pointer = m_pointers[e.dev_id.value];

        Vec2f new_pos{ e.x, e.y };

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

    input.submit_down = m_game_input.isPressed("ui_accept"_sid, player_id);
    input.submit_pressed = m_game_input.isJustPressed("ui_accept"_sid, player_id);
    input.submit_released = m_game_input.isJustReleased("ui_accept"_sid, player_id);

    input.left_pressed = m_game_input.isJustPressed("ui_left"_sid, player_id);
    input.right_pressed = m_game_input.isJustPressed("ui_right"_sid, player_id);
    input.up_pressed = m_game_input.isJustPressed("ui_up"_sid, player_id);
    input.down_pressed = m_game_input.isJustPressed("ui_down"_sid, player_id);

    const PointerState& pointer = m_pointers[device_id.value];
    Vec2f window_pos = m_app->services().displayService().windowPos();
    input.cursor_os = pointer.pos_win + window_pos;

    return input;
}

void InputService::tick(const FrameTime& time) {
    m_input_events.clear();

    // *) Poll devices -> raw events
    for (auto& d : m_devices) {
        d->poll(m_input_events);
    }

    // *) Update pointers
    updatePointers(m_input_events);

    // *) Build key/button state for this frame (from unconsumed events)
    m_key_state.beginFrame();
    m_key_state.updateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Build axis state for this frame (from unconsumed events)
    m_axis_state.beginFrame();
    m_axis_state.updateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Feed ImGui from remaining raw events
    if (ImGuiService* imgui = m_app->services().imgui) {
        imgui->Feed(m_input_events);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // *) Raw routing stage (shortcuts, viewport tools, gestures)
    InputFrame input_frame{
        .dt = time.dt,
        .events = m_input_events,
        .keystate = m_key_state,
    };
    m_router.dispatch(input_frame);

    // *) Mapping stage (non-consumed raw -> actions, with player assignment)
    DeviceRouting routing;
    m_game_input.updateActions(m_input_events,
                               m_key_state,
                               m_axis_state,
                               routing);

    // *) Build UI Input
    m_ui_input = buildUIInput();
}

}  // namespace cave
