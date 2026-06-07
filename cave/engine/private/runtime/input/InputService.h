#pragma once
#include "cave/core/Singleton.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/framework/IService.h"
#include "cave/runtime/input/IInputDevice.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/runtime/input/PointerState.h"
#include "cave/ui/UIInput.h"

#include "engine/private/runtime/input/GameInput.h"
#include "engine/private/runtime/input/InputActionMap.h"
#include "engine/private/runtime/input/InputRouter.h"

namespace cave {

struct FrameTime;
class IInputConsumer;
class KeyState;

class InputService : public IService {
public:
    InputService();

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void addDevice(std::unique_ptr<IInputDevice> device);

    void tick(const FrameTime& time);

    const KeyState& keyState() const {
        return key_state_;
    }

    void addConsumer(IInputConsumer* consumer) {
        router_.Register(consumer);
    }

    void removeConsumer(IInputConsumer* consumer) {
        router_.Unregister(consumer);
    }

    const UIInput& getUIInput() const { return ui_input_; }
    const GameInput& gameInput() const { return game_input_; }

private:
    void updatePointers(std::vector<InputEvent>& events);
    UIInput buildUIInput();

    std::vector<std::unique_ptr<IInputDevice>> devices_{};
    std::vector<InputEvent> input_events_;

    std::array<PointerState, InputDeviceId::kMax> pointers_;

    KeyState key_state_;
    AxisState axis_state_;

    GameInput game_input_;
    InputRouter router_;

    UIInput ui_input_;
};

};  // namespace cave
