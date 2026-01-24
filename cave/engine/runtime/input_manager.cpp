#include "input_manager.h"

#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

namespace cave {

InputManager::InputManager()
    : Module("InputManager")
    , m_mapper(m_input_action_map) {}

auto InputManager::InitializeImpl() -> Result<void> {
    InputActionMap& map = ActionMap();
    map.AddAction(StringId("ui_left"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_left"), Key::Left);
    map.BindDigital(StringId("ui_left"), Key::A);

    map.AddAction(StringId("ui_right"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_right"), Key::Right);
    map.BindDigital(StringId("ui_right"), Key::D);

    map.AddAction(StringId("ui_up"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_up"), Key::Up);
    map.BindDigital(StringId("ui_up"), Key::W);

    map.AddAction(StringId("ui_down"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_down"), Key::Down);
    map.BindDigital(StringId("ui_down"), Key::S);

    map.AddAction(StringId("ui_accept"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_accept"), Key::Enter);
    map.BindDigital(StringId("ui_accept"), Key::Space);

    map.AddAction(StringId("ui_back"), ActionValueType::Digital);
    map.BindDigital(StringId("ui_back"), Key::Backspace);

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

void InputManager::UpdateActions(const DeviceRouting& p_routing) {
    m_action_events.clear();
    m_mapper.Map(m_input_events, m_key_state, p_routing, m_action_events);

    m_action_state.BeginFrame();
    for (const auto& action : m_action_events) {
        m_action_state.Apply(action);
    }
}

void InputManager::Update() {
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

    // *) Feed ImGui from remaining raw events
    if (ImguiManager* imgui = m_app->GetImguiManager()) {
        imgui->Feed(m_input_events);

        // Gate gameplay/editor mapping based on ImGui capture
        // const bool blockKeyboard = imgui->WantKeyboard();
        // const bool blockMouse = imgui->WantMouse();
    }

    // *) Raw routing stage (shortcuts, viewport tools, gestures)
    m_raw_router.Dispatch(m_input_events);

    // *) Rebuild key state after raw consumption (critical for chords/drag gating)
    m_key_state.BeginFrame();
    m_key_state.UpdateFromEvents(m_input_events.data(), m_input_events.size());

    // *) Mapping stage (non-consumed raw -> actions, with player assignment)
    DeviceRouting routing;
    UpdateActions(routing);

    // *) Action routing stage (gameplay)
    for (const auto& a : m_action_events) {
        m_input_router.Dispatch(a);
    }
}

}  // namespace cave
