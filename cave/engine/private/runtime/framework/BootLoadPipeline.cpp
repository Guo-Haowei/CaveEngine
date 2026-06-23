#include "BootLoadPipeline.h"

#include "cave/core/string/StringUtils.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

BootLoadPipeline::BootLoadPipeline(TaskManager& p_task_manager,
                                   IAssetManager& p_asset_manager,
                                   AssetRegistry& p_asset_registry)
    : m_task_manager(p_task_manager)
    , m_asset_manager(p_asset_manager)
    , m_asset_registry(p_asset_registry) {}

std::vector<TaskSnapshot> BootLoadPipeline::ChildSnapshots() const {
    std::vector<TaskSnapshot> out;
    out.reserve(m_children.size());
    for (auto id : m_children) out.push_back(m_task_manager.GetSnapshot(id));
    return out;
}

auto BootLoadPipeline::RequestProject(const std::filesystem::path& p_project_path) -> Result<void> {
    namespace fs = std::filesystem;

    DEV_ASSERT(!p_project_path.empty());

    struct Pair {
        bool has_meta;
        bool has_source;
    };

    std::unordered_map<std::string, Pair> resources;

    // go through all files, create meta if not exists
    for (const auto& entry : fs::recursive_directory_iterator(p_project_path)) {
        if (entry.is_regular_file()) {
            std::string virtual_path = m_asset_manager.resolvePath(entry.path());

            auto ext = StringUtils::Extension(virtual_path);
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
            auto res = AssetMetaData::LoadMeta(meta_path);
            if (!res) {
                return CAVE_ERROR(res.error());
            }

            auto meta = std::move(*res);

            if (meta.import_path != key) {
                LOG_WARN("path of asset '{}' is outdated expect: '{}', actual: '{}'", meta.guid.ToString(), meta.import_path, key);
                meta.import_path = key;
            }

            assets.emplace_back(std::move(meta));
            continue;
        }

        DEV_ASSERT(value.has_source);
        auto meta = AssetMetaData::CreateMeta(key);
        if (meta.is_none()) {
            // LOG_WARN("file '{}' not supported", key);
            continue;
        }

        auto meta2 = std::move(meta.unwrap_unchecked());
        auto res = meta2.SaveToDisk(nullptr);
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        LOG_TRACE("'{}' not detected, creating", meta_path);
        assets.emplace_back(std::move(meta2));
    }

    const int N = static_cast<int>(assets.size());
    std::vector<TopoSortEdge> edges;
    std::unordered_map<Guid, int> mapping;
    for (int i = 0; i < N; ++i) {
        mapping[assets[i].guid] = i;
    }

    for (const auto& asset : assets) {
        auto to = mapping.find(asset.guid);
        DEV_ASSERT(to != mapping.end());
        for (const auto& guid : asset.dependencies) {
            auto from = mapping.find(guid);
            if (from == mapping.end()) {
                if (guid.IsNull()) continue;
                CRASH_NOW_MSG("dependency not found");
            }
            edges.push_back({ from->second, to->second });
        }
    }

    const auto order = TopologicalSort(N, edges).unwrap();
    for (int idx : order) {
        m_children.push_back(m_asset_registry.startAsyncLoad(std::move(assets[idx])));
    }

    TaskGroupSpec group;
    group.name = "Boot";
    group.children = m_children;

    m_root = m_task_manager.SubmitGroup(std::move(group), TaskPriority::High);

    return Result<void>();
}

}  // namespace cave
