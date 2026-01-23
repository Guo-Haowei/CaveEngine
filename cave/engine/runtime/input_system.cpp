#include "input_system.h"

#include "engine/input/input_code.h"

namespace cave {

void InputSystem::AddDevice(std::unique_ptr<IInputDevice> p_device) {
    if (DEV_VERIFY(p_device)) {
        m_devices.push_back(std::move(p_device));
    }
}

void InputSystem::Tick() {
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

}  // namespace cave
