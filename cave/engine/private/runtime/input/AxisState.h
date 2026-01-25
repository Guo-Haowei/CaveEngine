// =============================================================================
// File: engine/private/runtime/input/AxisState.h
// =============================================================================
#pragma once
#include "cave/runtime/input/InputTypes.h"
#include "cave/runtime/input/KeyCode.h"

namespace cave {

struct AxisSample {
    float value{ 0.0f };  // current value
    float delta{ 0.0f };  // value - prev_value this frame
};

struct AxisDeviceState {
    std::array<AxisSample, kAxisCount> axes{};
    bool active{ false };
};

class AxisState {
public:
    void BeginFrame();

    void UpdateFromEvents(const InputEvent* p_events, size_t p_count);

    float Get(InputDeviceId p_dev_id, AxisCode p_axis) const;

    float GetDelta(InputDeviceId p_dev_id, AxisCode p_axis) const;

    std::vector<InputDeviceId> ActiveDevices() const;

private:
    std::unordered_map<uint32_t, AxisDeviceState> m_devices;
};

}  // namespace cave
