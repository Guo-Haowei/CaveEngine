// =============================================================================
// File: cave/runtime/script/native/NativeScriptRegistry.h
// =============================================================================
#pragma once

#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "cave/core/base/NonCopyable.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace cave {

using NativeScriptCreateFn = NativeScript* (*)();
using NativeScriptDestroyFn = void (*)(NativeScript*);

struct NativeScriptInfo {
    std::string id;  // "game.PlayerController"

    NativeScriptCreateFn create = nullptr;
    NativeScriptDestroyFn destroy = nullptr;
};

class NativeScriptRegistry : public NonCopyable {
public:
    NativeScriptRegistry() = default;
    ~NativeScriptRegistry() = default;

    template<typename T>
    void registerScript(std::string id) {
        static_assert(std::is_base_of_v<NativeScript, T>);

        NativeScriptInfo info;
        info.id = std::move(id);

        info.create = []() -> NativeScript* {
            return new T();
        };

        info.destroy = [](NativeScript* script) {
            delete static_cast<T*>(script);
        };

        registerScriptImpl(std::move(info));
    }

    void registerScript(NativeScriptInfo info);

    const NativeScriptInfo* find(std::string_view id) const;

    NativeScript* create(std::string_view id) const;
    void destroy(std::string_view id, NativeScript* script) const;

    std::span<const NativeScriptInfo> all() const {
        return scripts_;
    }

    void clear();

private:
    void registerScriptImpl(NativeScriptInfo info);

private:
    std::vector<NativeScriptInfo> scripts_;
    std::unordered_map<std::string, size_t> lookup_;
};

}  // namespace cave
