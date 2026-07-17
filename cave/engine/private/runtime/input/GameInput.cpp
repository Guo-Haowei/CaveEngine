#include "GameInput.h"

namespace cave {

GameInput::GameInput()
    : mapper_(input_action_map_) {
}

void GameInput::initialize() {
    InputActionMap& map = input_action_map_;

    // @TODO: read bindings from config file
    constexpr auto ui_accept = CAVE_SID("ui_accept");
    constexpr auto ui_back = CAVE_SID("ui_back");
    constexpr auto ui_left = CAVE_SID("ui_left");
    constexpr auto ui_right = CAVE_SID("ui_right");
    constexpr auto ui_up = CAVE_SID("ui_up");
    constexpr auto ui_down = CAVE_SID("ui_down");

    map.addAction(ui_accept, ActionValueType::Digital);
    map.bindDigital(ui_accept, Key::Enter);
    map.bindDigital(ui_accept, Key::PadA);
    map.bindDigital(ui_accept, Key::LMB);

    map.addAction(ui_back, ActionValueType::Digital);
    map.bindDigital(ui_back, Key::Backspace);
    map.bindDigital(ui_back, Key::PadB);
    map.bindDigital(ui_back, Key::RMB);

    map.addAction(ui_left, ActionValueType::Digital);
    map.bindDigital(ui_left, Key::A);
    map.bindDigital(ui_left, Key::Left);
    map.bindDigital(ui_left, Key::PadLeft);

    map.addAction(ui_right, ActionValueType::Digital);
    map.bindDigital(ui_right, Key::D);
    map.bindDigital(ui_right, Key::Right);
    map.bindDigital(ui_right, Key::PadRight);

    map.addAction(ui_up, ActionValueType::Digital);
    map.bindDigital(ui_up, Key::W);
    map.bindDigital(ui_up, Key::Up);
    map.bindDigital(ui_up, Key::PadUp);

    map.addAction(ui_down, ActionValueType::Digital);
    map.bindDigital(ui_down, Key::S);
    map.bindDigital(ui_down, Key::Down);
    map.bindDigital(ui_down, Key::PadDown);

    constexpr auto ui_axis_x = CAVE_SID("ui_axis_x");
    constexpr auto ui_axis_y = CAVE_SID("ui_axis_y");

    // Movement scalar axes
    map.addAction(ui_axis_x, ActionValueType::Scalar);
    map.addAction(ui_axis_y, ActionValueType::Scalar);

    // Gamepad axes contribute scalar too
    map.bindScalar(ui_axis_x, AxisCode::LX, 1.0f, 0.2f);
    map.bindScalar(ui_axis_y, AxisCode::LY, 1.0f, 0.2f, /*invert=*/true);
}

bool GameInput::isPressed(StringId action, int player) const {
    if (auto entry = findEntry(player, action)) {
        return entry->down;
    }
    return false;
}

bool GameInput::isJustPressed(StringId action, int player) const {
    if (auto entry = findEntry(player, action)) {
        return entry->just_pressed;
    }
    return false;
}

bool GameInput::isJustReleased(StringId action, int player) const {
    if (auto entry = findEntry(player, action)) {
        return entry->just_released;
    }
    return false;
}

float GameInput::getStrength(StringId action, int player) const {
    if (auto entry = findEntry(player, action)) {
        return entry->x;
    }
    return 0.0f;
}

auto GameInput::getVector(StringId neg_x,
                          StringId pos_x,
                          StringId neg_y,
                          StringId pos_y,
                          int player) const -> std::pair<float, float> {
    float x = getStrength(pos_x, player) - getStrength(neg_x, player);
    float y = getStrength(pos_y, player) - getStrength(neg_y, player);
    return { x, y };
}

const PointerState& GameInput::pointerState() const {
    DEV_ASSERT(pointers_ != nullptr);
    // @HACK: just return the first pointer for now
    return *pointers_;
}

void GameInput::updateActions(const std::vector<InputEvent>& input_events,
                              const KeyState& key_state,
                              const AxisState& axis_state,
                              const DeviceRouting& routing) {
    action_events_.clear();
    mapper_.map(input_events, key_state, axis_state, routing, action_events_);

    beginFrame();
    for (const auto& action : action_events_) {
        apply(action);
    }
}

void GameInput::beginFrame() {
    for (auto& [player, map] : entries_) {
        for (auto& [_, entry] : map) {
            entry.just_pressed = false;
            entry.just_released = false;
            entry.x = 0.0f;
            entry.y = 0.0f;
            // NOTE: a.down persists; it will be updated by events/axes.
        }
    }
}

void GameInput::apply(const ActionEvent& event) {
    auto& entry = entries_[event.player][event.action];

    switch (event.type) {
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
            entry.x = event.x;
            entry.down = (event.x != 0.0f);
        } break;
        case ActionEventType::Axis2D: {
            entry.x = event.x;
            entry.y = event.y;
            entry.down = (event.x != 0.0f || event.y != 0.0f);
        } break;
    }
}

const GameInput::Entry* GameInput::findEntry(int player, StringId action) const {
    auto it_player = entries_.find(player);
    if (it_player == entries_.end()) {
        return nullptr;
    }

    auto it_action = it_player->second.find(action);
    if (it_action == it_player->second.end()) {
        return nullptr;
    }
    return &it_action->second;
}

}  // namespace cave
