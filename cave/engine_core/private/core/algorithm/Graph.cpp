#include "cave/core/algorithm/Graph.h"

namespace cave {

Option<Vector<int>> TopologicalSort(int node_count,
                                    std::span<const TopoSortEdge> edges) {
    Vector<int> indegree(node_count, 0);
    Vector<Vector<int>> adj(node_count);

    for (const auto& [from, to] : edges) {
        adj[from].push_back(to);
        indegree[to] += 1;
    }

    Vector<int> ready;
    Vector<int> next_ready;
    for (int i = 0; i < static_cast<int>(indegree.size()); ++i) {
        if (indegree[i] == 0) ready.push_back(i);
    }

    Vector<int> sorted;
    sorted.reserve(node_count);

    while (!ready.empty()) {
        next_ready.clear();
        for (int node : ready) {
            sorted.push_back(node);
            for (int to : adj[node]) {
                if (--indegree[to] == 0) {
                    next_ready.push_back(to);
                }
            }
        }
        std::swap(ready, next_ready);
    }

    if (sorted.size() != static_cast<size_t>(node_count)) {
        return None();
    }

    return Some(std::move(sorted));
}

}  // namespace cave
