// =============================================================================
// File: cave/core/time/Stopwatch.h
// =============================================================================
#pragma once
#include "Clock.h"

namespace cave {

template<typename T, typename ClockPolicy>
class StopwatchBase {
public:
    void start() {
        m_start = ClockPolicy::Now();
        m_running = true;
    }

    void stop() {
        if (m_running) {
            m_elapsed += ClockPolicy::Now() - m_start;
            m_running = false;
        }
    }

    void reset() {
        m_elapsed = T();
        m_running = false;
    }

    T elapsed() const {
        if (!m_running) {
            return m_elapsed;
        }

        return m_elapsed + (ClockPolicy::Now() - m_start);
    }

    // Reset + Start, and return the previous total elapsed.
    T restart() {
        const T elapsed_time = elapsed();
        m_start = ClockPolicy::Now();
        m_elapsed = T{};
        m_running = true;
        return elapsed_time;
    }

    const T& startPoint() const { return m_start; }
    bool isRunning() const { return m_running; }

protected:
    T m_start{};
    T m_elapsed{};
    bool m_running{ false };
};

using Stopwatch = StopwatchBase<Nanoseconds, Clock>;

}  // namespace cave
