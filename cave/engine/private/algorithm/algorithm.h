#pragma once

namespace cave {

using TopoSortEdge = std::pair<int, int>;

Option<std::vector<int>> TopologicalSort(int N, const std::vector<TopoSortEdge>& p_edges);

}  // namespace cave
