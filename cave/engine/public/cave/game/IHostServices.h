// =============================================================================
// File: engine/public/cave/game/IHostServices.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/ILogger.h"

namespace cave::ecs {
class ComponentRegistry;
}

namespace cave {

class AssetRegistry;
class SceneCommandWriter;
class SceneQuery;

class IHostServices {
public:
    virtual ~IHostServices() = default;

    virtual ILogger& Log() = 0;
    virtual AssetRegistry& AssetRegistry() = 0;
    virtual ecs::ComponentRegistry& ComponentRegistry() = 0;
    virtual SceneQuery& SceneQuery() = 0;
    virtual SceneCommandWriter& SceneWriter() = 0;
};

}  // namespace cave