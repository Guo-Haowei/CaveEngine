#include "file_access.h"

#include "engine/private/core/string/string_utils.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

FileAccess::CreateFunc FileAccess::s_create_funcs[ACCESS_MAX];

auto FileAccess::Create(AccessType p_access_type) -> std::shared_ptr<FileAccess> {
    DEV_ASSERT_INDEX(p_access_type, ACCESS_MAX);

    auto ret = s_create_funcs[p_access_type]();
    ret->SetAccessType(p_access_type);
    return std::shared_ptr<FileAccess>(ret);
}

auto FileAccess::CreateForPath(std::string_view p_path) -> std::shared_ptr<FileAccess> {
    if (p_path.starts_with("@res://")) {
        return Create(ACCESS_RESOURCE);
    }

    if (p_path.starts_with("@user://")) {
        return Create(ACCESS_USERDATA);
    }

    return Create(ACCESS_FILESYSTEM);
}

auto FileAccess::Open(std::string_view p_path, ModeFlags p_mode_flags) -> Result<std::shared_ptr<FileAccess>> {
    auto file_access = CreateForPath(p_path);

    // @TODO: FixPath should be put to FixPath should be a virtual function
    if (auto res = file_access->OpenInternal(FileAccess::FixPath(file_access->m_accessType, p_path), p_mode_flags); !res) {
        return CAVE_ERROR(res.error());
    }

    return file_access;
}

// @TODO: refactor this part
std::string FileAccess::FixPath(AccessType, std::string_view p_path) {
    VFS* vfs = VFS::GetSingletonPtr();
    return vfs ? vfs->Resolve(p_path) : std::string(p_path);
}

}  // namespace cave
