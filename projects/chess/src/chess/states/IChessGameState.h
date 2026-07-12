#pragma once
#include "cave/core/typedefs.h"

// clang-format off
namespace cave { class IntentBus; }
namespace cave { class SceneRuntime; }
// clang-format on

namespace chess {

class ChessGameMode;

class IChessGameState {
public:
    IChessGameState(cave::SceneRuntime& runtime,
                    cave::IntentBus& intent_bus) noexcept
        : m_runtime(runtime)
        , m_intent_bus(intent_bus) {}

    virtual ~IChessGameState() = default;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void tick(float) = 0;

#if USING(DEBUG_BUILD)
    virtual const char* debugName() const = 0;
#endif

protected:
    cave::SceneRuntime& m_runtime;
    cave::IntentBus& m_intent_bus;
};

}  // namespace chess
