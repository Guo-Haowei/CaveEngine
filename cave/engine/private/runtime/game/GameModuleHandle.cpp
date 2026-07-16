#include "cave/runtime/game/GameModuleHandle.h"

namespace cave {

bool GameModuleHandle::loadFromDll(const char* dll_path, NativeScriptRegistry& registry) {
    unload();

    if (!m_dll.load(dll_path)) {
        return false;
    }

    void* create_func = m_dll.symbol("CreateGameModule");
    if (create_func == nullptr) {
        unload();
        return false;
    }

    void* destroy_func = m_dll.symbol("DestroyGameModule");
    if (destroy_func == nullptr) {
        unload();
        return false;
    }

    m_create_func = reinterpret_cast<CreateFn>(create_func);
    m_destroy_func = reinterpret_cast<DestroyFn>(destroy_func);

    m_module = m_create_func();
    if (m_module == nullptr) {
        unload();
        return false;
    }

    m_module->registerNativeScripts(registry);

    return true;
}

void GameModuleHandle::unload() {
    if (m_module) {
        m_destroy_func(m_module);
    }

    m_module = nullptr;
    m_create_func = nullptr;
    m_destroy_func = nullptr;

    m_dll.unload();
}

}  // namespace cave
