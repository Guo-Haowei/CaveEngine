#include "EditorAssetManager.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/VFS.h"

// @TODO: refactor
#include "engine/private/drivers/windows/win32_prerequisites.h"

#include "editor/utility/ContentEntry.h"

namespace cave {

namespace fs = std::filesystem;

class FileWatcher {
public:
    void start(const std::string& path);
    void stop();

    bool hasChanged() const { return changed_.load(); }

    void clearFlag() { changed_.store(false); }

    bool isStopped() const { return stop_; }

private:
    void watchLoop();

    std::string path_;
    std::thread thread_;
    std::atomic<bool> stop_{ true };
    std::atomic<bool> changed_{ true };  // set to true to trigger build the first frame
    HANDLE dir_handle_ = INVALID_HANDLE_VALUE;
};

void FileWatcher::start(const std::string& path) {
    path_ = path;
    stop_ = false;

    thread_ = std::thread([this]() {
        watchLoop();
    });
}

void FileWatcher::stop() {
    stop_ = true;
    if (dir_handle_ != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(dir_handle_, nullptr);
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    if (dir_handle_ != INVALID_HANDLE_VALUE) {
        ::CloseHandle(dir_handle_);
        dir_handle_ = INVALID_HANDLE_VALUE;
    }
}

void FileWatcher::watchLoop() {
    std::wstring path(path_.begin(), path_.end());

    dir_handle_ = ::CreateFileW(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (dir_handle_ == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open directory {}", path_);
        return;
    }

    constexpr DWORD bufferSize = 8192;
    BYTE buffer[bufferSize];
    DWORD bytesReturned;

    while (!stop_.load()) {
        BOOL success = ReadDirectoryChangesW(
            dir_handle_,
            buffer,
            bufferSize,
            TRUE,  // recursive
            FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            nullptr,
            nullptr);

        if (!success || stop_.load()) {
            break;
        }

        changed_.store(true);  // flag to main thread
    }
}

namespace {

auto CreateImageAsset(const AssetMetaData& meta) -> Result<std::shared_ptr<ImageAsset>> {
    auto image = std::make_shared<ImageAsset>();
    if (auto res = image->loadFromDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    return image;
}

}  // namespace

EditorAssetManager::EditorAssetManager() = default;

EditorAssetManager::~EditorAssetManager() = default;

Result<void> EditorAssetManager::InitializeImpl() {
    if (auto res = AssetManager::InitializeImpl(); !res) {
        return CAVE_ERROR(res.error());
    }

    file_watcher_ = std::make_unique<FileWatcher>();

    return addAlwaysLoadImages();
}

void EditorAssetManager::FinalizeImpl() {
    file_watcher_->stop();

    AssetManager::FinalizeImpl();
}

void EditorAssetManager::update() {
    if (resource_folder_.empty()) {
        resource_folder_ = m_app->services().vfs().GetMount("@res");
        if (resource_folder_.empty()) {
            return;
        }
    }

    if (file_watcher_->isStopped()) {
        file_watcher_->start(resource_folder_.string());
    }

    if (file_watcher_->hasChanged()) {
        refreshAssetFolderTree();
        file_watcher_->clearFlag();
    }
}

static void BuildFolderLut(const ContentEntry* p_node,
                           std::unordered_map<std::string, const ContentEntry*>& p_lut) {
    p_lut[p_node->sys_path.string()] = p_node;
    for (const auto& child : p_node->children) {
        BuildFolderLut(child.get(), p_lut);
    }
}

void EditorAssetManager::refreshAssetFolderTree() {
    CAVE_PROFILE_EVENT("Build folder tree");

    asset_root_ = BuildFolderTree(resource_folder_, nullptr);

    folder_lut_.clear();
    if (asset_root_) {
        BuildFolderLut(asset_root_.get(), folder_lut_);
    }
}

Result<void> EditorAssetManager::addAlwaysLoadImages() {
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
            images_[file_name.string()] = image;
            m_app->services().renderDevice().RequestTexture(image.get());
        }
    }

    return Result<void>();
}
std::shared_ptr<ImageAsset> EditorAssetManager::findImage(const std::string& p_name) {
    auto it = images_.find(p_name);
    if (it == images_.end()) {
        return nullptr;
    }
    return it->second;
}

void EditorAssetManager::onAssetSaved(const AssetChangedEvent& event) {
    AssetRegistry& asset_reg = m_app->services().assetRegistry();
    auto users = asset_reg.findReverseDependenciesTransitively(event.guid);
    for (const Guid& user : users) {
        LOG_WARN("asset '{}' is affected", user.toString());
    }
}

}  // namespace cave
