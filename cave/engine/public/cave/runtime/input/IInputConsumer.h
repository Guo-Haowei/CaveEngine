// =============================================================================
// File: public/cave/runtime/input/IInputConsumer.h
// =============================================================================
#pragma once

namespace cave {

struct InputEvent;

class IInputConsumer {
public:
    virtual ~IInputConsumer() = default;
    virtual int GetPriority() const = 0;
    virtual void OnEvents(const std::vector<InputEvent>& p_events) = 0;
};

}  // namespace cave