#pragma once
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
    InputService(GameInput& game_input);

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void addDevice(std::unique_ptr<IInputDevice> device);

    void tick(const FrameTime& time);

    const KeyState& keyState() const {
        return m_key_state;
    }

    void addConsumer(IInputConsumer* consumer) {
        m_router.addConsumer(consumer);
    }

    void removeConsumer(IInputConsumer* consumer) {
        m_router.removeConsumer(consumer);
    }

    const UIInput& getUIInput() const { return m_ui_input; }
    const GameInput& gameInput() const { return m_game_input; }
    // @TODO: fix this part
    const PointerState* pointers() const { return m_pointers.data(); }

private:
    void updatePointers(std::vector<InputEvent>& events);
    UIInput buildUIInput();

    Vector<Owner<IInputDevice>> m_devices{};
    Vector<InputEvent> m_input_events;

    std::array<PointerState, InputDeviceId::kMax> m_pointers;

    KeyState m_key_state;
    AxisState m_axis_state;

    GameInput& m_game_input;
    InputRouter m_router;

    UIInput m_ui_input;
};

};  // namespace cave
