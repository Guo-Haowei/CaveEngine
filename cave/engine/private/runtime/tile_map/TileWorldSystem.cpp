#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

namespace cave {

TileWorldSystem::TileWorldSystem()
    : debug_id_(MakeDebugId(this)) {}

void TileWorldSystem::onAttach() {

}

void TileWorldSystem::onDetach() {

}

}  // namespace cave
