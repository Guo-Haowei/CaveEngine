#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

namespace cave {

class PIEHostServices final : public IHostServices {
public:
    explicit PIEHostServices(IApplication& p_app, Scene& p_scene) noexcept;

    cave::AssetRegistry& AssetRegistry() override;
    cave::ecs::ComponentRegistry& ComponentRegistry() override;
    IInputService& Input() override;
    ILogger& Log() override;
    cave::SceneQuery& SceneQuery() override { return m_query; }
    SceneCommandWriter& SceneWriter() override { return m_writer; }

private:
    IApplication& m_app;
    cave::SceneQuery m_query;
    SceneCommandWriter m_writer;
};

}  // namespace cave
