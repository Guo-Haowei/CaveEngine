#include "AssetManager.h"

#include <filesystem>
#include <fstream>

#include "cave/core/time/Stopwatch.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/assets/AssetImporter.h"
#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/assets/TileMapAsset.h"
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

static AssetRef CreateAssetInstance(AssetType p_type) {
    // @TODO: refactor this part
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
            return std::make_shared<Scene>("");
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
    DEV_ASSERT(0 && "fix this part");
    // m_app->GetAssetRegistry()->StartAsyncLoad(std::move(meta));
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
    return m_app->GetVFS().Resolve("@res", p_path);
}

uint64_t AssetManager::SubmitLoadAsset(const AssetLoadRequest& p_request) {
    class LoadAssetTask final : public IAsyncTask {
    public:
        LoadAssetTask(AssetManager& p_asset_manager,
                      const Guid& p_guid)
            : m_asset_manager(p_asset_manager)
            , m_guid(p_guid) {
        }

        const char* Name() const final {
            return "LoadFileTask";
        }

        void Run(TaskContext& p_ctx) final {
            p_ctx.SetIndeterminate(false);
            p_ctx.SetProgress(0.0f);

            AssetRef asset = m_asset_manager.LoadAssetSync(m_guid);
            if (!asset) {
                p_ctx.Fail(std::format("LoadAssetSync failed for '{}'", m_guid.ToString()));
                return;
            }

            p_ctx.SetProgress(1.0f);
        }

    private:
        AssetManager& m_asset_manager;
        Guid m_guid;
    };

    TaskSubmitOptions opt;
    opt.priority = TaskPriority::Normal;
    opt.start_immediately = true;

    return m_app->GetTaskManager()->Submit(std::make_unique<LoadAssetTask>(*this, p_request.guid),
                                           opt);
}

uint64_t AssetManager::SubmitImportScene(const SceneImportRequest& p_request) {
    class ImportAssetTask final : public IAsyncTask {
    public:
        ImportAssetTask(AssetManager& p_asset_manager,
                        const fs::path& p_source,
                        const fs::path& p_dest)
            : m_asset_manager(p_asset_manager)
            , m_source(p_source)
            , m_dest(p_dest) {
        }

        const char* Name() const final {
            return "ImportAssetTask";
        }

        void Run(TaskContext& p_ctx) final {
            p_ctx.SetIndeterminate(false);
            p_ctx.SetProgress(0.0f);

            auto loader = AssetImporter::Create(m_source, m_dest);

            if (!loader) {
                p_ctx.Fail(std::format("No suitable loader found for asset '{}'", m_source.string()));
                return;
            }

            auto res = loader->Import();

            if (!res) {
                p_ctx.Fail(std::format("Failed to load '{}', reason: {}", m_source.string(), ToString(res.error())));
                return;
            }

            p_ctx.SetProgress(1.0f);
        }

    private:
        AssetManager& m_asset_manager;
        fs::path m_source;
        fs::path m_dest;
    };

    return m_app->GetTaskManager()->Submit(
        std::make_unique<ImportAssetTask>(*this,
                                          p_request.source_path,
                                          p_request.dest_dir),
        TaskSubmitOptions{ .priority = TaskPriority::Normal,
                           .start_immediately = true },
        [](uint64_t p_id, TaskSnapshot p_snapshot) {
            unused(p_id);
            if (p_snapshot.status == TaskStatus::Succeeded) {
                // @TODO: handle result
            }
        });
}

AssetRef AssetManager::LoadAssetSync(const Guid& p_guid) {
    DEV_ASSERT(thread::GetThreadId() != thread::THREAD_MAIN);

    Stopwatch stopwatch;
    stopwatch.Start();
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
            m_app->GetRenderDevice()->RequestTexture(image.get());
        } break;
        case AssetType::Mesh: {
            auto mesh = std::dynamic_pointer_cast<MeshAsset>(asset);
            m_app->GetRenderDevice()->RequestMesh(mesh.get());
        } break;
        default:
            break;
    }

    stopwatch.Stop();
    LOG_TRACE("'{}' loaded in {}", entry->metadata.import_path, stopwatch.Elapsed().ToString());
    entry->MarkLoaded(asset);
    return asset;
}

void AssetManager::FinalizeImpl() {
}

}  // namespace cave
