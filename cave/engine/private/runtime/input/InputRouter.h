// =============================================================================
// File: engine/private/runtime/input/InputRouter.h
// =============================================================================
#pragma once
#include "cave/runtime/input/IInputConsumer.h"

namespace cave {

struct InputEvent;

class InputRouter {
public:
    void Register(IInputConsumer* p_consumer);
    void Unregister(IInputConsumer* p_consumer);

    void Dispatch(const InputFrame& p_input);

private:
    void Sort();

    std::vector<IInputConsumer*> m_consumers;
};

}  // namespace cave
