// =============================================================================
// File: cave/core/time/CountdownTimer.h
// =============================================================================
#pragma once
#include "cave/core/math/Scalar.h"

namespace cave {

class CountdownTimer {
public:
    CountdownTimer() noexcept = default;

    explicit CountdownTimer(float duration) noexcept
        : m_duration(duration)
        , m_remaining(0.0f) {}

    void start() {
        m_remaining = m_duration;
    }

    void start(float duration) {
        m_duration = duration;
        m_remaining = duration;
    }

    void stop() {
        m_remaining = 0.0f;
    }

    void tick(float dt) {
        m_remaining = cave::math::max(0.0f, m_remaining - dt);
    }

    bool active() const {
        return m_remaining > 0.0f;
    }

    bool finished() const {
        return m_remaining <= 0.0f;
    }

    float remaining() const {
        return m_remaining;
    }

    float duration() const {
        return m_duration;
    }

    float progress01() const {
        return m_duration > 0.0f
                   ? 1.0f - m_remaining / m_duration
                   : 1.0f;
    }

private:
    float m_duration = 0.0f;
    float m_remaining = 0.0f;
};

}  // namespace cave
