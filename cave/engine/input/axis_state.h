#pragma once
#include "engine/input/input_types.h"
#include "engine/input/key_code.h"

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

    // Set axis value for a device (called by devices)
    void Set(InputDeviceId p_dev_id, AxisCode p_axis, float p_value);

    // Read current axis value
    float Get(InputDeviceId p_dev_id, AxisCode p_axis) const;

    // Read axis delta (this frame)
    float GetDelta(InputDeviceId p_dev_id, AxisCode p_axis) const;

private:
    std::unordered_map<uint32_t, AxisDeviceState> m_devices;
};

}  // namespace cave
