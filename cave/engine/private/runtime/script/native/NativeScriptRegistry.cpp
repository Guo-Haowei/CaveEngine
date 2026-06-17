#include "cave/runtime/script/native/NativeScriptRegistry.h"

namespace cave {

void NativeScriptRegistry::registerScript(NativeScriptInfo info) {
    registerScriptImpl(std::move(info));
}

void NativeScriptRegistry::registerScriptImpl(NativeScriptInfo info) {
    DEV_ASSERT(!info.id.empty());
    DEV_ASSERT(info.create);
    DEV_ASSERT(info.destroy);

    auto it = lookup_.find(info.id);
    DEV_ASSERT(it == lookup_.end());

    const size_t index = scripts_.size();
    lookup_.emplace(info.id, index);
    scripts_.push_back(std::move(info));
}

const NativeScriptInfo* NativeScriptRegistry::find(std::string_view id) const {
    auto it = lookup_.find(std::string(id));
    if (it == lookup_.end()) {
        return nullptr;
    }

    return &scripts_[it->second];
}

NativeScript* NativeScriptRegistry::create(std::string_view id) const {
    const NativeScriptInfo* info = find(id);
    if (!info || !info->create) {
        return nullptr;
    }

    return info->create();
}

void NativeScriptRegistry::destroy(std::string_view id, NativeScript* script) const {
    if (!script) {
        return;
    }

    const NativeScriptInfo* info = find(id);
    if (!info || !info->destroy) {
        DEV_ASSERT(false);
        return;
    }

    info->destroy(script);
}

void NativeScriptRegistry::clear() {
    scripts_.clear();
    lookup_.clear();
}

}  // namespace cave
