#include "cave/game/GameModuleHandle.h"

namespace cave {

bool GameModuleHandle::LoadFromDll(const char* dll_path) {
    Unload();

    if (!m_dll.Load(dll_path)) {
        return false;
    }

    void* sym = m_dll.GetSymbol("CreateGameModule");
    if (!sym) {
        m_dll.Unload();
        return false;
    }

    m_create = reinterpret_cast<CreateFn>(sym);
    m_module.reset(m_create());
    return m_module != nullptr;
}

void GameModuleHandle::Unload() {
    m_module.reset();
    m_create = nullptr;
    m_dll.Unload();
}

}  // namespace cave
