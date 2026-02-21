#pragma once
#include <cstdint>

using PlayerId = uint8_t;

class IPlayerAgent {
public:
    virtual ~IPlayerAgent() = default;

    virtual void Tick() = 0;

    virtual PlayerId GetPlayer() const = 0;
};