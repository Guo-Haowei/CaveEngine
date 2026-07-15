#include "BootLoadPipeline.h"

#include "cave/core/string/StringUtils.h"
#include "cave/core/algorithm/Graph.h"

#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

BootLoadPipeline::BootLoadPipeline(TaskManager& task_manager,
                                   IAssetManager& asset_manager,
                                   AssetRegistry& asset_registry)
    : task_manager_(task_manager)
    , asset_manager_(asset_manager)
    , asset_registry_(asset_registry) {}

std::vector<TaskSnapshot> BootLoadPipeline::childSnapshots() const {
    std::vector<TaskSnapshot> out;
    out.reserve(children_.size());
    for (auto id : children_) {
        out.push_back(task_manager_.GetSnapshot(id));
    }
    return out;
}

auto BootLoadPipeline::requestProject(const std::filesystem::path& project_path) -> Result<void> {
    namespace fs = std::filesystem;

    DEV_ASSERT(!project_path.empty());

    struct Pair {
        bool has_meta;
        bool has_source;
    };

    std::unordered_map<std::string, Pair> resources;

    // go through all files, create meta if not exists
    for (const auto& entry : fs::recursive_directory_iterator(project_path)) {
        if (entry.is_regular_file()) {
            std::string virtual_path = asset_manager_.resolvePath(entry.path());

            auto ext = StringUtils::extension(virtual_path);
            if (ext == ".meta") {
                virtual_path.resize(virtual_path.size() - 5);  // remove '.meta'
                resources[virtual_path].has_meta = true;
            } else {
                resources[virtual_path].has_source = true;
            }
        }
    }

    std::vector<AssetMetaData> assets;
    assets.reserve(resources.size());

    for (const auto& [key, value] : resources) {
        auto meta_path = std::format("{}.meta", key);
        if (value.has_meta) {
            auto res = AssetMetaData::loadMeta(meta_path);
            if (!res) {
                return CAVE_ERROR(res.error());
            }

            auto meta = std::move(*res);

            if (meta.import_path != key) {
                LOG_WARN("path of asset '{}' is outdated expect: '{}', actual: '{}'", meta.guid.toString(), meta.import_path, key);
                meta.import_path = key;
            }

            assets.emplace_back(std::move(meta));
            continue;
        }

        DEV_ASSERT(value.has_source);
        auto meta = AssetMetaData::createMeta(key);
        if (meta.is_none()) {
            // LOG_WARN("file '{}' not supported", key);
            continue;
        }

        auto meta2 = std::move(meta.unwrap_unchecked());
        auto res = meta2.saveToDisk(nullptr);
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        LOG_TRACE("'{}' not detected, creating", meta_path);
        assets.emplace_back(std::move(meta2));
    }

    const int N = static_cast<int>(assets.size());
    Vector<TopoSortEdge> edges;
    HashMap<Guid, int> mapping;
    for (int i = 0; i < N; ++i) {
        mapping[assets[i].guid] = i;
    }

    for (const auto& asset : assets) {
        auto to = mapping.find(asset.guid);
        DEV_ASSERT(to != mapping.end());
        for (const auto& guid : asset.dependencies) {
            auto from = mapping.find(guid);
            if (from == mapping.end()) {
                if (!guid.isNull()) {
                    LOG_WARN(LogChannel::Asset, "Asset '{}' not found", guid.toString());
                    continue;
                }
            }
            edges.push_back({ from->second, to->second });
        }
    }

    const auto order = TopologicalSort(N, edges).unwrap();
    for (int idx : order) {
        children_.push_back(asset_registry_.startAsyncLoad(std::move(assets[idx])));
    }

    TaskGroupSpec group;
    group.name = "Boot";
    group.children = children_;

    root_task_id_ = task_manager_.SubmitGroup(std::move(group), TaskPriority::High);

    asset_registry_.refreshAllDependencies();
    return Result<void>();
}

}  // namespace cave
