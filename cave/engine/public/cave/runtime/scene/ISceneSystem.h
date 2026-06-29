// =============================================================================
// File: cave/runtime/scene/ISceneSystem.h
// =============================================================================
#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>

#include "cave/core/error/ErrorMacros.h"
#include "cave/core/ids/DebugId.h"
#include "cave/runtime/scene/SceneTickContext.h"

namespace cave {

enum class SceneSystemId : uint32_t {
    Invalid = 0,

    TileWorld,
    NativeScript,
    LuaScript,
    Motor,
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

    void attach(SceneContext& ctx) { onAttach(ctx); }
    void detach(SceneContext& ctx) { onDetach(ctx); }

    virtual void fixedUpdate(SceneTickContext&) {}
    virtual void update(SceneTickContext&) {}
    virtual void lateUpdate(SceneTickContext&) {}

    virtual SceneSystemId systemId() const = 0;
    virtual DebugId debugId() const = 0;

protected:
    virtual void onAttach(SceneContext&) {}
    virtual void onDetach(SceneContext&) {}
};

template<typename T>
concept SceneSystem =
    std::derived_from<T, ISceneSystem> &&
    requires {
        { T::kSystemId } -> std::convertible_to<SceneSystemId>;
    };

}  // namespace cave
