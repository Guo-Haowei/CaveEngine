#pragma once
#include "cave/runtime/input/IInputConsumer.h"

namespace cave {

struct InputEvent;

class InputRouter {
public:
    void addConsumer(IInputConsumer* consumer);
    void removeConsumer(IInputConsumer* consumer);

    void dispatch(const InputFrame& input);

private:
    void sortByPriority();

    std::vector<IInputConsumer*> consumers_;
};

}  // namespace cave
