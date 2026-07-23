#include "EditorAssetManager.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/VFS.h"

#if USING(PLATFORM_WINDOWS) && defined(CAVE_BUILD_ASSIMP)
#define USE_IMPORTER_ASSIMP NOT_IN_USE
#else
#define USE_IMPORTER_ASSIMP NOT_IN_USE
#endif

#if USING(USE_IMPORTER_ASSIMP)
#include "modules/assimp/assimp_importer.h"
#endif
#include "modules/tinygltf/tiny_gltf_importer.h"

// @TODO: refactor
#include "engine/private/drivers/windows/win32_prerequisites.h"

#include "editor/services/Workspace.h"
#include "editor/utility/ContentEntry.h"

namespace cave {

namespace fs = std::filesystem;

class FileWatcher {
public:
    void start(const std::string& path);
    void stop();

    bool hasChanged() const { return m_changed.load(); }

    void clearFlag() { m_changed.store(false); }

    bool isStopped() const { return m_stop; }

private:
    void watchLoop();

    std::string m_path;
    std::thread m_thread;
    std::atomic<bool> m_stop{ true };
    std::atomic<bool> m_changed{ true };  // set to true to trigger build the first frame
    HANDLE m_dir_handle = INVALID_HANDLE_VALUE;
};

void FileWatcher::start(const std::string& path) {
    m_path = path;
    m_stop = false;

    m_thread = std::thread([this]() {
        watchLoop();
    });
}

void FileWatcher::stop() {
    m_stop = true;
    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(m_dir_handle, nullptr);
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }

    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_dir_handle);
        m_dir_handle = INVALID_HANDLE_VALUE;
    }
}

void FileWatcher::watchLoop() {
    std::wstring path(m_path.begin(), m_path.end());

    m_dir_handle = ::CreateFileW(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (m_dir_handle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open directory {}", m_path);
        return;
    }

    constexpr DWORD bufferSize = 8192;
    BYTE buffer[bufferSize];
    DWORD bytesReturned;

    while (!m_stop.load()) {
        BOOL success = ReadDirectoryChangesW(
            m_dir_handle,
            buffer,
            bufferSize,
            TRUE,  // recursive
            FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            nullptr,
            nullptr);

        if (!success || m_stop.load()) {
            break;
        }

        m_changed.store(true);  // flag to main thread
    }
}

namespace {

auto CreateImageAsset(const AssetMetaData& meta) -> Result<Ref<ImageAsset>> {
    auto image = MakeRef<ImageAsset>();
    if (auto res = image->loadFromDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    return image;
}

void BuildFolderLut(const ContentEntry* node,
                    StringHashMap<const ContentEntry*>& lut) {
    lut[node->sys_path.string()] = node;
    for (const auto& child : node->children) {
        BuildFolderLut(child.get(), lut);
    }
}

}  // namespace

EditorAssetManager::EditorAssetManager() = default;

EditorAssetManager::~EditorAssetManager() = default;

Result<void> EditorAssetManager::InitializeImpl() {
    if (auto res = AssetManager::InitializeImpl(); !res) {
        return CAVE_ERROR(res.error());
    }

    // @TODO: use DLL
#if USING(USE_IMPORTER_TINYGLTF)
    AssetImporter::RegisterImporter(".gltf", TinyGltfImporter::CreateImporter);
    AssetImporter::RegisterImporter(".glb", TinyGltfImporter::CreateImporter);
#endif

#if USING(USE_IMPORTER_ASSIMP)
    AssetImporter::RegisterImporter(".obj", AssimpImporter::CreateImporter);
    AssetImporter::RegisterImporter(".fbx", AssimpImporter::CreateImporter);
#endif

    m_file_watcher = MakeOwner<FileWatcher>();

    return addAlwaysLoadImages();
}

void EditorAssetManager::FinalizeImpl() {
    m_file_watcher->stop();

    AssetManager::FinalizeImpl();
}

void EditorAssetManager::update() {
    if (m_resource_folder.empty()) {
        m_resource_folder = m_app->services().VFS().GetMount("@res");
        if (m_resource_folder.empty()) {
            return;
        }
    }

    if (m_file_watcher->isStopped()) {
        m_file_watcher->start(m_resource_folder.string());
    }

    if (m_file_watcher->hasChanged()) {
        refreshAssetFolderTree();
        m_file_watcher->clearFlag();
    }
}

void EditorAssetManager::refreshAssetFolderTree() {
    CAVE_PROFILE_EVENT("Build folder tree");

    m_asset_root = BuildFolderTree(m_resource_folder, nullptr);

    m_folder_lut.clear();
    if (m_asset_root) {
        BuildFolderLut(m_asset_root.get(), m_folder_lut);
    }
}

Result<void> EditorAssetManager::addAlwaysLoadImages() {
    // @TODO: fix this path, it won't work if the file is moved
    std::string_view tmp = StringUtils::basePath(__FILE__);
    tmp = StringUtils::basePath(tmp);
    tmp = StringUtils::basePath(tmp);
    fs::path image_folder = tmp;
    image_folder = image_folder / "resources" / "images";
    DEV_ASSERT(fs::is_directory(image_folder));
    for (const auto& entry : fs::directory_iterator(image_folder)) {
        if (entry.is_regular_file()) {
            fs::path path = entry.path();
            fs::path file_name = path.filename();
            AssetMetaData meta;
            meta.import_path = path.string();

            auto res = CreateImageAsset(meta);
            if (!res) {
                return CAVE_ERROR(res.error());
            }
            auto image = *res;
            m_images[file_name.string()] = image;
            m_app->services().renderDevice().RequestTexture(image.get());
        }
    }

    return Result<void>();
}

Ref<ImageAsset> EditorAssetManager::findImage(std::string_view name) {
    auto it = m_images.find(name);
    return it == m_images.end() ? nullptr : it->second;
}

void EditorAssetManager::onAssetSaved(const AssetChangedEvent& event) {
    AssetRegistry& asset_reg = m_app->services().assetRegistry();
    auto affected = asset_reg.findReverseDependenciesTransitively(event.guid);
    for (const Guid& guid : affected) {
        reloadAsset(guid);
    }

    m_editor_services->workspace().onAssetChanged(event.guid, affected);
}

}  // namespace cave
