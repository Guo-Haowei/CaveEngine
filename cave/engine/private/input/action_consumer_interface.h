#pragma once
#include "engine/private/input/input_types.h"

namespace cave {

class IActionConsumer {
public:
    virtual ~IActionConsumer() = default;
    virtual int GetPriority() const = 0;
    virtual bool OnAction(const ActionEvent& p_action) = 0;
};

}  // namespace cave