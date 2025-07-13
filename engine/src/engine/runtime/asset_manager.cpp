#include "asset_manager.h"

#include <filesystem>
#include <fstream>

// @TODO: get rid of this file
#include "engine/assets/assets.h"
#include "engine/assets/sprite_sheet_asset.h"
#include "engine/assets/asset_loader.h"
#include "engine/core/io/file_access.h"
#include "engine/core/os/threads.h"
#include "engine/core/os/timer.h"
#include "engine/core/string/string_builder.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"

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

namespace my {

namespace fs = std::filesystem;
using AssetCreateFunc = AssetRef (*)(void);

struct AssetManager::LoadTask {
    Guid guid;
    OnAssetLoadSuccessFunc on_success;
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

    AssetCreateFunc createFuncs[AssetType::Count];
} s_assetManagerGlob;

// @TODO: use DeserializerRegistry first,
// if no suitable, use importer

// @TODO: implement

AssetRef CreateAssetInstance(AssetType p_type) {
    if (p_type != AssetType::SpriteSheet) {
        return nullptr;
    }

    auto sprite = std::make_shared<SpriteSheetAsset>();
    return std::static_pointer_cast<IAsset>(sprite);
}

AssetRef LoadAsset(const std::shared_ptr<AssetEntry>& p_entry) {
    AssetRef asset = CreateAssetInstance(p_entry->metadata.type);
    if (!asset) {
        return nullptr;
    }

    asset->m_entry = p_entry;
    asset->LoadFromDisk(p_entry->metadata);
    return asset;
}

auto AssetManager::InitializeImpl() -> Result<void> {
    m_assets_root = fs::path{ m_app->GetResourceFolder() };

    IAssetLoader::RegisterLoader(".scene", SceneLoader::CreateLoader);
    IAssetLoader::RegisterLoader(".yaml", TextSceneLoader::CreateLoader);

#if USING(USE_TINYGLTF_LOADER)
    IAssetLoader::RegisterLoader(".gltf", TinyGLTFLoader::CreateLoader);
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

    //
    s_assetManagerGlob.createFuncs[AssetType::SpriteSheet] = []() -> AssetRef {
        auto sprite = std::make_shared<SpriteSheetAsset>();
        return std::static_pointer_cast<IAsset>(sprite);
    };

    return Result<void>();
}

void AssetManager::CreateAsset(const AssetType& p_type,
                               const fs::path& p_folder,
                               const char* p_name) {
    // @TODO: change this to serialize once done
    CRASH_NOW_MSG("Move logic to IAsset::SaveToDisk");

    DEV_ASSERT(p_type == AssetType::SpriteSheet);
    // 1. Creates both meta and file
    fs::path new_file = p_folder;
    if (p_name) {
        new_file = new_file / p_name;
    } else {
        // @TODO: extension
        new_file = new_file / std::format("untitled{}.sprite", ++m_counter);
    }

    DEV_ASSERT(s_assetManagerGlob.createFuncs[p_type.GetData()]);
    AssetRef asset = s_assetManagerGlob.createFuncs[p_type.GetData()]();

    if (fs::exists(new_file)) {
        fs::remove(new_file);
    }
    std::ofstream file(new_file);
    // asset->SaveToDisk();

    std::string meta_file = new_file.string();
    meta_file.append(".meta");

    auto short_path = ResolvePath(new_file);
    auto _meta = AssetMetaData::CreateMeta(short_path);
    DEV_ASSERT(_meta);
    if (!_meta) {
        return;
    }
    auto meta = std::move(_meta.value());

    if (fs::exists(meta_file)) {
        fs::remove(meta_file);
    }

    // auto res = meta.SaveMeta(meta_file);
    // if (!res) {
    //     return;
    // }

    // 2. Update AssetRegistry when done
    m_app->GetAssetRegistry()->StartAsyncLoad(std::move(meta), nullptr, nullptr);
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
        return HBN_ERROR(ErrorCode::ERR_FILE_NO_PERMISSION, "{}", e.what());
    }

    m_app->GetAssetRegistry()->MoveAsset(std::move(old_path), std::move(new_path));
    return Result<void>();
}

std::string AssetManager::ResolvePath(const fs::path& p_path) {
    fs::path relative = fs::relative(p_path, m_assets_root);
    return std::format("@res://{}", relative.generic_string());
}

void AssetManager::LoadAssetSync(const Guid& p_guid,
                                 OnAssetLoadSuccessFunc p_on_success,
                                 void* p_userdata) {
    DEV_ASSERT(thread::GetThreadId() != thread::THREAD_MAIN);

    Timer timer;
    auto entry = m_app->GetAssetRegistry()->GetEntry(p_guid);

    AssetRef asset;

    do {
        // @TODO: gradully replace all the logic
        // @TODO: change loader to importer
        asset = LoadAsset(entry);
        if (asset) {
            break;
        }

        auto loader = IAssetLoader::Create(entry->metadata);
        if (!loader) {
            LOG_ERROR("No suitable loader found for asset '{}'", entry->metadata.path);
            entry->MarkFailed();
            return;
        }

        auto res = loader->Load();
        if (!res) {
            LOG_ERROR("Failed to load '{}'", entry->metadata.path);
            entry->MarkFailed();
            return;
        }

        asset = *res;

    } while (0);

    DEV_ASSERT(asset);
    if (asset->type == AssetType::Image) {
        auto image = std::dynamic_pointer_cast<ImageAsset>(asset);

        // @TODO: based on render, create asset on work threads
        m_app->GetGraphicsManager()->RequestTexture(image.get());
    }

    if (p_on_success) {
        p_on_success(asset, p_userdata);
    }
    LOG_VERBOSE("[AssetManager] asset '{}' loaded in {}", entry->metadata.path, timer.GetDurationString());

    entry->MarkLoaded(asset);
}

void AssetManager::LoadAssetAsync(const Guid& p_guid, OnAssetLoadSuccessFunc p_on_success, void* p_userdata) {
    LoadTask task;
    task.guid = p_guid;
    task.on_success = p_on_success;
    task.userdata = p_userdata;
    EnqueueLoadTask(task);
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

        GetSingleton().LoadAssetSync(task.guid, task.on_success, task.userdata);

        s_assetManagerGlob.runningWorkers.fetch_sub(1);
    }
}

void AssetManager::RequestShutdown() {
    s_assetManagerGlob.wakeCondition.notify_all();
}

}  // namespace my
