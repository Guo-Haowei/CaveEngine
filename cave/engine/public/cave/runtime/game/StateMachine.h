// =============================================================================
// File: cave/runtime/game/StateMachine.h
// =============================================================================
#pragma once
#include <array>
#include <functional>

#include "cave/core/error/ErrorMacros.h"

namespace cave {

struct SceneContext;

template<typename T>
class GameStateMachine {
public:
    struct Callbacks {
        std::function<void(SceneContext&, float)> update;
        std::function<void(SceneContext&)> onEnter;
        std::function<void(SceneContext&)> onExit;
    };

    void addState(T state, Callbacks&& callbacks) {
        DEV_ASSERT_INDEX(state, T::Count);
        m_states[std::to_underlying(state)] = std::move(callbacks);
    }

    void switchTo(SceneContext& ctx, T state) {
        DEV_ASSERT_INDEX(state, T::Count);
        if (state == m_current) {
            return;
        }

        if (initialized()) {
            auto& exit_func = m_states[std::to_underlying(m_current)].onExit;
            if (exit_func) {
                exit_func(ctx);
            }
        }

        m_prev = m_current;
        m_current = state;
        m_state_time = 0.0f;

        auto& enter_func = m_states[std::to_underlying(state)].onEnter;
        if (enter_func) {
            enter_func(ctx);
        }
    }

    void update(SceneContext& ctx, float dt) {
        m_state_time += dt;
        auto& update_func = m_states[std::to_underlying(m_current)].update;
        if (update_func) {
            update_func(ctx, dt);
        }
    }

    bool initialized() const { return m_current != T::Invalid; }
    T prev() const { return m_prev; }
    T current() const { return m_current; }

    bool is(T state) const { return m_current == state; }

    float stateTime() const {
        return m_state_time;
    }

private:
    T m_current{ T::Invalid };
    T m_prev{ T::Invalid };

    float m_state_time = 0.0f;
    std::array<Callbacks, std::to_underlying(T::Count)> m_states;
};

}  // namespace cave
