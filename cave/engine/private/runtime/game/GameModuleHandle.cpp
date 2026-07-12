#include "cave/runtime/game/GameModuleHandle.h"

namespace cave {

bool GameModuleHandle::loadFromDll(const char* dll_path, NativeScriptRegistry& registry) {
    unload();

    if (!dll_.load(dll_path)) {
        return false;
    }

    void* create_func = dll_.symbol("CreateGameModule");
    if (create_func == nullptr) {
        unload();
        return false;
    }

    void* destroy_func = dll_.symbol("DestroyGameModule");
    if (destroy_func == nullptr) {
        unload();
        return false;
    }

    create_ = reinterpret_cast<CreateFn>(create_func);
    destroy_ = reinterpret_cast<DestroyFn>(destroy_func);

    module_ = create_();
    if (module_ == nullptr) {
        unload();
        return false;
    }

    module_->registerNativeScripts(registry);

    return true;
}

void GameModuleHandle::unload() {
    if (module_) {
        destroy_(module_);
    }

    module_ = nullptr;
    create_ = nullptr;
    destroy_ = nullptr;

    dll_.unload();
}

}  // namespace cave
