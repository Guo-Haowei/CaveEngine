#pragma once
#include "engine/input/input_types.h"

namespace cave {

class IRawInputConsumer {
public:
    virtual ~IRawInputConsumer() = default;
    virtual int GetPriority() const = 0;
    virtual void OnEvents(const std::vector<InputEvent>& p_events) = 0;
};

}  // namespace cave