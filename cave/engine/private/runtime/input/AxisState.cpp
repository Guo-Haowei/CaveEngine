// =============================================================================
// File: engine/private/runtime/input/AxisState.cpp
// =============================================================================
#include "AxisState.h"

namespace cave {

void AxisState::beginFrame() {
    for (auto& [_, dev] : devices_) {
        for (auto& a : dev.axes) {
            a.delta = 0.0f;
        }
        dev.active = false;
    }
}

void AxisState::updateFromEvents(const InputEvent* events, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const InputEvent& e = events[i];
        if (e.type != InputEventType::Axis) {
            continue;
        }

        AxisCode axis = static_cast<AxisCode>(e.code);
        auto& dev = devices_[e.device_id.value];
        dev.axes[std::to_underlying(axis)].value = e.x;
    }
}

float AxisState::get(InputDeviceId dev_id, AxisCode axis) const {
    auto it = devices_.find(dev_id.value);
    if (it == devices_.end()) {
        return 0.0f;
    }
    return it->second.axes[std::to_underlying(axis)].value;
}

float AxisState::getDelta(InputDeviceId dev_id, AxisCode axis) const {
    auto it = devices_.find(dev_id.value);
    if (it == devices_.end()) {
        return 0.0f;
    }
    return it->second.axes[std::to_underlying(axis)].delta;
}

std::vector<InputDeviceId> AxisState::activeDevices() const {
    std::vector<InputDeviceId> devices;
    for (const auto& [key, _] : devices_) {
        devices.push_back(InputDeviceId{ key });
    }
    return devices;
}

}  // namespace cave
