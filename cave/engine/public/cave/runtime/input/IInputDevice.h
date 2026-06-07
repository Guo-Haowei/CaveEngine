// =============================================================================
// File: cave/runtime/input/IInputDevice.h
// =============================================================================
#pragma once
#include <vector>
#include "cave/runtime/input/InputTypes.h"

namespace cave {

struct InputEvent;

class IInputDevice {
public:
    virtual ~IInputDevice() = default;

    virtual InputDeviceType type() const = 0;
    virtual InputDeviceId id() const = 0;

    virtual void poll(std::vector<InputEvent>& out_events) = 0;
};

}  // namespace cave