// =============================================================================
// File: cave/runtime/scene/ISceneSystem.h
// =============================================================================
#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>
#include "cave/core/error/ErrorMacros.h"

namespace cave {

struct SceneContext;

enum class SceneSystemId : uint32_t {
    Invalid = 0,

    TileWorld,
    NativeScript,
    LuaScript,
    Physics2D,
    Physics3D,

    Count,
};

#define CAVE_SCENE_SYSTEM(ID)                              \
public:                                                    \
    static constexpr ::cave::SceneSystemId kSystemId = ID; \
    ::cave::SceneSystemId systemId() const override {      \
        return kSystemId;                                  \
    }

class ISceneSystem {
public:
    virtual ~ISceneSystem() = default;

    void attach(SceneContext& ctx) {
        context_ = &ctx;
        onAttach();
    }

    void detach() {
        onDetach();
        context_ = nullptr;
    }

    virtual void fixedUpdate(float) {}
    virtual void update(float) {}
    virtual void lateUpdate(float) {}

    virtual SceneSystemId systemId() const = 0;

protected:
    SceneContext& context() {
        DEV_ASSERT(context_);
        return *context_;
    }

    virtual void onAttach() {}
    virtual void onDetach() {}

private:
    SceneContext* context_{ nullptr };
};

template<typename T>
concept SceneSystem =
    std::derived_from<T, ISceneSystem> &&
    requires {
        { T::kSystemId } -> std::convertible_to<SceneSystemId>;
    };

}  // namespace cave
