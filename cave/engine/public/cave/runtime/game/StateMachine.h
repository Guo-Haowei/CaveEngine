// =============================================================================
// File: cave/runtime/game/StateMachine.h
// =============================================================================
#pragma once
#include <array>
#include <functional>

#include "cave/core/error/ErrorMacros.h"

namespace cave {

template<typename T>
class GameStateMachine {
public:
    struct StateContext {
        std::function<void(float)> update;
        std::function<void()> on_enter;
        std::function<void()> on_exit;
        float duration = 0.0f;
        T next = T::Invalid;
    };

    void addState(T state, StateContext&& callbacks) {
        DEV_ASSERT_INDEX(state, T::Count);
        m_states[std::to_underlying(state)] = std::move(callbacks);
    }

    void switchTo(T state) {
        DEV_ASSERT_INDEX(state, T::Count);
        if (state == m_current) {
            return;
        }

        if (initialized()) {
            auto& exit_func = m_states[std::to_underlying(m_current)].on_exit;
            if (exit_func) {
                exit_func();
            }
        }

        m_prev = m_current;
        m_current = state;
        m_state_time = 0.0f;

        auto& enter_func = m_states[std::to_underlying(state)].on_enter;
        if (enter_func) {
            enter_func();
        }
    }

    void update(float dt) {
        m_state_time += dt;
        auto& context = m_states[std::to_underlying(m_current)];
        if (context.update) {
            context.update(dt);
        }
        if (context.duration > 0.0f && m_state_time >= context.duration) {
            switchTo(context.next);
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
    std::array<StateContext, std::to_underlying(T::Count)> m_states;
};

}  // namespace cave
