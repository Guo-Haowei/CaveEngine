// =============================================================================
// File: engine/public/cave/game/IHostServices.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/ILogger.h"
#include "cave/core/ids/ViewId.h"

// clang-format off
namespace cave::ecs { class ComponentRegistry; }
// clang-format on

namespace cave {

class AssetRegistry;
class IInputService;
class IUIRuntime;
class SceneCommandWriter;
class SceneQuery;

class IHostServices {
public:
    virtual ~IHostServices() = default;

    virtual AssetRegistry& AssetRegistry() = 0;
    virtual ecs::ComponentRegistry& ComponentRegistry() = 0;
    virtual IInputService& Input() = 0;
    virtual IUIRuntime& UI() = 0;
    virtual ILogger& Log() = 0;
    virtual SceneQuery& SceneQuery() = 0;
    virtual SceneCommandWriter& SceneWriter() = 0;

    virtual ViewId GetViewId() const = 0;
};

}  // namespace cave