#include "cave/core/PlatformDefines.h"

#if USING(PLATFORM_WINDOWS)
#include "cave/platform/Dll.h"
#include <Windows.h>

namespace cave {

Dll::~Dll() { unload(); }

bool Dll::load(const char* path) {
    unload();
    handle_ = (void*)::LoadLibraryA(path);
    if (!handle_) {
        DWORD err = ::GetLastError();
        LOG_ERROR(LogChannel::App, "Dll::Load: Failed to load '{}' (GetLastError={})", path, err);
        return false;
    }
    return true;
}

void Dll::unload() {
    if (handle_) {
        ::FreeLibrary((HMODULE)handle_);
        handle_ = nullptr;
    }
}

void* Dll::symbol(const char* p_name) const {
    if (!handle_) return nullptr;
    return (void*)::GetProcAddress((HMODULE)handle_, p_name);
}

Dll::Dll(Dll&& o) noexcept {
    handle_ = o.handle_;
    o.handle_ = nullptr;
}

Dll& Dll::operator=(Dll&& o) noexcept {
    if (this == &o) return *this;
    unload();
    handle_ = o.handle_;
    o.handle_ = nullptr;
    return *this;
}

}  // namespace cave
#endif
