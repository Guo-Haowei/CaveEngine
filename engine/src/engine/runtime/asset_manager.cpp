#include "asset_manager.h"

#include <filesystem>
#include <fstream>

// @TODO: get rid of this file
#include "engine/assets/assets.h"
#include "engine/assets/asset_loader.h"
#include "engine/assets/image_asset.h"
#include "engine/assets/sprite_asset.h"  // @TODO: deprecate sprite asset
#include "engine/sprite/sprite_animation_asset.h"
#include "engine/tile_map/tile_map_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/core/os/threads.h"
#include "engine/core/os/timer.h"
#include "engine/core/string/string_builder.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#if USING(PLATFORM_WINDOWS)
#define USE_TINYGLTF_LOADER IN_USE
#define USE_ASSIMP_LOADER   IN_USE
#elif USING(PLATFORM_APPLE)
#define USE_TINYGLTF_LOADER IN_USE
#define USE_ASSIMP_LOADER   NOT_IN_USE
#elif USING(PLATFORM_WASM)
#define USE_TINYGLTF_LOADER NOT_IN_USE
#define USE_ASSIMP_LOADER   NOT_IN_USE
#else
#error "Platform not supported"
#endif

#if USING(USE_TINYGLTF_LOADER)
#include "modules/tinygltf/tinygltf_loader.h"
#endif

// plugins
#include "plugins/loader_assimp/assimp_asset_loader.h"

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
    switch (p_type) {
        case AssetType::Scene: {
            auto scene = std::make_shared<Scene>();
            return scene;
        }
        case AssetType::Sprite:
            return std::make_shared<SpriteAsset>();
        case AssetType::SpriteAnimation:
            return std::make_shared<SpriteAnimationAsset>();
        case AssetType::TileMap:
            return std::make_shared<TileMapAsset>();
        default:
            return nullptr;
    }
}

static auto LoadAsset(const std::shared_ptr<AssetEntry>& p_entry) -> Result<AssetRef> {
    AssetRef asset = CreateAssetInstance(p_entry->metadata.type);
    if (!asset) {
        return nullptr;  // not an error
    }

    asset->m_entry = p_entry;
    if (auto res = asset->LoadFromDisk(p_entry->metadata); !res) {
        return CAVE_ERROR(res.error());
    }
    return asset;
}

auto AssetManager::InitializeImpl() -> Result<void> {
    m_assets_root = fs::path{ m_app->GetResourceFolder() };

#if USING(USE_TINYGLTF_LOADER)
    // IAssetLoader::RegisterLoader(".gltf", TinyGLTFLoader::CreateLoader);
#elif USING(USE_ASSIMP_LOADER)
    IAssetLoader::RegisterLoader(".gltf", AssimpAssetLoader::CreateLoader);
#endif

#if USING(USING_ASSIMP)
    IAssetLoader::RegisterLoader(".obj", AssimpAssetLoader::CreateLoader);
#endif

    IAssetLoader::RegisterLoader(".lua", TextAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".ttf", BufferAssetLoader::CreateLoader);

    IAssetLoader::RegisterLoader(".h", BufferAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".hlsl", BufferAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".glsl", BufferAssetLoader::CreateLoader);

    IAssetLoader::RegisterLoader(".png", ImageAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".jpg", ImageAssetLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".hdr", ImageAssetLoader::CreateLoaderF);

    return Result<void>();
}

void AssetManager::CreateAsset(AssetType p_type,
                               const fs::path& p_folder,
                               const char* p_name) {
    AssetRef asset = CreateAssetInstance(p_type);
    DEV_ASSERT(asset);
    if (!asset) {
        return;
    }

    // 1. Creates both meta and file
    fs::path new_file = p_folder;
    const char* ext = ToString(p_type);
    auto name = std::format("{}_{}.{}", p_name ? p_name : "untitled", ++m_counter, ext);
    new_file = new_file / name;

    std::string meta_file = new_file.string();
    meta_file.append(".meta");

    auto short_path = ResolvePath(new_file);
    auto _meta = AssetMetaData::CreateMeta(short_path);
    if (_meta.is_none()) {
        LOG_ERROR("can't create asset '{}'", short_path);
        return;
    }

    auto meta = std::move(_meta.unwrap_unchecked());
    // @TODO: handle error
    [[maybe_unused]] auto res = asset->SaveToDisk(meta);

    // 2. Update AssetRegistry when done
    m_app->GetAssetRegistry()->StartAsyncLoad(std::move(meta), nullptr, nullptr, nullptr);
}

auto AssetManager::MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) -> Result<void> {
    if (fs::is_directory(p_old)) {
        LOG_WARN("don't support moving folder yet");
        return Result<void>();
    }

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

                StringStreamBuilder builder;
                builder << res.error();
                LOG_ERROR("Failed to load asset '{}', reason {}", entry->metadata.path, builder.ToString());
                return nullptr;
            }

            asset = *res;
        }

        if (asset) {
            break;
        }

        auto loader = IAssetLoader::Create(entry->metadata);
        if (!loader) {
            LOG_ERROR("No suitable loader found for asset '{}'", entry->metadata.path);
            entry->MarkFailed();
            break;
        }

        auto res = loader->Load();
        if (!res) {
            LOG_ERROR("Failed to load '{}'", entry->metadata.path);
            entry->MarkFailed();
            return nullptr;
        }

        asset = *res;

    } while (0);

    DEV_ASSERT(asset);
    if (asset->type == AssetType::Image) {
        auto image = std::dynamic_pointer_cast<ImageAsset>(asset);

        // @TODO: based on render, create asset on work threads
        m_app->GetGraphicsManager()->RequestTexture(image.get());
    }
    LOG_VERBOSE("[AssetManager] asset '{}' loaded in {}", entry->metadata.path, timer.GetDurationString());

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

        auto asset = GetSingleton().LoadAssetSync(task.guid);
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
