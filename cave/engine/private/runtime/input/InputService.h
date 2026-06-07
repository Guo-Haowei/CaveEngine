#pragma once
#include "cave/core/Singleton.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/framework/IService.h"
#include "cave/runtime/input/IInputDevice.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/ui/UIInput.h"

#include "engine/private/runtime/input/AxisState.h"
#include "engine/private/runtime/input/GameInput.h"
#include "engine/private/runtime/input/InputActionMap.h"
#include "engine/private/runtime/input/InputMapper.h"
#include "engine/private/runtime/input/InputRouter.h"

namespace cave {

struct FrameTime;
class IInputConsumer;
class KeyState;

struct PointerState {
    bool has_pos = false;
    float x = 0.0f, y = 0.0f;
    float dx = 0.0f, dy = 0.0f;
};

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

    InputActionMap& actionMap() { return input_action_map_; }

    const UIInput& getUIInput() const { return ui_input_; }
    const GameInput& gameInput() const { return game_input_; }

private:
    void updatePointers(std::vector<InputEvent>& events);
    void updateActions(const DeviceRouting& routing);
    UIInput buildUIInput();

    std::vector<std::unique_ptr<IInputDevice>> devices_{};

    std::vector<InputEvent> inputEvents_;
    std::vector<ActionEvent> actionEvents_;

    std::unordered_map<uint32_t, PointerState> pointers_;

    KeyState key_state_;
    AxisState axis_state_;
    ActionState action_state_;

    GameInput game_input_;
    InputRouter router_;

    InputActionMap input_action_map_;
    InputMapper mapper_;
    UIInput ui_input_;
};

};  // namespace cave
