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

    void addDevice(std::unique_ptr<IInputDevice> p_device);

    void tick(const FrameTime& p_time);

    const KeyState& keyState() const {
        return keyState_;
    }

    void addConsumer(IInputConsumer* p_consumer) {
        router_.Register(p_consumer);
    }

    void removeConsumer(IInputConsumer* p_consumer) {
        router_.Unregister(p_consumer);
    }

    InputActionMap& actionMap() { return inputActionMap_; }

    const UIInput& getUIInput() const { return uiInput_; }
    const GameInput& gameInput() const { return gameInput_; }

private:
    void updatePointers(std::vector<InputEvent>& p_events);
    void updateActions(const DeviceRouting& p_routing);
    UIInput buildUIInput();

    std::vector<std::unique_ptr<IInputDevice>> devices_{};

    std::vector<InputEvent> inputEvents_;
    std::vector<ActionEvent> actionEvents_;

    std::unordered_map<uint32_t, PointerState> pointers_;

    KeyState keyState_;
    AxisState axisState_;
    ActionState actionState_;

    GameInput gameInput_;
    InputRouter router_;

    InputActionMap inputActionMap_;
    InputMapper mapper_;
    UIInput uiInput_;
};

};  // namespace cave
