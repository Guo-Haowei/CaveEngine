#include "vfs.h"

namespace cave {

namespace fs = std::filesystem;

void VFS::Mount(std::string p_mount_name, std::filesystem::path p_root) {
    m_mounts[std::move(p_mount_name)] = std::move(p_root);
}

void VFS::Unmount(std::string_view p_mount_name) {
    m_mounts.erase(std::string(p_mount_name));
}

void VFS::Clear() {
    m_mounts.clear();
}

bool VFS::HasMount(std::string_view mountName) const {
    return m_mounts.find(std::string(mountName)) != m_mounts.end();
}

std::filesystem::path VFS::GetMount(std::string_view p_mount_name) const {
    auto it = m_mounts.find(std::string(p_mount_name));
    if (it == m_mounts.end()) {
        return {};
    }
    return it->second;
}

std::string VFS::Resolve(std::string_view p_mount_name,
                         std::filesystem::path p_path) const {
    auto it = m_mounts.find(std::string(p_mount_name));
    if (it == m_mounts.end()) {
        return "";
    }

    fs::path relative = fs::relative(p_path, it->second);
    return std::format("{}://{}", p_mount_name, relative.generic_string());
}

std::string VFS::Resolve(std::string_view p_path) const {
    DEV_ASSERT(!p_path.empty());
    if (p_path.front() != '@') {
        return std::string(p_path);
    }

    auto pos = p_path.find(':');
    if (pos == std::string::npos)
        return std::string(p_path);

    std::string mount_name(p_path.substr(0, pos));

    auto it = m_mounts.find(mount_name);
    if (it == m_mounts.end()) {
        return std::string(p_path);
    }

    // @HACK: assume all 
    std::string path(it->second.string());
    path.append(p_path, pos + 1, std::string::npos);
    return path;
}

}  // namespace cave
