#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IApplication.h"

namespace cave {

class PIEHostServices final : public IHostServices {
public:
    PIEHostServices(IApplication& p_app, SceneId p_pie_scene);

    ILogger& Log() override;
    cave::AssetRegistry& AssetRegistry() override;

private:
    IApplication& m_app;
    SceneId m_pie_scene{};
};

}  // namespace cave
