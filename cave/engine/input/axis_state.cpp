#include "axis_state.h"

namespace cave {

void AxisState::BeginFrame() {
    for (auto& [_, dev] : m_devices) {
        for (auto& a : dev.axes) {
            a.delta = 0.0f;
        }
        dev.active = false;
    }
}

void AxisState::UpdateFromEvents(const InputEvent* p_events, size_t p_count) {
    for (size_t i = 0; i < p_count; ++i) {
        const InputEvent& e = p_events[i];
        if (e.type != InputEventType::Axis) {
            continue;
        }

        AxisCode axis = static_cast<AxisCode>(e.code);
        auto& dev = m_devices[e.device_id.value];
        dev.axes[std::to_underlying(axis)].value = e.x;
    }
}

float AxisState::Get(InputDeviceId p_dev_id, AxisCode p_axis) const {
    auto it = m_devices.find(p_dev_id.value);
    if (it == m_devices.end()) {
        return 0.0f;
    }
    return it->second.axes[std::to_underlying(p_axis)].value;
}

float AxisState::GetDelta(InputDeviceId p_dev_id, AxisCode p_axis) const {
    auto it = m_devices.find(p_dev_id.value);
    if (it == m_devices.end()) {
        return 0.0f;
    }
    return it->second.axes[std::to_underlying(p_axis)].delta;
}

}  // namespace cave
