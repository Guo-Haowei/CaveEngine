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

    virtual bool IsPressed(int p_player,
                           const StringId& p_action) const = 0;

    virtual bool IsJustPressed(int p_player,
                               const StringId& p_action) const = 0;

    virtual bool IsJustReleased(int p_player,
                                const StringId& p_action) const = 0;

    virtual float GetStrength(int p_player,
                              const StringId& p_action) const = 0;

    virtual std::pair<float, float> GetVector(int p_player,
                                              const StringId& p_neg_x,
                                              const StringId& p_pos_x,
                                              const StringId& p_neg_y,
                                              const StringId& p_pos_y) const = 0;
};

}  // namespace cave
