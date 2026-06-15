#pragma once
#include "cave/core/base/Singleton.h"

namespace cave {

class VFS : public Singleton<VFS> {
public:
    void Mount(std::string p_mount_name, std::filesystem::path p_root);

    void Unmount(std::string_view p_mount_name);

    void Clear();

    bool HasMount(std::string_view p_mount_name) const;

    std::filesystem::path GetMount(std::string_view p_mount_name) const;

    std::string Resolve(std::string_view p_mount_name,
                        std::filesystem::path p_path) const;

    std::string Resolve(std::string_view p_path) const;

private:
    std::unordered_map<std::string, std::filesystem::path> m_mounts;
};

}  // namespace cave
