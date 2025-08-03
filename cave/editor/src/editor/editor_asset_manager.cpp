#include "editor_asset_manager.h"

#include "engine/assets/image_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/debugger/profiler.h"
#include "engine/runtime/application.h"
#include "engine/runtime/graphics_manager_interface.h"

// @TODO: refactor
#include "engine/drivers/windows/win32_prerequisites.h"

#include "editor/utility/content_entry.h"

namespace cave {

namespace fs = std::filesystem;

class FileWatcher {
public:
    void Start(const std::string& path);
    void Stop();
    bool HasChanged() const;
    void ClearFlag();

private:
    void WatchLoop();

    std::string m_path;
    std::thread m_thread;
    std::atomic<bool> m_stop = false;
    std::atomic<bool> m_changed = false;
    HANDLE m_dir_handle = INVALID_HANDLE_VALUE;
};

void FileWatcher::Start(const std::string& path) {
    m_path = path;
    m_stop = false;

    m_thread = std::thread([this]() {
        WatchLoop();
    });
}

void FileWatcher::Stop() {
    m_stop = true;
    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        CancelIoEx(m_dir_handle, nullptr);  // this safely breaks ReadDirectoryChangesW
    }

    if (m_thread.joinable()) {
        m_thread.join();  // make sure WatchLoop exits
    }

    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_dir_handle);  // now it's safe
        m_dir_handle = INVALID_HANDLE_VALUE;
    }
}

bool FileWatcher::HasChanged() const {
    return m_changed.load();
}

void FileWatcher::ClearFlag() {
    m_changed.store(false);
}

void FileWatcher::WatchLoop() {
    std::wstring path(m_path.begin(), m_path.end());

    m_dir_handle = CreateFileW(
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

[[nodiscard]] static auto CreateImageAsset(const AssetMetaData& p_meta) -> Result<std::shared_ptr<ImageAsset>> {
    auto image = std::make_shared<ImageAsset>();
    if (auto res = image->LoadFromDisk(p_meta); !res) {
        return CAVE_ERROR(res.error());
    }

    return image;
}

EditorAssetManager::EditorAssetManager() = default;

EditorAssetManager::~EditorAssetManager() = default;

Result<void> EditorAssetManager::InitializeImpl() {
    if (auto res = AssetManager::InitializeImpl(); !res) {
        return std::unexpected(res.error());
    }

    RebuildAssetFolderTree();

    m_file_watcher = std::make_unique<FileWatcher>();
    m_file_watcher->Start(m_asset_root_path.string());

    return AddAlwaysLoadImages();
}

void EditorAssetManager::FinalizeImpl() {
    m_file_watcher->Stop();

    AssetManager::FinalizeImpl();
}

void EditorAssetManager::Update() {
    if (m_file_watcher->HasChanged()) {
        RebuildAssetFolderTree();
        m_file_watcher->ClearFlag();
    }
}

static void BuildFolderLut(const ContentEntry* p_node,
                           std::unordered_map<std::string, const ContentEntry*>& p_lut) {
    p_lut[p_node->sys_path.string()] = p_node;
    for (const auto& child : p_node->children) {
        BuildFolderLut(child.get(), p_lut);
    }
}

void EditorAssetManager::RebuildAssetFolderTree() {
    CAVE_PROFILE_EVENT("Build folder tree");
    const std::string& path = m_app->GetResourceFolder();
    m_asset_root = BuildFolderTree(std::filesystem::path(path), nullptr);

    m_folder_lut.clear();
    BuildFolderLut(m_asset_root.get(), m_folder_lut);
}

Result<void> EditorAssetManager::AddAlwaysLoadImages() {
    // @TODO: fix this path, it won't work if the file is moved
    std::string_view tmp = StringUtils::BasePath(__FILE__);
    tmp = StringUtils::BasePath(tmp);
    tmp = StringUtils::BasePath(tmp);
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
            m_app->GetGraphicsManager()->RequestTexture(image.get());
        }
    }

    return Result<void>();
}
std::shared_ptr<ImageAsset> EditorAssetManager::FindImage(const std::string& p_name) {
    auto it = m_images.find(p_name);
    if (it == m_images.end()) {
        return nullptr;
    }
    return it->second;
}

}  // namespace cave
