#include "asset_manager.h"

#include <filesystem>
#include <fstream>

#include "engine/assets/asset_importer.h"
#include "engine/assets/blob_asset.h"
#include "engine/assets/image_asset.h"
#include "engine/assets/material_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/assets/tile_set_asset.h"
#include "engine/assets/sprite_animation_asset.h"
#include "engine/assets/tile_map_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/core/os/threads.h"
#include "engine/core/os/timer.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "modules/tinygltf/tiny_gltf_importer.h"
#include "modules/assimp/assimp_importer.h"

namespace cave {

namespace fs = std::filesystem;
using AssetCreateFunc = AssetRef (*)(void);

enum class LoadTaskType : uint8_t {
    Load,
    Import,
};

struct AssetManager::LoadTask {
    LoadTaskType type;
    Guid guid;
    AssetLoadSuccessCallback on_success = nullptr;
    AssetLoadFailureCallback on_failure = nullptr;
    void* userdata;
    fs::path source;
    fs::path dest;
};

// @TODO: get rid of this?
static struct {
    // @TODO: better wake up
    std::condition_variable wake_condition;
    std::mutex wakeMutex;
    // @TODO: better thread safe queue
    ConcurrentQueue<AssetManager::LoadTask> job_queue;
    std::atomic_int runningWorkers;
} s_assetManagerGlob;

static AssetRef CreateAssetInstance(AssetType p_type) {
    // @TODO: [SCRUM-222] refactor this part
    switch (p_type) {
        case AssetType::Blob:
            return std::make_shared<BlobAsset>();
        case AssetType::Image:
            return std::make_shared<ImageAsset>();
        case AssetType::TileSet:
            return std::make_shared<TileSetAsset>();
        case AssetType::SpriteAnimation:
            return std::make_shared<SpriteAnimationAsset>();
        case AssetType::TileMap:
            return std::make_shared<TileMapAsset>();
        case AssetType::Material:
            return std::make_shared<MaterialAsset>();
        case AssetType::Mesh:
            return std::make_shared<MeshAsset>();
        case AssetType::Scene:
            return std::make_shared<Scene>();
        default:
            return nullptr;
    }
}

static auto LoadAsset(const std::shared_ptr<AssetEntry>& p_entry) -> Result<AssetRef> {
    AssetRef asset = CreateAssetInstance(p_entry->metadata.type);
    if (!asset) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
    }

    if (auto res = asset->LoadFromDisk(p_entry->metadata); !res) {
        return CAVE_ERROR(res.error());
    }
    return asset;
}

auto AssetManager::InitializeImpl() -> Result<void> {
    m_assets_root = fs::path{ m_app->GetResourceFolder() };

#if USING(USE_IMPORTER_TINYGLTF)
    AssetImporter::RegisterImporter(".gltf", TinyGltfImporter::CreateImporter);
#endif

#if USING(USE_IMPORTER_ASSIMP)
    AssetImporter::RegisterImporter(".obj", AssimpImporter::CreateImporter);
    AssetImporter::RegisterImporter(".fbx", AssimpImporter::CreateImporter);
#endif

    return Result<void>();
}

Result<Guid> AssetManager::CreateAsset(AssetType p_type,
                                       const std::string& p_short_path) {
    AssetRef asset = CreateAssetInstance(p_type);
    DEV_ASSERT(asset);
    if (!asset) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create instance '{}'", p_short_path);
    }

    auto _meta = AssetMetaData::CreateMeta(p_short_path);
    if (_meta.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create meta '{}'", p_short_path);
    }

    auto meta = std::move(_meta.unwrap_unchecked());
    if (auto res = asset->SaveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    Guid guid = meta.guid;
    m_app->GetAssetRegistry()->StartAsyncLoad(std::move(meta), nullptr, nullptr, nullptr);
    return guid;
}

Result<Guid> AssetManager::CreateAsset(AssetType p_type,
                                       const fs::path& p_folder,
                                       const char* p_name) {

    // 1. Creates both meta and file
    fs::path new_file = p_folder;
    const char* ext = EnumTraits<AssetType>::ToString(p_type).data();
    auto name = std::format("{}_{}.{}", p_name ? p_name : "untitled", ++m_fps_counter, ext);
    new_file = new_file / name;

    std::string meta_file = new_file.string();
    meta_file.append(".meta");

    auto short_path = ResolvePath(new_file);

    return CreateAsset(p_type, short_path);
}

