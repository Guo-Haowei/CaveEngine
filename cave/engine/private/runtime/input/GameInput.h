#include "cave/runtime/input/IGameInput.h"
#include "cave/ui/UIInput.h"

#include "engine/private/runtime/input/ActionState.h"

namespace cave {

struct PointerState;

class GameInput final : public IGameInput {
public:
    bool isPressed(StringId action, int player) const override {
        return m_state.IsPressed(player, action);
    }

    bool isJustPressed(StringId action, int player) const override {
        return m_state.IsJustPressed(player, action);
    }

    bool isJustReleased(StringId action, int player) const override {
        return m_state.IsJustReleased(player, action);
    }

    float getStrength(StringId action, int player) const override {
        return m_state.GetStrength(player, action);
    }

    auto getVector(StringId neg_x,
                   StringId pos_x,
                   StringId neg_y,
                   StringId pos_y,
                   int player) const -> std::pair<float, float> override {
        return m_state.GetVector(player,
                                 neg_x,
                                 pos_x,
                                 neg_y,
                                 pos_y);
    }

    UIInput buildUIInput();

public:
    ActionState m_state;
};

}  // namespace cave
