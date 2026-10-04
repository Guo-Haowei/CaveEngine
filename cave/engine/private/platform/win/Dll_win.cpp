#include "cave/core/PlatformDefines.h"

#if USING(PLATFORM_WINDOWS)
#include "cave/platform/Dll.h"
#include <Windows.h>

namespace cave {

Dll::~Dll() { unload(); }

bool Dll::load(const char* path) {
    unload();
    m_handle = (void*)::LoadLibraryA(path);
    if (!m_handle) {
        DWORD err = ::GetLastError();
        LOG_ERROR(LogChannel::App, "Dll::Load: Failed to load '{}' (GetLastError={})", path, err);
        return false;
    }
    return true;
}

void Dll::unload() {
    if (m_handle) {
        ::FreeLibrary((HMODULE)m_handle);
        m_handle = nullptr;
    }
}

void* Dll::symbol(const char* p_name) const {
    if (!m_handle) return nullptr;
    return (void*)::GetProcAddress((HMODULE)m_handle, p_name);
}

Dll::Dll(Dll&& o) noexcept {
    m_handle = o.m_handle;
    o.m_handle = nullptr;
}

Dll& Dll::operator=(Dll&& o) noexcept {
    if (this == &o) return *this;
    unload();
    m_handle = o.m_handle;
    o.m_handle = nullptr;
    return *this;
}

}  // namespace cave
#endif
