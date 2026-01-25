#pragma once
#include "engine/private/input/input_types.h"

namespace cave {

class IInputDevice {
public:
    virtual ~IInputDevice() = default;
    virtual InputDeviceType Type() const = 0;
    virtual InputDeviceId Id() const = 0;

    virtual void Poll(std::vector<InputEvent>& p_out_events) = 0;
};

}  // namespace cave