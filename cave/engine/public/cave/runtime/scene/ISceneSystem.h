// =============================================================================
// File: cave/runtime/scene/ISceneSystem.h
// =============================================================================
#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>
#include "SceneContext.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/core/ids/DebugId.h"

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
        std::memcpy(context_, &ctx, sizeof(context_));
        onAttach();
    }

    void detach() {
        onDetach();
        std::memset(context_, 0, sizeof(context_));
    }

    virtual void fixedUpdate(float) {}
    virtual void update(float) {}
    virtual void lateUpdate(float) {}

    virtual SceneSystemId systemId() const = 0;
    virtual DebugId debugId() const = 0;

protected:
    SceneContext& context() {
        DEV_ASSERT(context_[0]);
        return *reinterpret_cast<SceneContext*>(context_);
    }

    virtual void onAttach() {}
    virtual void onDetach() {}

private:
    uint8_t context_[sizeof(SceneContext)];
};

template<typename T>
concept SceneSystem =
    std::derived_from<T, ISceneSystem> &&
    requires {
        { T::kSystemId } -> std::convertible_to<SceneSystemId>;
    };

}  // namespace cave
