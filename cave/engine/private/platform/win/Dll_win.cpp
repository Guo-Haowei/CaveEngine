#include "cave/platform/Dll.h"

#if USING(PLATFORM_WINDOWS)
#include <Windows.h>

namespace cave {

Dll::~Dll() { Unload(); }

bool Dll::Load(const char* p_path) {
    Unload();
    m_handle = (void*)::LoadLibraryA(p_path);
    return m_handle != nullptr;
}

void Dll::Unload() {
    if (m_handle) {
        ::FreeLibrary((HMODULE)m_handle);
        m_handle = nullptr;
    }
}

void* Dll::GetSymbol(const char* p_name) const {
    if (!m_handle) return nullptr;
    return (void*)::GetProcAddress((HMODULE)m_handle, p_name);
}

Dll::Dll(Dll&& o) noexcept {
    m_handle = o.m_handle;
    o.m_handle = nullptr;
}

Dll& Dll::operator=(Dll&& o) noexcept {
    if (this == &o) return *this;
    Unload();
    m_handle = o.m_handle;
    o.m_handle = nullptr;
    return *this;
}

}  // namespace cave

#endif
