#pragma once
#include "cave/core/typedefs.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { struct FrameTime; }
// clang-format on

namespace chess {

class ChessGameMode;

class IChessGameState {
public:
    virtual ~IChessGameState() = default;

    virtual void OnEnter(cave::IHostServices&) {}
    virtual void OnExit(cave::IHostServices&) {}

    virtual void Tick(cave::IHostServices&, const cave::FrameTime&) = 0;

#if USING(DEBUG_BUILD)
    virtual const char* DebugName() const = 0;
#endif
};

}  // namespace chess
