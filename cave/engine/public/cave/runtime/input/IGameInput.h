// =============================================================================
// File: public/cave/runtime/input/IGameInput.h
// =============================================================================
#pragma once
#include <utility>

#include "cave/core/string/StringId.h"

namespace cave {

class IGameInput {
public:
    virtual ~IGameInput() = default;

    virtual bool isPressed(StringId action, int player = 0) const = 0;

    virtual bool isJustPressed(StringId action, int player = 0) const = 0;

    virtual bool isJustReleased(StringId action, int player = 0) const = 0;

    virtual float getStrength(StringId action, int player = 0) const = 0;

    virtual auto getVector(StringId neg_x,
                           StringId pos_x,
                           StringId neg_y,
                           StringId pos_y,
                           int player = 0) const -> std::pair<float, float> = 0;
};

}  // namespace cave
