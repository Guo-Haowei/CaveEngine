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
    void beginFrame();

    void updateFromEvents(const InputEvent* events, size_t count);

    float get(InputDeviceId dev_id, AxisCode axis) const;

    float getDelta(InputDeviceId dev_id, AxisCode axis) const;

    std::vector<InputDeviceId> activeDevices() const;

private:
    std::unordered_map<uint32_t, AxisDeviceState> devices_;
};

}  // namespace cave
