// =============================================================================
// File: cave/core/time/Clock.h
// =============================================================================
#pragma once
#include "Nanoseconds.h"

namespace cave {

class Clock {
public:
    // steady, never goes backward
    static Nanoseconds Now();

private:
    Clock() = delete;  // pure static class
};

}  // namespace cave
