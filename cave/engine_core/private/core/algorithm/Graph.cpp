#include "cave/core/algorithm/Graph.h"
#include "cave/core/error/ErrorMacros.h"

#include <deque>

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

Vector<int> FindConnectedTiles(std::span<const uint16_t> tiles,
                               int width,
                               int height,
                               int tile_idx) {
    DEV_ASSERT(tiles.size() == width * height);
    DEV_ASSERT(tile_idx < tiles.size());

    const auto tile_value = tiles[tile_idx];

    Vector<int> result{ tile_idx };
    std::deque<int> ready{ tile_idx };

    Vector<bool> visited(tiles.size(), false);
    visited[tile_idx] = true;

    auto check_tile = [&](int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return;
        }

        const int idx = width * y + x;
        if (visited[idx]) {
            return;
        }

        visited[idx] = true;
        if (tiles[idx] == tile_value) {
            ready.push_back(idx);
            result.push_back(idx);
        }
    };

    while (!ready.empty()) {
        const int idx = ready.front();
        ready.pop_front();

        const int x = idx % width;
        const int y = idx / width;

        check_tile(x - 1, y);
        check_tile(x + 1, y);
        check_tile(x, y - 1);
        check_tile(x, y + 1);
    }

    return result;
}

}  // namespace cave
