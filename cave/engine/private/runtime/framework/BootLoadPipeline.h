#pragma once
#include "cave/core/ids/Guid.h"
#include "engine/private/runtime/framework/TaskManager.h"

namespace cave {

class IAssetManager;
class AssetRegistry;

class BootLoadPipeline {
public:
    BootLoadPipeline(TaskManager& task_manager,
                     IAssetManager& asset_manager,
                     AssetRegistry& asset_registry);

    auto requestProject(const std::filesystem::path& project_path) -> Result<void>;

    TaskSnapshot rootSnapshot() const { return task_manager_.GetSnapshot(root_task_id_); }

    std::vector<TaskSnapshot> childSnapshots() const;

    uint64_t rootTaskId() const { return root_task_id_; }

private:
    TaskManager& task_manager_;
    IAssetManager& asset_manager_;
    AssetRegistry& asset_registry_;

    uint64_t root_task_id_{ kInvalidTaskId };
    std::vector<uint64_t> children_{};
};

}  // namespace cave
