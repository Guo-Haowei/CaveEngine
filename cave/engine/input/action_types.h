#pragma once

namespace cave {

using ActionId = uint32_t;

enum class ActionEventType : uint8_t {
    Pressed,
    Released,
    Value,  // axis/analog value updates
};

struct ActionEvent {
    std::string action; // @TODO: use StringId to convert to uint64_t
    ActionEventType type{};
    int player_index = 0;
    float v0 = 0.0f;
    float v1 = 0.0f;
    uint64_t timestamp_us = 0;
};


}  // namespace cave
