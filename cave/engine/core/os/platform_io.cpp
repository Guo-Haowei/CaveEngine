#include "platform_io.h"

#include "engine/core/string/string_utils.h"

#if USING(PLATFORM_WINDOWS)
#include "engine/drivers/windows/win32_prerequisites.h"
#include <shellapi.h>
#endif

namespace cave::os {
namespace fs = std::filesystem;

#if USING(PLATFORM_WINDOWS)

void RevealInFolder(const fs::path& p_path) {
    const bool is_dir = fs::is_directory(p_path);
    fs::path folder = is_dir ? p_path : p_path.parent_path();
    std::string path = folder.string();
    std::replace(path.begin(), path.end(), '/', '\\');
    ::ShellExecuteA(NULL, "open", "explorer.exe", path.c_str(), NULL, SW_SHOWNORMAL);
}

Option<std::string> OpenFileDialog(const std::vector<const char*>& p_filters) {
    std::string filter_str;
    if (p_filters.empty()) {
        filter_str = "*.*";
    } else {
        for (const auto& filter : p_filters) {
            filter_str.append(";*");
            filter_str.append(filter);
        }
        filter_str = filter_str.substr(1);
    }

    char buffer[1024] = { 0 };
    StringUtils::Sprintf(buffer, "Supported Files(%s)\n%s", filter_str.c_str(), filter_str.c_str());
    for (char* p = buffer; *p; ++p) {
        if (*p == '\n') {
            *p = '\0';
            break;
        }
    }

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    char file_buffer[260] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    // ofn.hwndOwner = DEV_ASSERT(0);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = file_buffer;
    ofn.nMaxFile = sizeof(file_buffer);
    ofn.lpstrFilter = buffer;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn)) {
        return Some(std::string(file_buffer));
    }

    return None();
}

bool OpenSaveDialog(std::filesystem::path& p_inout_path) {
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char file_name[MAX_PATH]{ 0 };
    char extension[MAX_PATH]{ 0 };
    char dir[MAX_PATH]{ 0 };
    StringUtils::Strcpy(file_name, p_inout_path.filename().replace_extension().string());
    StringUtils::Strcpy(dir, p_inout_path.parent_path().string());
    StringUtils::Strcpy(extension, p_inout_path.extension().string());

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    // ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = extension + 1;
    ofn.lpstrInitialDir = dir;

    if (GetSaveFileNameA(&ofn)) {
        p_inout_path = std::filesystem::path(ofn.lpstrFile);
        return true;
    }

    return false;
}
#elif USING(PLATFORM_WASM)
void RevealInFolder(const std::filesystem::path&) {
    CRASH_NOW_MSG("not supported");
}

std::string OpenFileDialog(const std::vector<const char*>&) {
    CRASH_NOW_MSG("not supported");
    return "";
}

bool OpenSaveDialog(std::filesystem::path&) {
    CRASH_NOW_MSG("not supported");
    return false;
}
#else
#error NOT IMPLEMENTED
#endif

}  // namespace cave::os
