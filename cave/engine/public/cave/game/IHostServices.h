// =============================================================================
// File: cave/game/IHostServices.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/LogWrapper.h"
#include "cave/core/ids/ViewId.h"

// clang-format off
namespace cave::ecs { class ComponentRegistry; }
// clang-format on

namespace cave {

class AssetRegistry;
class IGameInput;
class IntentDispatcher;
class IUIRuntime;
class SceneCommandWriter;
class SceneQuery;

class IHostServices {
public:
    virtual ~IHostServices() = default;

    virtual AssetRegistry& assetRegistry() = 0;
    virtual ecs::ComponentRegistry& componentRegistry() = 0;
    virtual const IGameInput& gameInput() const = 0;
    virtual IntentDispatcher& intentDispatcher() = 0;
    virtual IUIRuntime& ui() = 0;
    virtual LogWrapper& log() = 0;
    virtual SceneQuery& sceneQuery() = 0;
    virtual SceneCommandWriter& sceneWriter() = 0;

    virtual ViewId viewId() const = 0;
};

}  // namespace cave