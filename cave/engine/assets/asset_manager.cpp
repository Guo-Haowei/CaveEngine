#include "asset_manager.h"

#include <filesystem>
#include <fstream>

#include "engine/assets/asset_loader.h"
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

#if USING(PLATFORM_WINDOWS)
#define USE_IMPORTER_TINYGLTF IN_USE
#define USE_IMPORTER_ASSIMP   IN_USE
#elif USING(PLATFORM_APPLE)
#define USE_IMPORTER_TINYGLTF IN_USE
#define USE_IMPORTER_ASSIMP   NOT_IN_USE
#elif USING(PLATFORM_WASM)
#define USE_IMPORTER_TINYGLTF NOT_IN_USE
#define USE_IMPORTER_ASSIMP   NOT_IN_USE
#else
#error "Platform not supported"
#endif

#if USING(USE_IMPORTER_TINYGLTF)
#include "modules/tinygltf/tinygltf_loader.h"
#endif
#if USING(USE_IMPORTER_ASSIMP)
#include "modules/assimp/importer_assimp.h"
#endif

namespace cave {

namespace fs = std::filesystem;
using AssetCreateFunc = AssetRef (*)(void);

struct AssetManager::LoadTask {
    Guid guid;
    AssetLoadSuccessCallback on_success;
    AssetLoadFailureCallback on_failure;
    void* userdata;
};

// @TODO: get rid of this?
static struct {
    // @TODO: better wake up
    std::condition_variable wakeCondition;
    std::mutex wakeMutex;
    // @TODO: better thread safe queue
    ConcurrentQueue<AssetManager::LoadTask> jobQueue;
    std::atomic_int runningWorkers;
} s_assetManagerGlob;

static AssetRef CreateAssetInstance(AssetType p_type) {
    // @TODO: [SCRUM-222] refactor this part
    switch (p_type) {
        case AssetType::Image:
            return std::make_shared<ImageAsset>();
        case AssetType::Scene:
            return std::make_shared<Scene>();
        case AssetType::TileSet:
            return std::make_shared<TileSetAsset>();
        case AssetType::SpriteAnimation:
            return std::make_shared<SpriteAnimationAsset>();
        case AssetType::TileMap:
            return std::make_shared<TileMapAsset>();
        case AssetType::Material:
            return std::make_shared<MaterialAsset>();
        default:
            return nullptr;
    }
}

static auto LoadAsset(const std::shared_ptr<AssetEntry>& p_entry) -> Result<AssetRef> {
    AssetRef asset = CreateAssetInstance(p_entry->metadata.type);
    if (!asset) {
        return nullptr;  // not an error
    }

    if (auto res = asset->LoadFromDisk(p_entry->metadata); !res) {
        return CAVE_ERROR(res.error());
    }
    return asset;
}

auto AssetManager::InitializeImpl() -> Result<void> {
    m_assets_root = fs::path{ m_app->GetResourceFolder() };

#if USING(USE_IMPORTER_TINYGLTF)
    // IAssetLoader::RegisterLoader(".gltf", TinyGLTFLoader::CreateLoader);
#elif USING(USE_ASSIMP_LOADER)
    IAssetLoader::RegisterLoader(".gltf", ImporterAssimp::CreateLoader);
#endif

#if USING(USING_ASSIMP)
    IAssetLoader::RegisterLoader(".obj", ImporterAssimp::CreateLoader);
#endif

    IAssetLoader::RegisterLoader(".lua", BufferAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".ttf", BufferAssetLoader::CreateLoader);

#if 0
    IAssetLoader::RegisterLoader(".h", BufferAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".hlsl", BufferAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".glsl", BufferAssetLoader::CreateLoader);
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

AssetRef AssetManager::LoadAssetSync(const Guid& p_guid) {
    DEV_ASSERT(thread::GetThreadId() != thread::THREAD_MAIN);

    Timer timer;
    auto entry = m_app->GetAssetRegistry()->GetEntry(p_guid);

    AssetRef asset;

    do {
        // @TODO: slowly replace all the logic
        // @TODO: change loader to importer

        {
            auto res = LoadAsset(entry);
            if (!res) {
                entry->MarkFailed();
                LOG_ERROR("Failed to load asset '{}', reason {}", entry->metadata.import_path, ToString(res.error()));
                return nullptr;
            }

            asset = *res;
        }

        if (asset) {
            break;
        }

        auto loader = IAssetLoader::Create(entry->metadata.import_path);
        if (!loader) {
            LOG_ERROR("No suitable loader found for asset '{}'", entry->metadata.import_path);
            entry->MarkFailed();
            break;
        }

        auto res = loader->Load();
        if (!res) {
            LOG_ERROR("Failed to load '{}'", entry->metadata.import_path);
            entry->MarkFailed();
            return nullptr;
        }

        asset = *res;

    } while (0);

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

bool AssetManager::LoadAssetAsync(const Guid& p_guid,
                                  AssetLoadSuccessCallback&& p_on_success,
                                  AssetLoadFailureCallback&& p_on_failure,
                                  void* p_userdata) {
    // @TODO: check queue full
    LoadTask task;
    task.guid = p_guid;
    task.on_success = std::move(p_on_success);
    task.on_failure = std::move(p_on_failure);
    task.userdata = p_userdata;
    EnqueueLoadTask(task);
    return true;
}

void AssetManager::FinalizeImpl() {
    RequestShutdown();
}

void AssetManager::EnqueueLoadTask(LoadTask& p_task) {
    s_assetManagerGlob.jobQueue.push(std::move(p_task));
    s_assetManagerGlob.wakeCondition.notify_one();
}

void AssetManager::WorkerMain() {
    for (;;) {
        if (thread::ShutdownRequested()) {
            break;
        }

        LoadTask task;
        if (!s_assetManagerGlob.jobQueue.pop(task)) {
            std::unique_lock<std::mutex> lock(s_assetManagerGlob.wakeMutex);
            s_assetManagerGlob.wakeCondition.wait(lock);
            continue;
        }

        s_assetManagerGlob.runningWorkers.fetch_add(1);

        AssetManager& asset_manager = static_cast<AssetManager&>(IAssetManager::GetSingleton());
        auto asset = asset_manager.LoadAssetSync(task.guid);
        if (asset) {
            task.on_success ? task.on_success(asset, task.userdata) : (void)0;
        } else {
            task.on_failure ? task.on_failure(task.userdata) : (void)0;
        }

        s_assetManagerGlob.runningWorkers.fetch_sub(1);
    }
}

void AssetManager::RequestShutdown() {
    s_assetManagerGlob.wakeCondition.notify_all();
}

}  // namespace cave
