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

class SceneRuntime;

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
    ISceneSystem(SceneRuntime& runtime) noexcept
        : m_runtime(runtime) {}

    virtual ~ISceneSystem() = default;

    virtual void start() = 0;
    virtual void update(SceneTickContext&) = 0;

    virtual SceneTickDomain domain() const = 0;

    virtual SceneSystemId systemId() const = 0;
    virtual DebugId debugId() const = 0;

protected:
    SceneRuntime& m_runtime;
};

template<typename T>
concept SceneSystem =
    std::derived_from<T, ISceneSystem> &&
    requires {
        { T::kSystemId } -> std::convertible_to<SceneSystemId>;
    };

}  // namespace cave
