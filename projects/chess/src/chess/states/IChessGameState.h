#pragma once
#include "cave/core/typedefs.h"

// clang-format off
namespace cave { struct SceneContext; }
namespace cave { class IntentBus; }
// clang-format on

namespace chess {

class ChessGameMode;

class IChessGameState {
public:
    IChessGameState(cave::IntentBus& intent_bus)
        : m_intent_bus(intent_bus) {}

    virtual ~IChessGameState() = default;

    virtual void onEnter(cave::SceneContext&) {}
    virtual void onExit() {}

    virtual void tick(cave::SceneContext&, float) = 0;

#if USING(DEBUG_BUILD)
    virtual const char* debugName() const = 0;
#endif

protected:
    cave::IntentBus& m_intent_bus;
};

}  // namespace chess
