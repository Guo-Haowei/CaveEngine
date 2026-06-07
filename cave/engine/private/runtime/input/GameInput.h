#include "cave/runtime/input/IGameInput.h"
#include "cave/ui/UIInput.h"

#include "engine/private/runtime/input/ActionState.h"

namespace cave {

class GameInput final : public IGameInput {
public:
    bool IsPressed(int p_player,
                   const StringId& p_action) const override {
        return m_state.IsPressed(p_player, p_action);
    }

    bool IsJustPressed(int p_player,
                       const StringId& p_action) const override {
        return m_state.IsJustPressed(p_player, p_action);
    }

    bool IsJustReleased(int p_player,
                        const StringId& p_action) const override {
        return m_state.IsJustReleased(p_player, p_action);
    }

    float GetStrength(int p_player,
                      const StringId& p_action) const override {
        return m_state.GetStrength(p_player, p_action);
    }

    std::pair<float, float>
    GetVector(int p_player,
              const StringId& p_neg_x,
              const StringId& p_pos_x,
              const StringId& p_neg_y,
              const StringId& p_pos_y) const override {
        return m_state.GetVector(p_player,
                                 p_neg_x,
                                 p_pos_x,
                                 p_neg_y,
                                 p_pos_y);
    }

    UIInput BuildUIInput();

public:
    ActionState m_state;
};

}  // namespace cave
