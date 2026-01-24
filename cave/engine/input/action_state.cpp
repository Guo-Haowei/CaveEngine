#include "action_state.h"

namespace cave {

void ActionState::BeginFrame() {
    for (auto& [player, map] : m_state) {
        for (auto& [_, entry] : map) {
            entry.just_pressed = false;
            entry.just_released = false;
            entry.x = 0.0f;
            entry.y = 0.0f;
            // NOTE: a.down persists; it will be updated by events/axes.
        }
    }
}

void ActionState::Apply(const ActionEvent& p_event) {
    auto& entry = m_state[p_event.player][p_event.action];

    switch (p_event.type) {
        case ActionEventType::Pressed: {
            if (!entry.down) {
                entry.just_pressed = true;
            }
            entry.down = true;
        } break;
        case ActionEventType::Released: {
            if (entry.down) {
                entry.just_released = true;
            }
            entry.down = false;
        } break;
        case ActionEventType::Axis1D: {
            entry.x = p_event.x;
            entry.down = (p_event.x != 0.0f);
        } break;
        case ActionEventType::Axis2D: {
            entry.x = p_event.x;
            entry.y = p_event.y;
            entry.down = (p_event.x != 0.0f || p_event.y != 0.0f);
        } break;
    }
}

bool ActionState::IsPressed(int p_player, const StringId& p_str_id) const {
    if (auto entry = Find(p_player, p_str_id)) {
        return entry->down;
    }
    return false;
}

bool ActionState::IsJustPressed(int p_player, const StringId& p_str_id) const {
    if (auto entry = Find(p_player, p_str_id)) {
        return entry->just_pressed;
    }
    return false;
}

bool ActionState::IsJustReleased(int p_player, const StringId& p_str_id) const {
    if (auto entry = Find(p_player, p_str_id)) {
        return entry->just_released;
    }
    return false;
}

float ActionState::GetStrength(int p_player, const StringId& p_str_id) const {
    if (auto entry = Find(p_player, p_str_id)) {
        return entry->x;
    }
    return 0.0f;
}

std::pair<float, float> ActionState::GetVector(int p_player,
                                               const StringId& p_neg_x,
                                               const StringId& p_pos_x,
                                               const StringId& p_neg_y,
                                               const StringId& p_pos_y) const {
    float x = GetStrength(p_player, p_pos_x) - GetStrength(p_player, p_neg_x);
    float y = GetStrength(p_player, p_pos_y) - GetStrength(p_player, p_neg_y);
    return { x, y };
}

const ActionStateEntry* ActionState::Find(int p_player, const StringId& p_str_id) const {
    auto it_player = m_state.find(p_player);
    if (it_player == m_state.end()) {
        return nullptr;
    }

    auto it_action = it_player->second.find(p_str_id);
    if (it_action == it_player->second.end()) {
        return nullptr;
    }
    return &it_action->second;
}

}  // namespace cave
