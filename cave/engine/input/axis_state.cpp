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

void AxisState::Set(InputDeviceId p_dev_id, AxisCode p_axis, float p_value) {
    auto& d = m_devices[p_dev_id.value];
    d.active = true;

    AxisSample& s = d.axes[std::to_underlying(p_axis)];
    const float prev = s.value;
    s.value = p_value;
    s.delta += (p_value - prev);  // accumulate if set multiple times per frame
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
