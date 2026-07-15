#include "AssetManager.h"

#include <filesystem>
#include <fstream>

#include "cave/core/threading/Threads.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/ecs/components/MiscComponents.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/assets/AssetImporter.h"
#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/assets/PrefabAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IAsyncTask.h"
#include "engine/private/runtime/framework/TaskContext.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;
using AssetCreateFunc = AssetRef (*)(void);

namespace {

void InitializeDefault(Scene& scene) {
    auto root = scene.createEntity();
    scene.create(HierarchyComponent_Id, root);
    scene.create(TransformComponent_Id, root);
    scene.create<NameComponent>(root).setName("root");
    scene.setRoot(root);
}

AssetRef CreateAssetInstance(AssetType type, bool create) {
    // @TODO: refactor this part
    switch (type) {
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
        case AssetType::Scene: {
            auto asset = std::make_shared<SceneAsset>();
            if (create && asset) {
                InitializeDefault(asset->sceneMut());
            }
            return asset;
        }
        case AssetType::Prefab: {
            auto asset = std::make_shared<PrefabAsset>();
            if (create && asset) {
                InitializeDefault(asset->sceneMut());
            }
            return asset;
        }
        default:
            return nullptr;
    }
}

auto LoadAsset(const Ref<AssetEntry>& entry) -> Result<AssetRef> {
    AssetRef asset = CreateAssetInstance(entry->metadata.type, false);
    if (!asset) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
    }

    if (auto res = asset->loadFromDisk(entry->metadata); !res) {
        return CAVE_ERROR(res.error());
    }
    return asset;
}

}  // namespace

auto AssetManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void AssetManager::FinalizeImpl() {}

Result<Guid> AssetManager::createAsset(AssetType type,
                                       const std::string& short_path) {
    AssetRef asset = CreateAssetInstance(type, true);
    DEV_ASSERT(asset);
    if (!asset) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create instance '{}'", short_path);
    }

    auto meta_opt = AssetMetaData::createMeta(short_path);
    if (meta_opt.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create meta '{}'", short_path);
    }

    auto meta = std::move(meta_opt.unwrap_unchecked());
    if (auto res = asset->saveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    Guid guid = meta.guid;
    services().assetRegistry().startAsyncLoad(std::move(meta));
    return guid;
}

Result<Guid> AssetManager::createAsset(AssetType type,
                                       const fs::path& folder,
                                       const char* name) {

    // 1. Creates both meta and file
    fs::path fullpath = folder;
    const char* ext = EnumTraits<AssetType>::ToString(type).data();
    fullpath = fullpath /
               std::format("{}_{}.{}", name ? name : "untitled", ++counter_, ext);

    std::string meta_file = fullpath.string();
    meta_file.append(".meta");

    auto short_path = resolvePath(fullpath);

    return createAsset(type, short_path);
}

Result<void> AssetManager::renameAssetOrFolder(const fs::path& old_path,
                                               const fs::path& new_path) {
    if (fs::is_directory(old_path)) {
        try {
            fs::rename(old_path, new_path);
        } catch (const fs::filesystem_error& e) {
            return CAVE_ERROR(ErrorCode::ERR_FILE_NO_PERMISSION, "{}", e.what());
        }
        return Result<void>();
    }

    auto meta_path_str = std::format("{}.meta", old_path.string());
    fs::path old_meta{ meta_path_str };

    meta_path_str = std::format("{}.meta", new_path.string());
    fs::path new_meta{ meta_path_str };

    try {
        fs::rename(old_meta, new_meta);
        fs::rename(old_path, new_path);
    } catch (const fs::filesystem_error& e) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_NO_PERMISSION, "{}", e.what());
    }

    AssetRegistry& reg = services().assetRegistry();
    std::string old_meta_path = resolvePath(old_path);
    std::string new_meta_path = resolvePath(new_path);

    reg.moveAsset(std::move(old_meta_path), new_meta_path);

    AssetHandle handle = reg.findByPath(new_meta_path).unwrap();
    AssetMetaData* meta = handle.meta();
    meta->name = new_path.filename().string();
    meta->import_path = new_meta_path;
    reg.saveAsset(meta->guid);

    return Result<void>();
}

std::string AssetManager::resolvePath(const fs::path& path) {
    return services().VFS().Resolve("@res", path);
}

