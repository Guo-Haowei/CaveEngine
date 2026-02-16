// =============================================================================
// File: engine/public/cave/runtime/input/IInputDevice.h
// =============================================================================
#pragma once
#include <vector>
#include "cave/runtime/input/InputTypes.h"

namespace cave {

struct InputEvent;

class IInputDevice {
public:
    virtual ~IInputDevice() = default;
    virtual InputDeviceType Type() const = 0;
    virtual InputDeviceId Id() const = 0;

    virtual void Poll(std::vector<InputEvent>& p_out_events) = 0;
};

}  // namespace cave