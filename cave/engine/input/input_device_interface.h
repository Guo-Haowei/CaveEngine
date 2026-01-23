#pragma once
#include "engine/input/input_types.h"

// @TODO: refactor
#include "engine/input/input_event.h"

namespace cave {

class IInputDevice {
public:
    using Event = std::shared_ptr<InputEvent>;

    virtual ~IInputDevice() = default;
    virtual InputDeviceType Type() const = 0;
    virtual InputDeviceId Id() const = 0;

    virtual void Poll(std::vector<Event>& p_out_events) = 0;
};

}  // namespace cave