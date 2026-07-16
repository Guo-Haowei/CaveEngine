// =============================================================================
// File: cave/core/algorithm/Graph.h
// =============================================================================
#pragma once
#include <cstdint>
#include <span>

#include "cave/core/CoreExport.h"
#include "cave/core/Option.h"
#include "cave/core/containers/Containers.h"

namespace cave {

using TopoSortEdge = std::pair<int, int>;

CAVE_CORE_API
auto TopologicalSort(int node_count,
                     std::span<const TopoSortEdge> edges) -> Option<Vector<int>>;

CAVE_CORE_API
Vector<int> FindConnectedTiles(std::span<const uint16_t> tiles,
                               int width,
                               int height,
                               int tile_idx);

}  // namespace cave
