// =============================================================================
// File: engine/private/runtime/input/ActionState.h
// =============================================================================
#pragma once
#include "InputTypes.h"

namespace cave {

struct ActionStateEntry {
    bool down = false;
    bool just_pressed = false;
    bool just_released = false;

    float x = 0.0f;
    float y = 0.0f;
};

class ActionState {
public:
    void BeginFrame();

    void Apply(const ActionEvent& p_event);

    bool IsPressed(int p_player, const StringId& p_str_id) const;

    bool IsJustPressed(int p_player, const StringId& p_str_id) const;

    bool IsJustReleased(int p_player, const StringId& p_str_id) const;

    float GetStrength(int p_player, const StringId& p_str_id) const;

    std::pair<float, float> GetVector(int p_player,
                                      const StringId& p_neg_x,
                                      const StringId& p_pos_x,
                                      const StringId& p_neg_y,
                                      const StringId& p_pos_y) const;

private:
    const ActionStateEntry* Find(int p_player, const StringId& p_str_id) const;

    std::unordered_map<int, std::unordered_map<StringId, ActionStateEntry>> m_state;
};

}  // namespace cave