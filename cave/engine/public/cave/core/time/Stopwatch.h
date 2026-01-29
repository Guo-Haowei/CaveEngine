// =============================================================================
// File: public/cave/core/time/Stopwatch.h
// =============================================================================
#pragma once
#include "Clock.h"

namespace cave {

template<typename T, typename ClockPolicy>
class StopwatchBase {
public:
    void Start() {
        m_start = ClockPolicy::Now();
        m_running = true;
    }

    void Stop() {
        if (m_running) {
            m_elapsed += ClockPolicy::Now() - m_start;
            m_running = false;
        }
    }

    void Reset() {
        m_elapsed = T();
        m_running = false;
    }

    T Elapsed() const {
        if (!m_running) {
            return m_elapsed;
        }

        return m_elapsed + (ClockPolicy::Now() - m_start);
    }

    // Reset + Start, and return the previous total elapsed.
    T Restart() {
        const T elapsed = Elapsed();
        m_start = ClockPolicy::Now();
        m_elapsed = T{};
        m_running = true;
        return elapsed;
    }

    const T& StartPoint() const { return m_start; }
    bool IsRunning() const { return m_running; }

protected:
    T m_start{};
    T m_elapsed{};
    bool m_running{ false };
};

using Stopwatch = StopwatchBase<Nanoseconds, Clock>;

}  // namespace cave
