#pragma once
#include "engine/assets/guid.h"
#include "engine/runtime/framework/TaskManager.h"

namespace cave {

class IAssetManager;
class AssetRegistry;

class BootLoadPipeline {
public:
    BootLoadPipeline(TaskManager& p_task_manager,
                     IAssetManager& p_asset_manager,
                     AssetRegistry& p_asset_registry);

    auto RequestProject(const std::filesystem::path& p_project_path) -> Result<void>;

    TaskSnapshot RootSnapshot() const { return m_task_manager.GetSnapshot(m_root); }

    std::vector<TaskSnapshot> ChildSnapshots() const;

    uint64_t RootId() const { return m_root; }

private:
    TaskManager& m_task_manager;
    IAssetManager& m_asset_manager;
    AssetRegistry& m_asset_registry;

    uint64_t m_root{ kInvalidTaskId };
    std::vector<uint64_t> m_children{};
};

}  // namespace cave