uint64_t AssetManager::submitLoadAsset(const AssetLoadRequest& request) {
    class LoadAssetTask final : public IAsyncTask {
    public:
        LoadAssetTask(AssetManager& asset_manager,
                      const Guid& guid)
            : asset_manager_(asset_manager)
            , guid_(guid) {
        }

        const char* Name() const final {
            return "LoadFileTask";
        }

        void Run(TaskContext& ctx) final {
            ctx.SetIndeterminate(false);
            ctx.SetProgress(0.0f);

            AssetRef asset = asset_manager_.loadAssetSync(guid_);
            if (!asset) {
                ctx.Fail(std::format("LoadAssetSync failed for '{}'", guid_.toString()));
                return;
            }

            ctx.SetProgress(1.0f);
        }

    private:
        AssetManager& asset_manager_;
        Guid guid_;
    };

    TaskSubmitOptions opt;
    opt.priority = TaskPriority::Normal;
    opt.start_immediately = true;

    return services().taskManager().submit(MakeOwner<LoadAssetTask>(*this, request.guid),
                                           opt);
}

uint64_t AssetManager::submitImportScene(const SceneImportRequest& request) {
    class ImportAssetTask final : public IAsyncTask {
    public:
        ImportAssetTask(AssetManager& asset_manager,
                        const fs::path& source,
                        const fs::path& dest)
            : asset_manager_(asset_manager)
            , source_(source)
            , dest_(dest) {
        }

        const char* Name() const final {
            return "ImportAssetTask";
        }

        void Run(TaskContext& ctx) final {
            ctx.SetIndeterminate(false);
            ctx.SetProgress(0.0f);

            auto loader = AssetImporter::Create(source_, dest_);

            if (!loader) {
                ctx.Fail(std::format("No suitable loader found for asset '{}'", source_.string()));
                return;
            }

            auto res = loader->Import();

            if (!res) {
                ctx.Fail(std::format("Failed to load '{}', reason: {}", source_.string(), ToString(res.error())));
                return;
            }

            ctx.SetProgress(1.0f);
        }

    private:
        AssetManager& asset_manager_;
        fs::path source_;
        fs::path dest_;
    };

    return services().taskManager().submit(
        std::make_unique<ImportAssetTask>(*this,
                                          request.source_path,
                                          request.dest_dir),
        TaskSubmitOptions{ .priority = TaskPriority::Normal,
                           .start_immediately = true },
        [](uint64_t, TaskSnapshot snapshot) {
            if (snapshot.status == TaskStatus::Succeeded) {
                // @TODO: handle result
            }
        });
}

AssetRef AssetManager::loadAssetSync(const Guid& guid) {
    DEV_ASSERT(thread::GetThreadId() != thread::THREAD_MAIN);
    auto asset = loadAssetSyncHelper(guid);

    auto& device = services().renderDevice();

    // @TODO: based on render, create asset on work threads
    DEV_ASSERT(asset);
    switch (asset->type()) {
        case AssetType::Image: {
            auto image = std::dynamic_pointer_cast<ImageAsset>(asset);
            device.RequestTexture(image.get());
        } break;
        case AssetType::Mesh: {
            auto mesh = std::dynamic_pointer_cast<MeshAsset>(asset);
            device.RequestMesh(mesh.get());
        } break;
        default:
            break;
    }

    return asset;
}

void AssetManager::reloadAsset(const Guid& guid) {
    loadAssetSyncHelper(guid);
}

AssetRef AssetManager::loadAssetSyncHelper(const Guid& guid) {
    Stopwatch stopwatch;
    stopwatch.Start();
    auto entry = services().assetRegistry().entry(guid);

    auto res = LoadAsset(entry);
    if (!res) {
        entry->markFailed();
        LOG_ERROR("Failed to load asset '{}', reason {}",
                  entry->metadata.import_path,
                  ToString(res.error()));
        return nullptr;
    }

    AssetRef asset = *res;

    stopwatch.Stop();
    entry->markLoaded(asset);
    ++entry->revision;

    // @TODO: emit event?
    LOG_TRACE(LogChannel::Asset,
              "Asset '{}' loaded. revision={} ({})",
              entry->metadata.import_path,
              entry->revision,
              stopwatch.Elapsed().ToString());
    return asset;
}

EngineServices& AssetManager::services() {
    return m_app->services();
}

}  // namespace cave