Result<void> AssetManager::MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) {
    DEV_ASSERT(!fs::is_directory(p_old));

    auto meta_path_str = std::format("{}.meta", p_old.string());
    fs::path old_meta{ meta_path_str };

    meta_path_str = std::format("{}.meta", p_new.string());
    fs::path new_meta{ meta_path_str };

    auto old_path = ResolvePath(p_old);
    auto new_path = ResolvePath(p_new);
    try {
        fs::rename(old_meta, new_meta);
        fs::rename(p_old, p_new);
    } catch (const fs::filesystem_error& e) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_NO_PERMISSION, "{}", e.what());
    }

    m_app->GetAssetRegistry()->MoveAsset(std::move(old_path), std::move(new_path));
    return Result<void>();
}

std::string AssetManager::ResolvePath(const fs::path& p_path) {
    fs::path relative = fs::relative(p_path, m_assets_root);
    return std::format("@res://{}", relative.generic_string());
}

bool AssetManager::LoadAssetAsync(const Guid& p_guid,
                                  AssetLoadSuccessCallback&& p_on_success,
                                  AssetLoadFailureCallback&& p_on_failure,
                                  void* p_userdata) {
    LoadTask task;
    task.type = LoadTaskType::Load;
    task.guid = p_guid;
    task.on_success = std::move(p_on_success);
    task.on_failure = std::move(p_on_failure);
    task.userdata = p_userdata;
    return EnqueueLoadTask(task);
}

bool AssetManager::ImportSceneAsync(const std::filesystem::path& p_source_path,
                                    const std::filesystem::path& p_dest_dir) {
    LoadTask task;
    task.type = LoadTaskType::Import;
    task.source = p_source_path;
    task.dest = p_dest_dir;
    return EnqueueLoadTask(task);
}

AssetRef AssetManager::LoadAssetSync(const Guid& p_guid) {
    DEV_ASSERT(thread::GetThreadId() != thread::THREAD_MAIN);

    Timer timer;
    auto entry = m_app->GetAssetRegistry()->GetEntry(p_guid);

    auto res = LoadAsset(entry);
    if (!res) {
        entry->MarkFailed();
        LOG_ERROR("Failed to load asset '{}', reason {}", entry->metadata.import_path, ToString(res.error()));
        return nullptr;
    }

    AssetRef asset = *res;

    // @TODO: based on render, create asset on work threads
    DEV_ASSERT(asset);
    switch (asset->GetType()) {
        case AssetType::Image: {
            auto image = std::dynamic_pointer_cast<ImageAsset>(asset);
            m_app->GetGraphicsManager()->RequestTexture(image.get());
        } break;
        case AssetType::Mesh: {
            auto mesh = std::dynamic_pointer_cast<MeshAsset>(asset);
            m_app->GetGraphicsManager()->RequestMesh(mesh.get());
        } break;
        default:
            break;
    }

    LOG_VERBOSE("[AssetManager] asset '{}' loaded in {}", entry->metadata.import_path, timer.GetDurationString());
    entry->MarkLoaded(asset);
    return asset;
}

void AssetManager::ImportSceneSync(LoadTask&& p_task) {
    auto loader = AssetImporter::Create(p_task.source, std::move(p_task.dest));

    if (!loader) {
        LOG_ERROR("No suitable loader found for asset '{}'", p_task.source.string());
        return;
    }

    auto res = loader->Import();

    if (!res) {
        LOG_ERROR("Failed to load '{}', reason: {}", p_task.source.string(), ToString(res.error()));
        return;
    }

    return *res;
}

void AssetManager::FinalizeImpl() {
    RequestShutdown();
}

bool AssetManager::EnqueueLoadTask(LoadTask& p_task) {
    s_assetManagerGlob.job_queue.push(std::move(p_task));
    s_assetManagerGlob.wake_condition.notify_one();
    return true;
}

void AssetManager::WorkerMain() {
    for (;;) {
        if (thread::ShutdownRequested()) {
            break;
        }

        LoadTask task;
        if (!s_assetManagerGlob.job_queue.pop(task)) {
            std::unique_lock<std::mutex> lock(s_assetManagerGlob.wakeMutex);
            s_assetManagerGlob.wake_condition.wait(lock);
            continue;
        }

        s_assetManagerGlob.runningWorkers.fetch_add(1);

        AssetManager& asset_manager = static_cast<AssetManager&>(IAssetManager::GetSingleton());
        switch (task.type) {
            case LoadTaskType::Load: {
                auto asset = asset_manager.LoadAssetSync(task.guid);
                if (asset) {
                    task.on_success ? task.on_success(asset, task.userdata) : (void)0;
                } else {
                    task.on_failure ? task.on_failure(task.userdata) : (void)0;
                }
            } break;
            case LoadTaskType::Import: {
                asset_manager.ImportSceneSync(std::move(task));
            } break;
        }

        s_assetManagerGlob.runningWorkers.fetch_sub(1);
    }
}

void AssetManager::RequestShutdown() {
    s_assetManagerGlob.wake_condition.notify_all();
}

}  // namespace cave
