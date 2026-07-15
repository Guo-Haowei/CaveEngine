// =============================================================================
// File: cave/core/algorithm/Graph.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/CoreExport.h"
#include "cave/core/Option.h"
#include "cave/core/containers/Containers.h"

namespace cave {

using TopoSortEdge = std::pair<int, int>;

CAVE_CORE_API Option<Vector<int>> TopologicalSort(int node_count,
                                                  std::span<const TopoSortEdge> edges);

}  // namespace cave
