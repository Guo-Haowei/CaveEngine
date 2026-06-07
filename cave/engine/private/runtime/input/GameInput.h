#pragma once
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/input/InputTypes.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/ui/UIInput.h"

#include "engine/private/runtime/input/AxisState.h"
#include "engine/private/runtime/input/InputMapper.h"

namespace cave {

class GameInput final : public IGameInput {
public:
    GameInput(const PointerState* pointers);

    void initialize();

    bool isPressed(StringId action, int player) const override;

    bool isJustPressed(StringId action, int player) const override;

    bool isJustReleased(StringId action, int player) const override;

    float getStrength(StringId action, int player) const override;

    auto getVector(StringId neg_x,
                   StringId pos_x,
                   StringId neg_y,
                   StringId pos_y,
                   int player) const -> std::pair<float, float> override;

    void updateActions(const std::vector<InputEvent>& input_events,
                       const KeyState& key_state,
                       const AxisState& axis_state,
                       const DeviceRouting& routing);

    const PointerState& pointerState() const override;

private:
    struct Entry {
        bool down = false;
        bool just_pressed = false;
        bool just_released = false;

        float x = 0.0f;
        float y = 0.0f;
    };

    void beginFrame();
    void apply(const ActionEvent& event);

    const Entry* findEntry(int player, StringId action) const;

    const PointerState* pointers_{ nullptr };
    InputActionMap input_action_map_;
    InputMapper mapper_;
    std::vector<ActionEvent> action_events_;
    std::unordered_map<int, std::unordered_map<StringId, Entry>> entries_;
};

}  // namespace cave
