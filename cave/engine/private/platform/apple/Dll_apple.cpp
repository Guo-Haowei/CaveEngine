#include "cave/platform/Dll.h"

#if USING(PLATFORM_APPLE)
#include <dlfcn.h>

namespace cave {

Dll::~Dll() {
    unload();
}

bool Dll::load(const char* path) {
    unload();

    ::dlerror();
    m_handle = ::dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!m_handle) {
        const char* error = ::dlerror();
        LOG_ERROR(LogChannel::App,
                  "Dll::Load: Failed to load '{}' ({})",
                  path,
                  error ? error : "unknown error");
        return false;
    }

    return true;
}

void Dll::unload() {
    if (m_handle) {
        ::dlclose(m_handle);
        m_handle = nullptr;
    }
}

void* Dll::symbol(const char* name) const {
    if (!m_handle) {
        return nullptr;
    }

    ::dlerror();
    void* result = ::dlsym(m_handle, name);
    return ::dlerror() ? nullptr : result;
}

Dll::Dll(Dll&& o) noexcept
    : m_handle(o.m_handle) {
    o.m_handle = nullptr;
}

Dll& Dll::operator=(Dll&& o) noexcept {
    if (this == &o) {
        return *this;
    }

    unload();
    m_handle = o.m_handle;
    o.m_handle = nullptr;
    return *this;
}

}  // namespace cave

#endif
