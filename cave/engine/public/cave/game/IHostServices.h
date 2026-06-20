// =============================================================================
// File: cave/game/IHostServices.h
// =============================================================================
#pragma once
#include "cave/core/ids/ViewId.h"

// clang-format off
namespace cave::ecs { class ComponentRegistry; }
// clang-format on

namespace cave {

class AssetRegistry;
class DisplayService;
class IDebugDrawService;
class IGameInput;
class IntentDispatcher;
class IUIRuntime;
class SceneCommandWriter;
class SceneQuery;
class ViewQuery;

class IHostServices {
public:
    virtual ~IHostServices() = default;

    virtual AssetRegistry& assetRegistry() = 0;
    virtual ecs::ComponentRegistry& componentRegistry() = 0;
    virtual DisplayService& displayService() = 0;
    virtual IDebugDrawService& debugDraw() = 0;
    virtual const IGameInput& gameInput() const = 0;
    virtual IntentDispatcher& intentDispatcher() = 0;
    virtual IUIRuntime& ui() = 0;
    virtual SceneCommandWriter& sceneWriter() = 0;
    virtual SceneQuery& sceneQuery() = 0;

    virtual ViewId viewId() const = 0;

    virtual const ViewQuery& viewQuery() const = 0;
};

}  // namespace cave