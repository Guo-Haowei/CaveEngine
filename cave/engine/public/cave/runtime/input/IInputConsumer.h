// =============================================================================
// File: public/cave/runtime/input/IInputConsumer.h
// =============================================================================
#pragma once
#include "cave/core/ids/DebugId.h"

namespace cave {

struct InputEvent;

class IInputConsumer {
public:
    virtual ~IInputConsumer() = default;

    virtual int GetPriority() const = 0;
    virtual void OnEvents(const std::vector<InputEvent>& p_events) = 0;
    virtual DebugId GetDebugId() = 0;
};

}  // namespace cave