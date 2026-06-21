#include "AssetManager.h"

#include <filesystem>
#include <fstream>

#include "cave/core/threading/Threads.h"
#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/ecs/components/NameComponent.h"
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
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IAsyncTask.h"
#include "engine/private/runtime/framework/TaskContext.h"
#include "engine/private/runtime/framework/TaskManager.h"
#include "engine/private/runtime/framework/VFS.h"
#include "engine/private/runtime/scene/Scene.h"

#include "modules/tinygltf/tiny_gltf_importer.h"

#if USING(PLATFORM_WINDOWS) && defined(CAVE_BUILD_ASSIMP)
#define USE_IMPORTER_ASSIMP NOT_IN_USE
#else
#define USE_IMPORTER_ASSIMP NOT_IN_USE
#endif

#if USING(USE_IMPORTER_ASSIMP)
#include "modules/assimp/assimp_importer.h"
#endif

namespace cave {

namespace fs = std::filesystem;
using AssetCreateFunc = AssetRef (*)(void);

EngineServices& AssetManager::services() {
    return m_app->services();
}

static AssetRef CreateAssetInstance(AssetType type, bool create) {
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
            auto scene = std::make_shared<Scene>("");
            if (create) {
                auto root = scene->createEntity();
                scene->create(TransformComponent_Id, root);
                scene->create<NameComponent>(root);

                auto ent = scene->createEntity();
                scene->create(TransformComponent_Id, ent);

                scene->m_root = root;
                scene->attachChild(ent);
            }
            return scene;
        }
        default:
            return nullptr;
    }
}

static auto LoadAsset(const std::shared_ptr<AssetEntry>& entry) -> Result<AssetRef> {
    AssetRef asset = CreateAssetInstance(entry->metadata.type, false);
    if (!asset) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
    }

    if (auto res = asset->LoadFromDisk(entry->metadata); !res) {
        return CAVE_ERROR(res.error());
    }
    return asset;
}

auto AssetManager::InitializeImpl() -> Result<void> {
#if USING(USE_IMPORTER_TINYGLTF)
    AssetImporter::RegisterImporter(".gltf", TinyGltfImporter::CreateImporter);
    AssetImporter::RegisterImporter(".glb", TinyGltfImporter::CreateImporter);
#endif

#if USING(USE_IMPORTER_ASSIMP)
    AssetImporter::RegisterImporter(".obj", AssimpImporter::CreateImporter);
    AssetImporter::RegisterImporter(".fbx", AssimpImporter::CreateImporter);
#endif

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

    auto _meta = AssetMetaData::CreateMeta(short_path);
    if (_meta.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create meta '{}'", short_path);
    }

    auto meta = std::move(_meta.unwrap_unchecked());
    if (auto res = asset->SaveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    Guid guid = meta.guid;
    services().assetRegistry().StartAsyncLoad(std::move(meta));
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

Result<void> AssetManager::moveAsset(const fs::path& old_path,
                                     const fs::path& new_path) {
    DEV_ASSERT(!fs::is_directory(old_path));

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

    services().assetRegistry().MoveAsset(resolvePath(old_path), resolvePath(new_path));
    return Result<void>();
}

std::string AssetManager::resolvePath(const fs::path& path) {
    return services().vfs().Resolve("@res", path);
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
                ctx.Fail(std::format("LoadAssetSync failed for '{}'", guid_.ToString()));
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

    return services().taskManager().submit(std::make_unique<LoadAssetTask>(*this, request.guid),
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

    Stopwatch stopwatch;
    stopwatch.Start();
    auto entry = services().assetRegistry().GetEntry(guid);

    auto res = LoadAsset(entry);
    if (!res) {
        entry->MarkFailed();
        LOG_ERROR("Failed to load asset '{}', reason {}",
                  entry->metadata.import_path,
                  ToString(res.error()));
        return nullptr;
    }

    AssetRef asset = *res;
    auto& device = services().renderDevice();

    // @TODO: based on render, create asset on work threads
    DEV_ASSERT(asset);
    switch (asset->GetType()) {
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

    stopwatch.Stop();
    LOG_TRACE(LogChannel::Asset, "Loaded {} {}", entry->metadata.import_path, stopwatch.Elapsed().ToString());
    entry->MarkLoaded(asset);
    return asset;
}

}  // namespace cave
