#include "cave/runtime/core/time/Clock.h"

#include <chrono>

namespace cave {

Nanoseconds Clock::Now() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return Nanoseconds(ns);
}

}  // namespace cave
